/*
 * File      : ipc.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-14     Bernard      the first version
 * 2006-04-25     Bernard      implement semaphore
 * 2006-05-03     Bernard      add RT_IPC_DEBUG
 *                             modify the type of IPC waiting time to rt_int32_t
 * 2006-05-10     Bernard      fix the semaphore take bug and add IPC object
 * 2006-05-12     Bernard      implement mailbox and message queue
 * 2006-05-20     Bernard      implement mutex
 * 2006-05-23     Bernard      implement fast event
 * 2006-05-24     Bernard      implement event
 * 2006-06-03     Bernard      fix the thread timer init bug
 * 2006-06-05     Bernard      fix the mutex release bug
 * 2006-06-07     Bernard      fix the message queue send bug
 * 2006-08-04     Bernard      add hook support
 * 2009-05-21     Yi.qiu       fix the sem release bug
 * 2009-07-18     Bernard      fix the event clear bug
 * 2009-09-09     Bernard      remove fast event and fix ipc release bug
 * 2009-10-10     Bernard      change semaphore and mutex value to unsigned value
 * 2009-10-25     Bernard      change the mb/mq receive timeout to 0 if the
 *                             re-calculated delta tick is a negative number.
 * 2009-12-16     Bernard      fix the rt_ipc_object_suspend issue when IPC flag
 *                             is RT_IPC_FLAG_PRIO
 * 2010-01-20     mbbill       remove rt_ipc_object_decrease function.
 * 2010-04-20     Bernard      move memcpy outside interrupt disable in mq
 * 2010-10-26     yi.qiu       add module support in rt_mp_delete and rt_mq_delete
 * 2010-11-10     Bernard      add IPC reset command implementation.
 * 2011-12-18     Bernard      add more parameter checking in message queue
 * 2013-09-14     Grissiom     add an option check in rt_event_recv
 */

#include <rtthread.h>
#include <rthw.h>

#ifdef RT_USING_HOOK
extern void (*rt_object_trytake_hook)(struct rt_object *object);
extern void (*rt_object_take_hook)(struct rt_object *object);
extern void (*rt_object_put_hook)(struct rt_object *object);
#endif

/**
 * @addtogroup IPC
 */

/**@{*/

/**
 * @brief IPC对象初始化
 *
 * 该函数将初始化一个 IPC 对象。
 *
 * @param ipc 指定的 IPC 对象
 *
 * @return the operation status, RT_EOK on successful
 */
rt_inline rt_err_t rt_ipc_object_init(struct rt_ipc_object *ipc)
{
    /* init ipc object */
    rt_list_init(&(ipc->suspend_thread));

    return RT_EOK;
}

/**
 * @brief 将线程挂起到对象链表
 *
 * 该函数将挂起一个线程到指定的链表。 IPC对象或某些双队列对象（邮箱等）包含这种链表。
 *
 * @param list 该IPC上挂起的线程链表
 * @param thread 要刮起的线程对象
 * @param flag IPC对象的标志位，应为RT_IPC_FLAG_FIFO / RT_IPC_FLAG_PRIO。
 *
 * @return RT_EOK
 */
rt_inline rt_err_t rt_ipc_list_suspend(rt_list_t        *list,
                                       struct rt_thread *thread,
                                       rt_uint8_t        flag)
{
    /* suspend thread */
    rt_thread_suspend(thread);

    switch (flag)
    {
    case RT_IPC_FLAG_FIFO:
        rt_list_insert_before(list, &(thread->tlist));
        break;

    case RT_IPC_FLAG_PRIO:
        {
            struct rt_list_node *n;
            struct rt_thread *sthread;

            /* find a suitable position */
            for (n = list->next; n != list; n = n->next)
            {
                sthread = rt_list_entry(n, struct rt_thread, tlist);

                /* find out */
                if (thread->current_priority < sthread->current_priority)
                {
                    /* insert this thread before the sthread */
                    rt_list_insert_before(&(sthread->tlist), &(thread->tlist));
                    break;
                }
            }

            /*
             * not found a suitable position,
             * append to the end of suspend_thread list
             */
            if (n == list)
                rt_list_insert_before(list, &(thread->tlist));
        }
        break;
    }

    return RT_EOK;
}

/**
 * @brief 恢复IPC对象上挂起的第一个线程
 *
 *此函数将恢复IPC对象列表中的第一个线程：
 * - 从IPC对象的挂起队列中删除线程
 * - 将线程放入系统就绪队列
 *
 * @param list 被挂起的线程链表
 *
 * @return RT_EOK
 */
rt_inline rt_err_t rt_ipc_list_resume(rt_list_t *list)
{
    struct rt_thread *thread;

    /* get thread entry */
    thread = rt_list_entry(list->next, struct rt_thread, tlist);

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("resume thread:%s\n", thread->name));

    /* resume it */
    rt_thread_resume(thread);

    return RT_EOK;
}

/**
 * @brief 恢复IPC对象上挂起的所有线程
 *
 *此函数将恢复列表中所有挂起的线程，包括IPC对象的挂起链表和邮箱的私有链表等。
 *
 * @param list 将要恢复的线程链表
 *
 * @return RT_EOK
 */
rt_inline rt_err_t rt_ipc_list_resume_all(rt_list_t *list)
{
    struct rt_thread *thread;
    register rt_ubase_t temp;

    /* wakeup all suspend threads */
    while (!rt_list_isempty(list))
    {
        /* disable interrupt */
        temp = rt_hw_interrupt_disable();

        /* get next suspend thread */
        thread = rt_list_entry(list->next, struct rt_thread, tlist);
        /* set error code to RT_ERROR */
        thread->error = -RT_ERROR;

        /*
         * resume thread
         * In rt_thread_resume function, it will remove current thread from
         * suspend list
         */
        rt_thread_resume(thread);

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);
    }

    return RT_EOK;
}

/**@}*/

#ifdef RT_USING_SEMAPHORE
/**
 * @addtogroup semaphore
 */

/**@{*/

/**
 * @brief 初始化信号量
 *
 * 该函数将初始化信号量并将其置于内核管理器的控制之下。
 *
 * @param sem 指定的信号量对象句柄
 * @param name 信号量的名称
 * @param value 信号量初始化时的值
 * @param flag 信号量的标志位
 *
 * @return RT_EOK
 */
rt_err_t rt_sem_init(rt_sem_t    sem,
                     const char *name,
                     rt_uint32_t value,
                     rt_uint8_t  flag)
{
    RT_ASSERT(sem != RT_NULL);

    /* init object */
    rt_object_init(&(sem->parent.parent), RT_Object_Class_Semaphore, name);

    /* init ipc object */
    rt_ipc_object_init(&(sem->parent));

    /* set init value */
    sem->value = value;

    /* set parent */
    sem->parent.parent.flag = flag;

    return RT_EOK;
}
RTM_EXPORT(rt_sem_init);

/**
 * @brief 脱离信号量
 *
 * 该函数将把指定的静态信号量从内核对象管理器中移除。
 *
 * @param sem 指定的信号量对象
 *
 * @return 脱离成功返回 RT_EOK
 *
 * @see rt_sem_delete
 */
rt_err_t rt_sem_detach(rt_sem_t sem)
{
    RT_ASSERT(sem != RT_NULL);

    /* wakeup all suspend threads */
    rt_ipc_list_resume_all(&(sem->parent.suspend_thread));

    /* detach semaphore object */
    rt_object_detach(&(sem->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_sem_detach);

#ifdef RT_USING_HEAP
/**
 * @brief 创建信号量
 *
 * 调用该函数时，系统将先从对象管理器中分配一个semaphore对象，并初始化这个对象，
 * 然后初始化 IPC 对象以及与 semaphore 相关的部分。。
 *
 * @param name 信号量的名称
 * @param value 信号量初始化时的值
 * @param flag 信号量的标志位
 *
 * @return 返回创建的信号量控制块, 当错误时返回 RT_NULL
 *
 * @see rt_sem_init
 */
rt_sem_t rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag)
{
    rt_sem_t sem;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* allocate object */
    sem = (rt_sem_t)rt_object_allocate(RT_Object_Class_Semaphore, name);
    if (sem == RT_NULL)
        return sem;

    /* init ipc object */
    rt_ipc_object_init(&(sem->parent));

    /* set init value */
    sem->value = value;

    /* set parent */
    sem->parent.parent.flag = flag;

    return sem;
}
RTM_EXPORT(rt_sem_create);

/**
 * @brief 删除信号量
 *
 * 系统不再使用信号量时，可通过删除信号量以释放系统资源，适用于动态创建的信号量。
 * 调用该函数将删除一个信号量对象，并释放其占用的内存空间。
 *
 * @param sem 信号量对象句柄
 *
 * @return RT_EOK删除成功
 *
 * @see rt_sem_detach
 */
rt_err_t rt_sem_delete(rt_sem_t sem)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    RT_ASSERT(sem != RT_NULL);

    /* wakeup all suspend threads */
    rt_ipc_list_resume_all(&(sem->parent.suspend_thread));

    /* delete semaphore object */
    rt_object_delete(&(sem->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_sem_delete);
#endif

/**
 * @brief 获取信号量
 *
 * 线程通过调用该函数获取信号量来获得信号量资源实例，当信号量值大于零时，线程将获得信号量，
 * 并且相应的信号量值会减 1；如果信号量的值等于零，那么说明当前信号量资源实例不可用，申请
 * 该信号量的线程将根据 time 参数的情况选择直接返回、或挂起等待一段时间、或永久等待，直到
 * 其他线程或中断释放该信号量。
 *
 * @param sem 信号量对象的句柄
 * @param time 指定的等待时间，单位是操作系统时钟节拍（OS Tick）
 *
 * @return RT_EOK 成功获得信号量；-RT_ETIMEOUT 超时依然未获得信号量；-RT_ERROR 其他错误
 */
rt_err_t rt_sem_take(rt_sem_t sem, rt_int32_t time)
{
    register rt_base_t temp;
    struct rt_thread *thread;

    RT_ASSERT(sem != RT_NULL);

    RT_OBJECT_HOOK_CALL(rt_object_trytake_hook, (&(sem->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("thread %s take sem:%s, which value is: %d\n",
                                rt_thread_self()->name,
                                ((struct rt_object *)sem)->name,
                                sem->value));

    if (sem->value > 0)
    {
        /* semaphore is available */
        sem->value --;

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);
    }
    else
    {
        /* no waiting, return with timeout */
        if (time == 0)
        {
            rt_hw_interrupt_enable(temp);

            return -RT_ETIMEOUT;
        }
        else
        {
            /* current context checking */
            RT_DEBUG_IN_THREAD_CONTEXT;

            /* semaphore is unavailable, push to suspend list */
            /* get current thread */
            thread = rt_thread_self();

            /* reset thread error number */
            thread->error = RT_EOK;

            RT_DEBUG_LOG(RT_DEBUG_IPC, ("sem take: suspend thread - %s\n",
                                        thread->name));

            /* suspend thread */
            rt_ipc_list_suspend(&(sem->parent.suspend_thread),
                                thread,
                                sem->parent.parent.flag);

            /* has waiting time, start thread timer */
            if (time > 0)
            {
                RT_DEBUG_LOG(RT_DEBUG_IPC, ("set thread:%s to timer list\n",
                                            thread->name));

                /* reset the timeout of thread timer and start it */
                rt_timer_control(&(thread->thread_timer),
                                 RT_TIMER_CTRL_SET_TIME,
                                 &time);
                rt_timer_start(&(thread->thread_timer));
            }

            /* enable interrupt */
            rt_hw_interrupt_enable(temp);

            /* do schedule */
            rt_schedule();

            if (thread->error != RT_EOK)
            {
                return thread->error;
            }
        }
    }

    RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(sem->parent.parent)));

    return RT_EOK;
}
RTM_EXPORT(rt_sem_take);

/**
 * @brief 无等待获取信号量
 *
 * 当用户不想在申请的信号量上挂起线程进行等待时，可以调用该函数以无等待方式获取信号量。
 *
 * @param sem 信号量对象的句柄
 *
 * @return RT_EOK 成功获得信号量；-RT_ETIMEOUT 获取失败
 */
rt_err_t rt_sem_trytake(rt_sem_t sem)
{
    return rt_sem_take(sem, 0);
}
RTM_EXPORT(rt_sem_trytake);

/**
 * @brief 释放信号量
 *
 * 该函数将释放一个信号量，当信号量的值等于零时，并且有线程等待这个信号量时，
 * 释放信号量将唤醒等待在该信号量线程队列中的第一个线程，由它获取信号量；否则
 * 将把信号量的值加一。
 *
 * @param sem 信号量对象的句柄
 *
 * @return RT_EOK 成功释放信号量
 */
rt_err_t rt_sem_release(rt_sem_t sem)
{
    register rt_base_t temp;
    register rt_bool_t need_schedule;

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(sem->parent.parent)));

    need_schedule = RT_FALSE;

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("thread %s releases sem:%s, which value is: %d\n",
                                rt_thread_self()->name,
                                ((struct rt_object *)sem)->name,
                                sem->value));

    if (!rt_list_isempty(&sem->parent.suspend_thread))
    {
        /* resume the suspended thread */
        rt_ipc_list_resume(&(sem->parent.suspend_thread));
        need_schedule = RT_TRUE;
    }
    else
        sem->value ++; /* increase value */

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    /* resume a thread, re-schedule */
    if (need_schedule == RT_TRUE)
        rt_schedule();

    return RT_EOK;
}
RTM_EXPORT(rt_sem_release);

/*
 * @brief 信号量设置
 *
 * 该函数可以获取或设置信号量对象的一些额外属性。
 *
 * @param sem 信号量对象的句柄
 * @param cmd 执行命令
 * @param arg 执行命令的参数
 *
 * @return 错误代码
 */
rt_err_t rt_sem_control(rt_sem_t sem, int cmd, void *arg)
{
    rt_ubase_t level;
    RT_ASSERT(sem != RT_NULL);

    if (cmd == RT_IPC_CMD_RESET)
    {
        rt_uint32_t value;

        /* get value */
        value = (rt_uint32_t)arg;
        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        /* resume all waiting thread */
        rt_ipc_list_resume_all(&sem->parent.suspend_thread);

        /* set new value */
        sem->value = (rt_uint16_t)value;

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        rt_schedule();

        return RT_EOK;
    }

    return -RT_ERROR;
}
RTM_EXPORT(rt_sem_control);

/**@}*/
#endif /* end of RT_USING_SEMAPHORE */

#ifdef RT_USING_MUTEX
/**
 * @addtogroup mutex
 */

/**@{*/

/**
 * @brief 初始化互斥量
 *
 * 该函数将初始化互斥锁并将其置于内核管理器的控制之下
 *
 * @param mutex 互斥量对象的句柄
 * @param name 互斥量的名称
 * @param flag 互斥量的标志位
 *
 * @return RT_EOK 初始化成功
 */
rt_err_t rt_mutex_init(rt_mutex_t mutex, const char *name, rt_uint8_t flag)
{
    RT_ASSERT(mutex != RT_NULL);

    /* init object */
    rt_object_init(&(mutex->parent.parent), RT_Object_Class_Mutex, name);

    /* init ipc object */
    rt_ipc_object_init(&(mutex->parent));

    mutex->value = 1;
    mutex->owner = RT_NULL;
    mutex->original_priority = 0xFF;
    mutex->hold  = 0;

    /* set flag */
    mutex->parent.parent.flag = flag;

    return RT_EOK;
}
RTM_EXPORT(rt_mutex_init);

/**
 * @brief 脱离互斥量
 *
 * 该函数将把互斥量对象从内核对象管理器中删除，适用于静态初始化的互斥量。
 *
 * @param mutex 互斥量对象的句柄
 *
 * @return RT_EOK 成功
 *
 * @see rt_mutex_delete
 */
rt_err_t rt_mutex_detach(rt_mutex_t mutex)
{
    RT_ASSERT(mutex != RT_NULL);

    /* wakeup all suspend threads */
    rt_ipc_list_resume_all(&(mutex->parent.suspend_thread));

    /* detach semaphore object */
    rt_object_detach(&(mutex->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_mutex_detach);

#ifdef RT_USING_HEAP
/**
 * @brief 创建互斥量
 *
 * 调用该函数创建一个互斥量时，内核首先创建一个互斥量控制块，然后完成对该控制块的初始化工作。
 *
 * @param name 互斥量的名称
 * @param flag 互斥量的标志位
 *
 * @return 返回创建的互斥量，失败时返回RT_NULL
 *
 * @see rt_mutex_init
 */
rt_mutex_t rt_mutex_create(const char *name, rt_uint8_t flag)
{
    struct rt_mutex *mutex;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* allocate object */
    mutex = (rt_mutex_t)rt_object_allocate(RT_Object_Class_Mutex, name);
    if (mutex == RT_NULL)
        return mutex;

    /* init ipc object */
    rt_ipc_object_init(&(mutex->parent));

    mutex->value              = 1;
    mutex->owner              = RT_NULL;
    mutex->original_priority  = 0xFF;
    mutex->hold               = 0;

    /* set flag */
    mutex->parent.parent.flag = flag;

    return mutex;
}
RTM_EXPORT(rt_mutex_create);

/**
 * @brief 删除互斥量
 *
 * 当系统不再使用互斥量时，通过调用该函数删除互斥量以释放系统资源，适用于动态创建的互斥量。
 *
 * @param mutex 互斥量对象的句柄
 *
 * @return RT_EOK 删除成功
 *
 * @see rt_mutex_detach
 */
rt_err_t rt_mutex_delete(rt_mutex_t mutex)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    RT_ASSERT(mutex != RT_NULL);

    /* wakeup all suspend threads */
    rt_ipc_list_resume_all(&(mutex->parent.suspend_thread));

    /* delete semaphore object */
    rt_object_delete(&(mutex->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_mutex_delete);
#endif

/**
 * @brief 获取互斥量
 *
 * 该函数将获取一个互斥量，如果互斥量没有被其他线程控制，那么申请该互斥量的线程将成功获得该互斥量。
 * 如果互斥量已经被当前线程线程控制，则该互斥量的持有计数加 1，当前线程也不会挂起等待。如果互斥量
 * 已经被其他线程占有，则当前线程在该互斥量上挂起等待，直到其他线程释放它或者等待时间超过指定的
 * 超时时间。
 *
 * @param mutex 互斥量对象的句柄
 * @param time 指定的等待时间
 *
 * @return RT_EOK 成功获得互斥量；-RT_ETIMEOUT 超时；-RT_ERROR 获取失败
 */
rt_err_t rt_mutex_take(rt_mutex_t mutex, rt_int32_t time)
{
    register rt_base_t temp;
    struct rt_thread *thread;

    /* this function must not be used in interrupt even if time = 0 */
    RT_DEBUG_IN_THREAD_CONTEXT;

    RT_ASSERT(mutex != RT_NULL);

    /* get current thread */
    thread = rt_thread_self();

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    RT_OBJECT_HOOK_CALL(rt_object_trytake_hook, (&(mutex->parent.parent)));

    RT_DEBUG_LOG(RT_DEBUG_IPC,
                 ("mutex_take: current thread %s, mutex value: %d, hold: %d\n",
                  thread->name, mutex->value, mutex->hold));

    /* reset thread error */
    thread->error = RT_EOK;

    if (mutex->owner == thread)
    {
        /* it's the same thread */
        mutex->hold ++;
    }
    else
    {
__again:
        /* The value of mutex is 1 in initial status. Therefore, if the
         * value is great than 0, it indicates the mutex is avaible.
         */
        if (mutex->value > 0)
        {
            /* mutex is available */
            mutex->value --;

            /* set mutex owner and original priority */
            mutex->owner             = thread;
            mutex->original_priority = thread->current_priority;
            mutex->hold ++;
        }
        else
        {
            /* no waiting, return with timeout */
            if (time == 0)
            {
                /* set error as timeout */
                thread->error = -RT_ETIMEOUT;

                /* enable interrupt */
                rt_hw_interrupt_enable(temp);

                return -RT_ETIMEOUT;
            }
            else
            {
                /* mutex is unavailable, push to suspend list */
                RT_DEBUG_LOG(RT_DEBUG_IPC, ("mutex_take: suspend thread: %s\n",
                                            thread->name));

                /* change the owner thread priority of mutex */
                if (thread->current_priority < mutex->owner->current_priority)
                {
                    /* change the owner thread priority */
                    rt_thread_control(mutex->owner,
                                      RT_THREAD_CTRL_CHANGE_PRIORITY,
                                      &thread->current_priority);
                }

                /* suspend current thread */
                rt_ipc_list_suspend(&(mutex->parent.suspend_thread),
                                    thread,
                                    mutex->parent.parent.flag);

                /* has waiting time, start thread timer */
                if (time > 0)
                {
                    RT_DEBUG_LOG(RT_DEBUG_IPC,
                                 ("mutex_take: start the timer of thread:%s\n",
                                  thread->name));

                    /* reset the timeout of thread timer and start it */
                    rt_timer_control(&(thread->thread_timer),
                                     RT_TIMER_CTRL_SET_TIME,
                                     &time);
                    rt_timer_start(&(thread->thread_timer));
                }

                /* enable interrupt */
                rt_hw_interrupt_enable(temp);

                /* do schedule */
                rt_schedule();

                if (thread->error != RT_EOK)
                {
                	/* interrupt by signal, try it again */
                	if (thread->error == -RT_EINTR) goto __again;

                    /* return error */
                    return thread->error;
                }
                else
                {
                    /* the mutex is taken successfully. */
                    /* disable interrupt */
                    temp = rt_hw_interrupt_disable();
                }
            }
        }
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(mutex->parent.parent)));

    return RT_EOK;
}
RTM_EXPORT(rt_mutex_take);

/**
 * @brief 释放互斥量
 *
 * 使用该函数接口时，只有已经拥有互斥量控制权的线程才能释放它，每释放一次该互斥量，
 * 它的持有计数就减 1。当该互斥量的持有计数为零时（即持有线程已经释放所有的持有操作），
 * 它变为可用，等待在该信号量上的线程将被唤醒。如果线程的运行优先级被互斥量提升，
 * 那么当互斥量被释放后，线程恢复为持有互斥量前的优先级。
 *
 * @param mutex 互斥量对象的句柄
 *
 * @return RT_EOK 成功
 */
rt_err_t rt_mutex_release(rt_mutex_t mutex)
{
    register rt_base_t temp;
    struct rt_thread *thread;
    rt_bool_t need_schedule;

    need_schedule = RT_FALSE;

    /* only thread could release mutex because we need test the ownership */
    RT_DEBUG_IN_THREAD_CONTEXT;

    /* get current thread */
    thread = rt_thread_self();

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    RT_DEBUG_LOG(RT_DEBUG_IPC,
                 ("mutex_release:current thread %s, mutex value: %d, hold: %d\n",
                  thread->name, mutex->value, mutex->hold));

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(mutex->parent.parent)));

    /* mutex only can be released by owner */
    if (thread != mutex->owner)
    {
        thread->error = -RT_ERROR;

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        return -RT_ERROR;
    }

    /* decrease hold */
    mutex->hold --;
    /* if no hold */
    if (mutex->hold == 0)
    {
        /* change the owner thread to original priority */
        if (mutex->original_priority != mutex->owner->current_priority)
        {
            rt_thread_control(mutex->owner,
                              RT_THREAD_CTRL_CHANGE_PRIORITY,
                              &(mutex->original_priority));
        }

        /* wakeup suspended thread */
        if (!rt_list_isempty(&mutex->parent.suspend_thread))
        {
            /* get suspended thread */
            thread = rt_list_entry(mutex->parent.suspend_thread.next,
                                   struct rt_thread,
                                   tlist);

            RT_DEBUG_LOG(RT_DEBUG_IPC, ("mutex_release: resume thread: %s\n",
                                        thread->name));

            /* set new owner and priority */
            mutex->owner             = thread;
            mutex->original_priority = thread->current_priority;
            mutex->hold ++;

            /* resume thread */
            rt_ipc_list_resume(&(mutex->parent.suspend_thread));

            need_schedule = RT_TRUE;
        }
        else
        {
            /* increase value */
            mutex->value ++;

            /* clear owner */
            mutex->owner             = RT_NULL;
            mutex->original_priority = 0xff;
        }
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    /* perform a schedule */
    if (need_schedule == RT_TRUE)
        rt_schedule();

    return RT_EOK;
}
RTM_EXPORT(rt_mutex_release);

/*
 * @brief 互斥量参数修改
 *
 * 该函数可以获取或设置互斥对象的一些额外属性。
 *
 * @param mutex 互斥量对象的句柄
 * @param cmd 执行命令
 * @param arg 执行命令相关的参数
 *
 * @return 错误代码
 */
rt_err_t rt_mutex_control(rt_mutex_t mutex, int cmd, void *arg)
{
    return -RT_ERROR;
}
RTM_EXPORT(rt_mutex_control);

/**@}*/
#endif /* end of RT_USING_MUTEX */

#ifdef RT_USING_EVENT
/**
 * @addtogroup event
 */

/**@{*/

/**
 * @brief 初始化事件集
 *
 * 该函数将初始化事件集对象，并加入到系统对象容器中进行管理。
 *
 * @param event 事件集对象的句柄
 * @param name 事件集的名称
 * @param flag 事件集的标志，它可以取值：RT_IPC_FLAG_FIFO或RT_IPC_FLAG_PRIO
 *
 * @return RT_EOK
 */
rt_err_t rt_event_init(rt_event_t event, const char *name, rt_uint8_t flag)
{
    RT_ASSERT(event != RT_NULL);

    /* init object */
    rt_object_init(&(event->parent.parent), RT_Object_Class_Event, name);

    /* set parent flag */
    event->parent.parent.flag = flag;

    /* init ipc object */
    rt_ipc_object_init(&(event->parent));

    /* init event */
    event->set = 0;

    return RT_EOK;
}
RTM_EXPORT(rt_event_init);

/**
 * @brief 脱离事件集
 *
 * 该函数将把一个事件从内核管理器中脱离
 *
 * @param event 事件对象的句柄
 *
 * @return RT_EOK
 */
rt_err_t rt_event_detach(rt_event_t event)
{
    /* parameter check */
    RT_ASSERT(event != RT_NULL);

    /* resume all suspended thread */
    rt_ipc_list_resume_all(&(event->parent.suspend_thread));

    /* detach event object */
    rt_object_detach(&(event->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_event_detach);

#ifdef RT_USING_HEAP
/**
 * @brief 创建事件集
 *
 * 调用该函数接口时，系统会从对象管理器中分配事件对象，然后进行对象的初始化，
 * IPC对象初始化，并把set设置成0。
 *
 * @param name 事件集的名称
 * @param flag 事件集的标志，它可以取值：RT_IPC_FLAG_FIFO或RT_IPC_FLAG_PRIO
 *
 * @return 返回创建的事件对象的句柄，创建失败返回 RT_NULL
 */
rt_event_t rt_event_create(const char *name, rt_uint8_t flag)
{
    rt_event_t event;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* allocate object */
    event = (rt_event_t)rt_object_allocate(RT_Object_Class_Event, name);
    if (event == RT_NULL)
        return event;

    /* set parent */
    event->parent.parent.flag = flag;

    /* init ipc object */
    rt_ipc_object_init(&(event->parent));

    /* init event */
    event->set = 0;

    return event;
}
RTM_EXPORT(rt_event_create);

/**
 * @brief 删除事件集
 *
 * 系统不再使用rt_event_create() 创建的事件集对象时，通过调用该函数删除事件集对象
 * 控制块来释放系统资源。
 *
 * @param event 事件集对象的句柄
 *
 * @return RT_EOK
 */
rt_err_t rt_event_delete(rt_event_t event)
{
    /* parameter check */
    RT_ASSERT(event != RT_NULL);

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* resume all suspended thread */
    rt_ipc_list_resume_all(&(event->parent.suspend_thread));

    /* delete event object */
    rt_object_delete(&(event->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_event_delete);
#endif

/**
 * @brief 发送事件
 *
 * 使用该函数接口时，通过参数 set 指定的事件标志来设定 event 事件集对象的事件标志值，
 * 然后遍历等待在event事件集对象上的等待线程链表，判断是否有线程的事件激活要求与当前 
 * event对象事件标志值匹配，如果有，则唤醒该线程。
 *
 * @param event 事件集对象的句柄
 * @param set 发送的一个或多个事件的标志值
 *
 * @return RT_EOK
 */
rt_err_t rt_event_send(rt_event_t event, rt_uint32_t set)
{
    struct rt_list_node *n;
    struct rt_thread *thread;
    register rt_ubase_t level;
    register rt_base_t status;
    rt_bool_t need_schedule;

    /* parameter check */
    RT_ASSERT(event != RT_NULL);
    if (set == 0)
        return -RT_ERROR;

    need_schedule = RT_FALSE;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    /* set event */
    event->set |= set;

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(event->parent.parent)));
    
    if (!rt_list_isempty(&event->parent.suspend_thread))
    {
        /* search thread list to resume thread */
        n = event->parent.suspend_thread.next;
        while (n != &(event->parent.suspend_thread))
        {
            /* get thread */
            thread = rt_list_entry(n, struct rt_thread, tlist);

            status = -RT_ERROR;
            if (thread->event_info & RT_EVENT_FLAG_AND)
            {
                if ((thread->event_set & event->set) == thread->event_set)
                {
                    /* received an AND event */
                    status = RT_EOK;
                }
            }
            else if (thread->event_info & RT_EVENT_FLAG_OR)
            {
                if (thread->event_set & event->set)
                {
                    /* save recieved event set */
                    thread->event_set = thread->event_set & event->set;

                    /* received an OR event */
                    status = RT_EOK;
                }
            }

            /* move node to the next */
            n = n->next;

            /* condition is satisfied, resume thread */
            if (status == RT_EOK)
            {
                /* clear event */
                if (thread->event_info & RT_EVENT_FLAG_CLEAR)
                    event->set &= ~thread->event_set;

                /* resume thread, and thread list breaks out */
                rt_thread_resume(thread);

                /* need do a scheduling */
                need_schedule = RT_TRUE;
            }
        }
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    /* do a schedule */
    if (need_schedule == RT_TRUE)
        rt_schedule();

    return RT_EOK;
}
RTM_EXPORT(rt_event_send);

/**
 * @brief 接收事件
 *
 * 当用户调用这个接口时，系统首先根据set参数和接收选项option来判断它要接收的事件是否发生，
 * 如果已经发生，则根据参数option上是否设置有 RT_EVENT_FLAG_CLEAR 来决定是否重置事件的相应
 * 标志位，然后返回（其中recved参数返回收到的事件）；如果没有发生，则把等待的set和option
 * 参数填入线程本身的结构中，然后把线程挂起在此事件上，直到其等待的事件满足条件或等待时间
 * 超过指定的超时时间。如果超时时间设置为零，则表示当线程要接受的事件没有满足其要求时就不
 * 等待，而直接返回-RT_ETIMEOUT。
 *
 * @param event 事件集对象的句柄
 * @param set 接收线程感兴趣的事件
 * @param option 接收选项，应设置接收选项RT_EVENT_FLAG_AND或RT_EVENT_FLAG_OR。
 * @param timeout 指定超时时间
 * @param recved 指向收到的事件，如果不感兴趣，可以设置为 RT_NULL。
 *
 * @return RT_EOK 成功；-RT_ETIMEOUT 超时；-RT_ERROR 错误
 */
rt_err_t rt_event_recv(rt_event_t   event,
                       rt_uint32_t  set,
                       rt_uint8_t   option,
                       rt_int32_t   timeout,
                       rt_uint32_t *recved)
{
    struct rt_thread *thread;
    register rt_ubase_t level;
    register rt_base_t status;

    RT_DEBUG_IN_THREAD_CONTEXT;

    /* parameter check */
    RT_ASSERT(event != RT_NULL);
    if (set == 0)
        return -RT_ERROR;

    /* init status */
    status = -RT_ERROR;
    /* get current thread */
    thread = rt_thread_self();
    /* reset thread error */
    thread->error = RT_EOK;

    RT_OBJECT_HOOK_CALL(rt_object_trytake_hook, (&(event->parent.parent)));

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    /* check event set */
    if (option & RT_EVENT_FLAG_AND)
    {
        if ((event->set & set) == set)
            status = RT_EOK;
    }
    else if (option & RT_EVENT_FLAG_OR)
    {
        if (event->set & set)
            status = RT_EOK;
    }
    else
    {
        /* either RT_EVENT_FLAG_AND or RT_EVENT_FLAG_OR should be set */
        RT_ASSERT(0);
    }

    if (status == RT_EOK)
    {
        /* set received event */
        if (recved)
            *recved = (event->set & set);

        /* received event */
        if (option & RT_EVENT_FLAG_CLEAR)
            event->set &= ~set;
    }
    else if (timeout == 0)
    {
        /* no waiting */
        thread->error = -RT_ETIMEOUT;
    }
    else
    {
        /* fill thread event info */
        thread->event_set  = set;
        thread->event_info = option;

        /* put thread to suspended thread list */
        rt_ipc_list_suspend(&(event->parent.suspend_thread),
                            thread,
                            event->parent.parent.flag);

        /* if there is a waiting timeout, active thread timer */
        if (timeout > 0)
        {
            /* reset the timeout of thread timer and start it */
            rt_timer_control(&(thread->thread_timer),
                             RT_TIMER_CTRL_SET_TIME,
                             &timeout);
            rt_timer_start(&(thread->thread_timer));
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        /* do a schedule */
        rt_schedule();

        if (thread->error != RT_EOK)
        {
            /* return error */
            return thread->error;
        }

        /* received an event, disable interrupt to protect */
        level = rt_hw_interrupt_disable();

        /* set received event */
        if (recved)
            *recved = thread->event_set;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(event->parent.parent)));

    return thread->error;
}
RTM_EXPORT(rt_event_recv);

/*
 * @brief 事件参数的修改
 *
 * 该函数可以获取或设置事件对象的一些额外属性。
 *
 * @param event 事件对象的句柄
 * @param cmd 执行命令
 * @param arg 执行命令的参数
 *
 * @return 错误代码
 */
rt_err_t rt_event_control(rt_event_t event, int cmd, void *arg)
{
    rt_ubase_t level;
    RT_ASSERT(event != RT_NULL);

    if (cmd == RT_IPC_CMD_RESET)
    {
        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        /* resume all waiting thread */
        rt_ipc_list_resume_all(&event->parent.suspend_thread);

        /* init event set */
        event->set = 0;

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        rt_schedule();

        return RT_EOK;
    }

    return -RT_ERROR;
}
RTM_EXPORT(rt_event_control);

/**@}*/
#endif /* end of RT_USING_EVENT */

#ifdef RT_USING_MAILBOX
/**
 * @addtogroup mailbox
 */

/**@{*/

/**
* @brief 初始化邮箱
 *
 * 该函数将初始化邮箱并将其置于内核管理器的控制之下。
 *
 * @param mb 邮箱对象的句柄
 * @param name 邮箱的名称
 * @param msgpool 缓冲区指针
 * @param size 邮箱容量
 * @param flag 邮箱标志，它可以取数值：RT_IPC_FLAG_FIFO或RT_IPC_FLAG_PRIO
 *
 * @return RT_EOK
 */
rt_err_t rt_mb_init(rt_mailbox_t mb,
                    const char  *name,
                    void        *msgpool,
                    rt_size_t    size,
                    rt_uint8_t   flag)
{
    RT_ASSERT(mb != RT_NULL);

    /* init object */
    rt_object_init(&(mb->parent.parent), RT_Object_Class_MailBox, name);

    /* set parent flag */
    mb->parent.parent.flag = flag;

    /* init ipc object */
    rt_ipc_object_init(&(mb->parent));

    /* init mailbox */
    mb->msg_pool   = msgpool;
    mb->size       = size;
    mb->entry      = 0;
    mb->in_offset  = 0;
    mb->out_offset = 0;

    /* init an additional list of sender suspend thread */
    rt_list_init(&(mb->suspend_sender_thread));

    return RT_EOK;
}
RTM_EXPORT(rt_mb_init);

/**
* @brief 脱离邮箱
 *
 * 该函数将将把静态初始化的邮箱对象从内核对象管理器中删除。使用该函数接口后，
 * 内核先唤醒所有挂在该邮箱上的线程（线程获得返回值是 - RT_ERROR ），然后将
 * 该邮箱对象从内核对象管理器中删除。
 *
 * @param mb 邮箱对象的句柄
 *
 * @return RT_EOK
 */
rt_err_t rt_mb_detach(rt_mailbox_t mb)
{
    /* parameter check */
    RT_ASSERT(mb != RT_NULL);

    /* resume all suspended thread */
    rt_ipc_list_resume_all(&(mb->parent.suspend_thread));
    /* also resume all mailbox private suspended thread */
    rt_ipc_list_resume_all(&(mb->suspend_sender_thread));

    /* detach mailbox object */
    rt_object_detach(&(mb->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_mb_detach);

#ifdef RT_USING_HEAP
/**
 * @brief 创建邮箱
 *
 * 调用该函数创建邮箱对象时会先从对象管理器中分配一个邮箱对象，然后给邮箱动态分配
 * 一块内存空间用来存放邮件，这块内存的大小等于邮件大小（4 字节）与邮箱容量的乘积，
 * 接着初始化接收邮件数目和发送邮件在邮箱中的偏移量。
 *
 * @param name 邮箱的名称
 * @param size 邮箱容量
 * @param flag 邮箱标志，它可以取值：RT_IPC_FLAG_FIFO 或 RT_IPC_FLAG_PRIO
 *
* @return 返回创建的邮箱对象，创建失败时返回 RT_NULL
 */
rt_mailbox_t rt_mb_create(const char *name, rt_size_t size, rt_uint8_t flag)
{
    rt_mailbox_t mb;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* allocate object */
    mb = (rt_mailbox_t)rt_object_allocate(RT_Object_Class_MailBox, name);
    if (mb == RT_NULL)
        return mb;

    /* set parent */
    mb->parent.parent.flag = flag;

    /* init ipc object */
    rt_ipc_object_init(&(mb->parent));

    /* init mailbox */
    mb->size     = size;
    mb->msg_pool = RT_KERNEL_MALLOC(mb->size * sizeof(rt_uint32_t));
    if (mb->msg_pool == RT_NULL)
    {
        /* delete mailbox object */
        rt_object_delete(&(mb->parent.parent));

        return RT_NULL;
    }
    mb->entry      = 0;
    mb->in_offset  = 0;
    mb->out_offset = 0;

    /* init an additional list of sender suspend thread */
    rt_list_init(&(mb->suspend_sender_thread));

    return mb;
}
RTM_EXPORT(rt_mb_create);

/**
 * @brief 删除邮箱
 *
 * 当用rt_mb_create()创建的邮箱不再被使用时，应该调用该函数删除它来释放相应的系统资源，
 * 一旦操作完成，邮箱将被永久性的删除。删除邮箱时，如果有线程被挂起在该邮箱对象上，
 * 内核先唤醒挂起在该邮箱上的所有线程（线程返回值是- RT_ERROR），然后再释放邮箱使用的
 * 内存，最后删除邮箱对象。
 *
 * @param mb 邮箱对象的句柄
 *
 * @return 错误代码
 */
rt_err_t rt_mb_delete(rt_mailbox_t mb)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    /* parameter check */
    RT_ASSERT(mb != RT_NULL);

    /* resume all suspended thread */
    rt_ipc_list_resume_all(&(mb->parent.suspend_thread));

    /* also resume all mailbox private suspended thread */
    rt_ipc_list_resume_all(&(mb->suspend_sender_thread));

    /* free mailbox pool */
    RT_KERNEL_FREE(mb->msg_pool);

    /* delete mailbox object */
    rt_object_delete(&(mb->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_mb_delete);
#endif

/**
 * @brief 等待方式发送邮件
 *
 * 用户也可以通过调用该函数接口向指定邮箱发送邮件。它与 rt_mb_send()的区别在于，
 * 如果邮箱已经满了，那么发送线程将根据设定的timeout参数等待邮箱中因为收取邮件
 * 而空出空间。如果设置的超时时间到达依然没有空出空间，这时发送线程将被唤醒返回
 * 错误码。
 *
 * @param mb 邮箱对象的句柄
 * @param value 邮件内容
 * @param timeout 超时时间
 *
 * @return RT_EOK 发送成功；-RT_ETIMEOUT 超时；-RT_ERROR 失败，返回错误号
 */
rt_err_t rt_mb_send_wait(rt_mailbox_t mb,
                         rt_uint32_t  value,
                         rt_int32_t   timeout)
{
    struct rt_thread *thread;
    register rt_ubase_t temp;
    rt_uint32_t tick_delta;

    /* parameter check */
    RT_ASSERT(mb != RT_NULL);

    /* initialize delta tick */
    tick_delta = 0;
    /* get current thread */
    thread = rt_thread_self();

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(mb->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* for non-blocking call */
    if (mb->entry == mb->size && timeout == 0)
    {
        rt_hw_interrupt_enable(temp);

        return -RT_EFULL;
    }

    /* mailbox is full */
    while (mb->entry == mb->size)
    {
        /* reset error number in thread */
        thread->error = RT_EOK;

        /* no waiting, return timeout */
        if (timeout == 0)
        {
            /* enable interrupt */
            rt_hw_interrupt_enable(temp);

            return -RT_EFULL;
        }

        RT_DEBUG_IN_THREAD_CONTEXT;
        /* suspend current thread */
        rt_ipc_list_suspend(&(mb->suspend_sender_thread),
                            thread,
                            mb->parent.parent.flag);

        /* has waiting time, start thread timer */
        if (timeout > 0)
        {
            /* get the start tick of timer */
            tick_delta = rt_tick_get();

            RT_DEBUG_LOG(RT_DEBUG_IPC, ("mb_send_wait: start timer of thread:%s\n",
                                        thread->name));

            /* reset the timeout of thread timer and start it */
            rt_timer_control(&(thread->thread_timer),
                             RT_TIMER_CTRL_SET_TIME,
                             &timeout);
            rt_timer_start(&(thread->thread_timer));
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        /* re-schedule */
        rt_schedule();

        /* resume from suspend state */
        if (thread->error != RT_EOK)
        {
            /* return error */
            return thread->error;
        }

        /* disable interrupt */
        temp = rt_hw_interrupt_disable();

        /* if it's not waiting forever and then re-calculate timeout tick */
        if (timeout > 0)
        {
            tick_delta = rt_tick_get() - tick_delta;
            timeout -= tick_delta;
            if (timeout < 0)
                timeout = 0;
        }
    }

    /* set ptr */
    mb->msg_pool[mb->in_offset] = value;
    /* increase input offset */
    ++ mb->in_offset;
    if (mb->in_offset >= mb->size)
        mb->in_offset = 0;
    /* increase message entry */
    mb->entry ++;

    /* resume suspended thread */
    if (!rt_list_isempty(&mb->parent.suspend_thread))
    {
        rt_ipc_list_resume(&(mb->parent.suspend_thread));

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        rt_schedule();

        return RT_EOK;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    return RT_EOK;
}
RTM_EXPORT(rt_mb_send_wait);

/**
 * @brief 发送邮件
 *
 * 线程或者中断服务程序可以通过调用该函数给其他线程发送邮件。发送的邮件可以是 32 位
 * 任意格式的数据，一个整型值或者一个指向缓冲区的指针。当邮箱中的邮件已经满时，发送
 * 邮件的线程或者中断程序会收到- RT_EFULL 的返回值。
 *
 * @param mb 邮箱对象的句柄
 * @param value 邮件内容
 *
 * @return RT_EOK 发送成功；-RT_EFULL 邮箱已经满了
 */
rt_err_t rt_mb_send(rt_mailbox_t mb, rt_uint32_t value)
{
    return rt_mb_send_wait(mb, value, 0);
}
RTM_EXPORT(rt_mb_send);

/**
 * @brief 接收邮件
 *
 * 只有当接收者接收的邮箱中有邮件时，接收者才能立即取到邮件并返回 RT_EOK 的返回值，
 * 否则接收线程会根据超时时间设置，或挂起在邮箱的等待线程队列上，或直接返回。
 *
 * @param mb 邮箱对象的句柄
 * @param value 邮件内容
 * @param timeout 超时时间
 *
 * @return RT_EOK 发送成功；-RT_ETIMEOUT 超时；-RT_ERROR 失败，返回错误号
 */
rt_err_t rt_mb_recv(rt_mailbox_t mb, rt_uint32_t *value, rt_int32_t timeout)
{
    struct rt_thread *thread;
    register rt_ubase_t temp;
    rt_uint32_t tick_delta;

    /* parameter check */
    RT_ASSERT(mb != RT_NULL);

    /* initialize delta tick */
    tick_delta = 0;
    /* get current thread */
    thread = rt_thread_self();

    RT_OBJECT_HOOK_CALL(rt_object_trytake_hook, (&(mb->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* for non-blocking call */
    if (mb->entry == 0 && timeout == 0)
    {
        rt_hw_interrupt_enable(temp);

        return -RT_ETIMEOUT;
    }

    /* mailbox is empty */
    while (mb->entry == 0)
    {
        /* reset error number in thread */
        thread->error = RT_EOK;

        /* no waiting, return timeout */
        if (timeout == 0)
        {
            /* enable interrupt */
            rt_hw_interrupt_enable(temp);

            thread->error = -RT_ETIMEOUT;

            return -RT_ETIMEOUT;
        }

        RT_DEBUG_IN_THREAD_CONTEXT;
        /* suspend current thread */
        rt_ipc_list_suspend(&(mb->parent.suspend_thread),
                            thread,
                            mb->parent.parent.flag);

        /* has waiting time, start thread timer */
        if (timeout > 0)
        {
            /* get the start tick of timer */
            tick_delta = rt_tick_get();

            RT_DEBUG_LOG(RT_DEBUG_IPC, ("mb_recv: start timer of thread:%s\n",
                                        thread->name));

            /* reset the timeout of thread timer and start it */
            rt_timer_control(&(thread->thread_timer),
                             RT_TIMER_CTRL_SET_TIME,
                             &timeout);
            rt_timer_start(&(thread->thread_timer));
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        /* re-schedule */
        rt_schedule();

        /* resume from suspend state */
        if (thread->error != RT_EOK)
        {
            /* return error */
            return thread->error;
        }

        /* disable interrupt */
        temp = rt_hw_interrupt_disable();

        /* if it's not waiting forever and then re-calculate timeout tick */
        if (timeout > 0)
        {
            tick_delta = rt_tick_get() - tick_delta;
            timeout -= tick_delta;
            if (timeout < 0)
                timeout = 0;
        }
    }

    /* fill ptr */
    *value = mb->msg_pool[mb->out_offset];

    /* increase output offset */
    ++ mb->out_offset;
    if (mb->out_offset >= mb->size)
        mb->out_offset = 0;
    /* decrease message entry */
    mb->entry --;

    /* resume suspended thread */
    if (!rt_list_isempty(&(mb->suspend_sender_thread)))
    {
        rt_ipc_list_resume(&(mb->suspend_sender_thread));

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(mb->parent.parent)));

        rt_schedule();

        return RT_EOK;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(mb->parent.parent)));

    return RT_EOK;
}
RTM_EXPORT(rt_mb_recv);

/*
 * @brief 邮箱参数的修改
 *
 * 该函数可以获取或设置邮箱对象的一些额外属性。
 *
 * @param mb 邮箱对象的句柄
 * @param cmd 执行命令
 * @param arg 执行命令相关的参数
 *
 * @return 错误代码
 */
rt_err_t rt_mb_control(rt_mailbox_t mb, int cmd, void *arg)
{
    rt_ubase_t level;
    RT_ASSERT(mb != RT_NULL);

    if (cmd == RT_IPC_CMD_RESET)
    {
        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        /* resume all waiting thread */
        rt_ipc_list_resume_all(&(mb->parent.suspend_thread));
        /* also resume all mailbox private suspended thread */
        rt_ipc_list_resume_all(&(mb->suspend_sender_thread));

        /* re-init mailbox */
        mb->entry      = 0;
        mb->in_offset  = 0;
        mb->out_offset = 0;

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        rt_schedule();

        return RT_EOK;
    }

    return -RT_ERROR;
}
RTM_EXPORT(rt_mb_control);

/**@}*/
#endif /* end of RT_USING_MAILBOX */

#ifdef RT_USING_MESSAGEQUEUE
/**
 * @addtogroup messagequeue
 */

/**@{*/

struct rt_mq_message
{
    struct rt_mq_message *next;
};

/**
 * @brief 初始化消息队列
 *
 * 该函数将初始化消息队列并将其置于内核管理器的控制之下。
 *
 * @param mq 消息队列对象句柄
 * @param name 消息队列的名称
 * @param msgpool 用于存放消息的缓冲区指针
 * @param msg_size 消息队列中一条消息的最大长度，单位字节
 * @param pool_size 存放消息的缓冲区大小
 * @param flag 消息队列采用的等待方式，它可以取值：RT_IPC_FLAG_FIFO或RT_IPC_FLAG_PRIO
 *
 * @return RT_EOK
 */
rt_err_t rt_mq_init(rt_mq_t     mq,
                    const char *name,
                    void       *msgpool,
                    rt_size_t   msg_size,
                    rt_size_t   pool_size,
                    rt_uint8_t  flag)
{
    struct rt_mq_message *head;
    register rt_base_t temp;

    /* parameter check */
    RT_ASSERT(mq != RT_NULL);

    /* init object */
    rt_object_init(&(mq->parent.parent), RT_Object_Class_MessageQueue, name);

    /* set parent flag */
    mq->parent.parent.flag = flag;

    /* init ipc object */
    rt_ipc_object_init(&(mq->parent));

    /* set messasge pool */
    mq->msg_pool = msgpool;

    /* get correct message size */
    mq->msg_size = RT_ALIGN(msg_size, RT_ALIGN_SIZE);
    mq->max_msgs = pool_size / (mq->msg_size + sizeof(struct rt_mq_message));

    /* init message list */
    mq->msg_queue_head = RT_NULL;
    mq->msg_queue_tail = RT_NULL;

    /* init message empty list */
    mq->msg_queue_free = RT_NULL;
    for (temp = 0; temp < mq->max_msgs; temp ++)
    {
        head = (struct rt_mq_message *)((rt_uint8_t *)mq->msg_pool +
                                        temp * (mq->msg_size + sizeof(struct rt_mq_message)));
        head->next = mq->msg_queue_free;
        mq->msg_queue_free = head;
    }

    /* the initial entry is zero */
    mq->entry = 0;

    return RT_EOK;
}
RTM_EXPORT(rt_mq_init);

/**
 * @brief 脱离消息队列
 *
 * 该函数将使消息队列对象被从内核对象管理器中删除。使用该函数接口后，
 * 内核先唤醒所有挂在该消息等待队列对象上的线程（返回值是 - RT_ERROR ），
 * 然后将该消息队列对象从内核对象管理器中删除。
 *
 * @param mq 消息队列对象句柄
 *
 * @return RT_EOK
 */
rt_err_t rt_mq_detach(rt_mq_t mq)
{
    /* parameter check */
    RT_ASSERT(mq != RT_NULL);

    /* resume all suspended thread */
    rt_ipc_list_resume_all(&mq->parent.suspend_thread);

    /* detach message queue object */
    rt_object_detach(&(mq->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_mq_detach);

#ifdef RT_USING_HEAP
/**
 * @brief 创建消息队列
 *
 * 调用该函数创建消息队列时先从对象管理器中分配一个消息队列对象，然后
 * 给消息队列对象分配一块内存空间，组织成空闲消息链表，这块内存的大小= 
 * [消息大小+消息头（用于链表连接）] *消息队列最大个数，接着再初始化
 * 消息队列，此时消息队列为空。
 *
 * @param name 消息队列的名称
 * @param msg_size 消息队列中一条消息的最大长度，单位字节
 * @param max_msgs 消息队列的最大个数
 * @param flag 消息队列采用的等待方式，它可以取值：RT_IPC_FLAG_FIFO或RT_IPC_FLAG_PRIO
 *
 * @return 返回创建的消息队列，当发生错误时返回 RT_NULL
 */
rt_mq_t rt_mq_create(const char *name,
                     rt_size_t   msg_size,
                     rt_size_t   max_msgs,
                     rt_uint8_t  flag)
{
    struct rt_messagequeue *mq;
    struct rt_mq_message *head;
    register rt_base_t temp;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* allocate object */
    mq = (rt_mq_t)rt_object_allocate(RT_Object_Class_MessageQueue, name);
    if (mq == RT_NULL)
        return mq;

    /* set parent */
    mq->parent.parent.flag = flag;

    /* init ipc object */
    rt_ipc_object_init(&(mq->parent));

    /* init message queue */

    /* get correct message size */
    mq->msg_size = RT_ALIGN(msg_size, RT_ALIGN_SIZE);
    mq->max_msgs = max_msgs;

    /* allocate message pool */
    mq->msg_pool = RT_KERNEL_MALLOC((mq->msg_size + sizeof(struct rt_mq_message)) * mq->max_msgs);
    if (mq->msg_pool == RT_NULL)
    {
        rt_mq_delete(mq);

        return RT_NULL;
    }

    /* init message list */
    mq->msg_queue_head = RT_NULL;
    mq->msg_queue_tail = RT_NULL;

    /* init message empty list */
    mq->msg_queue_free = RT_NULL;
    for (temp = 0; temp < mq->max_msgs; temp ++)
    {
        head = (struct rt_mq_message *)((rt_uint8_t *)mq->msg_pool +
                                        temp * (mq->msg_size + sizeof(struct rt_mq_message)));
        head->next = mq->msg_queue_free;
        mq->msg_queue_free = head;
    }

    /* the initial entry is zero */
    mq->entry = 0;

    return mq;
}
RTM_EXPORT(rt_mq_create);

/**
 * @brief 删除消息队列
 *
 * 当消息队列不再被使用时，应该调用该函数接口删除它以释放系统资源，一旦操作完成，
 * 消息队列将被永久性地删除。删除消息队列时，如果有线程被挂起在该消息队列等待队列上，
 * 则内核先唤醒挂起在该消息等待队列上的所有线程（返回值是 -RT_ERROR），然后再释放
 * 消息队列使用的内存，最后删除消息队列对象。
 *
 * @param mq 消息队列对象句柄
 *
 * @return RT_EOK 成功
 */
rt_err_t rt_mq_delete(rt_mq_t mq)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    /* parameter check */
    RT_ASSERT(mq != RT_NULL);

    /* resume all suspended thread */
    rt_ipc_list_resume_all(&(mq->parent.suspend_thread));

    /* free message queue pool */
    RT_KERNEL_FREE(mq->msg_pool);

    /* delete message queue object */
    rt_object_delete(&(mq->parent.parent));

    return RT_EOK;
}
RTM_EXPORT(rt_mq_delete);
#endif

/**
 * @brief 发送消息
 *
 * 线程或者中断服务程序都可以调用该函数给消息队列发送消息。当发送消息时，
 * 消息队列对象先从空闲消息链表上取下一个空闲消息块，把线程或者中断服务
 * 程序发送的消息内容复制到消息块上，然后把该消息块挂到消息队列的尾部。
 * 当且仅当空闲消息链表上有可用的空闲消息块时，发送者才能成功发送消息；
 * 当空闲消息链表上无可用消息块，说明消息队列已满，此时，发送消息的的
 * 线程或者中断程序会收到一个错误码（-RT_EFULL）。
 *
 * @param mq 消息队列对象句柄
 * @param buffer 消息内容
 * @param size 消息大小
 *
 * @return RT_EOK 成功；-RT_EFULL 消息队列已满；-RT_ERROR 失败，表示发送的
 * 消息长度大于消息队列中消息的最大长度
 */
rt_err_t rt_mq_send(rt_mq_t mq, void *buffer, rt_size_t size)
{
    register rt_ubase_t temp;
    struct rt_mq_message *msg;

    RT_ASSERT(mq != RT_NULL);
    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(size != 0);

    /* greater than one message size */
    if (size > mq->msg_size)
        return -RT_ERROR;

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(mq->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* get a free list, there must be an empty item */
    msg = (struct rt_mq_message *)mq->msg_queue_free;
    /* message queue is full */
    if (msg == RT_NULL)
    {
        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        return -RT_EFULL;
    }
    /* move free list pointer */
    mq->msg_queue_free = msg->next;

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    /* the msg is the new tailer of list, the next shall be NULL */
    msg->next = RT_NULL;
    /* copy buffer */
    rt_memcpy(msg + 1, buffer, size);

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();
    /* link msg to message queue */
    if (mq->msg_queue_tail != RT_NULL)
    {
        /* if the tail exists, */
        ((struct rt_mq_message *)mq->msg_queue_tail)->next = msg;
    }

    /* set new tail */
    mq->msg_queue_tail = msg;
    /* if the head is empty, set head */
    if (mq->msg_queue_head == RT_NULL)
        mq->msg_queue_head = msg;

    /* increase message entry */
    mq->entry ++;

    /* resume suspended thread */
    if (!rt_list_isempty(&mq->parent.suspend_thread))
    {
        rt_ipc_list_resume(&(mq->parent.suspend_thread));

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        rt_schedule();

        return RT_EOK;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    return RT_EOK;
}
RTM_EXPORT(rt_mq_send);

/**
 * @brief 发送紧急消息
 *
 * 发送紧急消息的过程与发送消息几乎一样，唯一的不同是，当发送紧急消息时，
 * 从空闲消息链表上取下来的消息块不是挂到消息队列的队尾，而是挂到队首，
 * 这样，接收者就能够优先接收到紧急消息，从而及时进行消息处理。
 *
 * @param mq 消息队列对象句柄
 * @param buffer 消息内容
 * @param size 消息大小
 *
 * @return RT_EOK 成功；-RT_EFULL 消息队列已满；-RT_ERROR 失败
 */
rt_err_t rt_mq_urgent(rt_mq_t mq, void *buffer, rt_size_t size)
{
    register rt_ubase_t temp;
    struct rt_mq_message *msg;

    RT_ASSERT(mq != RT_NULL);
    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(size != 0);

    /* greater than one message size */
    if (size > mq->msg_size)
        return -RT_ERROR;

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(mq->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* get a free list, there must be an empty item */
    msg = (struct rt_mq_message *)mq->msg_queue_free;
    /* message queue is full */
    if (msg == RT_NULL)
    {
        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        return -RT_EFULL;
    }
    /* move free list pointer */
    mq->msg_queue_free = msg->next;

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    /* copy buffer */
    rt_memcpy(msg + 1, buffer, size);

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* link msg to the beginning of message queue */
    msg->next = mq->msg_queue_head;
    mq->msg_queue_head = msg;

    /* if there is no tail */
    if (mq->msg_queue_tail == RT_NULL)
        mq->msg_queue_tail = msg;

    /* increase message entry */
    mq->entry ++;

    /* resume suspended thread */
    if (!rt_list_isempty(&mq->parent.suspend_thread))
    {
        rt_ipc_list_resume(&(mq->parent.suspend_thread));

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        rt_schedule();

        return RT_EOK;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    return RT_EOK;
}
RTM_EXPORT(rt_mq_urgent);

/**
 * @brief 接收消息
 *
 * 当消息队列中有消息时，接收者才能接收消息，否则接收者会根据超时时间设置，
 * 或挂起在消息队列的等待线程队列上，或直接返回。
 *
 * @param mq 消息队列对象句柄
 * @param buffer 消息内容
 * @param size 消息大小
 * @param timeout 指定的超时时间
 *
 * @return RT_EOK 成功；-RT_ETIMEOUT 超时；-RT_ERROR 失败
 */
rt_err_t rt_mq_recv(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout)
{
    struct rt_thread *thread;
    register rt_ubase_t temp;
    struct rt_mq_message *msg;
    rt_uint32_t tick_delta;

    RT_ASSERT(mq != RT_NULL);
    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(size != 0);

    /* initialize delta tick */
    tick_delta = 0;
    /* get current thread */
    thread = rt_thread_self();
    RT_OBJECT_HOOK_CALL(rt_object_trytake_hook, (&(mq->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* for non-blocking call */
    if (mq->entry == 0 && timeout == 0)
    {
        rt_hw_interrupt_enable(temp);

        return -RT_ETIMEOUT;
    }

    /* message queue is empty */
    while (mq->entry == 0)
    {
        RT_DEBUG_IN_THREAD_CONTEXT;

        /* reset error number in thread */
        thread->error = RT_EOK;

        /* no waiting, return timeout */
        if (timeout == 0)
        {
            /* enable interrupt */
            rt_hw_interrupt_enable(temp);

            thread->error = -RT_ETIMEOUT;

            return -RT_ETIMEOUT;
        }

        /* suspend current thread */
        rt_ipc_list_suspend(&(mq->parent.suspend_thread),
                            thread,
                            mq->parent.parent.flag);

        /* has waiting time, start thread timer */
        if (timeout > 0)
        {
            /* get the start tick of timer */
            tick_delta = rt_tick_get();

            RT_DEBUG_LOG(RT_DEBUG_IPC, ("set thread:%s to timer list\n",
                                        thread->name));

            /* reset the timeout of thread timer and start it */
            rt_timer_control(&(thread->thread_timer),
                             RT_TIMER_CTRL_SET_TIME,
                             &timeout);
            rt_timer_start(&(thread->thread_timer));
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        /* re-schedule */
        rt_schedule();

        /* recv message */
        if (thread->error != RT_EOK)
        {
            /* return error */
            return thread->error;
        }

        /* disable interrupt */
        temp = rt_hw_interrupt_disable();

        /* if it's not waiting forever and then re-calculate timeout tick */
        if (timeout > 0)
        {
            tick_delta = rt_tick_get() - tick_delta;
            timeout -= tick_delta;
            if (timeout < 0)
                timeout = 0;
        }
    }

    /* get message from queue */
    msg = (struct rt_mq_message *)mq->msg_queue_head;

    /* move message queue head */
    mq->msg_queue_head = msg->next;
    /* reach queue tail, set to NULL */
    if (mq->msg_queue_tail == msg)
        mq->msg_queue_tail = RT_NULL;

    /* decrease message entry */
    mq->entry --;

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    /* copy message */
    rt_memcpy(buffer, msg + 1, size > mq->msg_size ? mq->msg_size : size);

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();
    /* put message to free list */
    msg->next = (struct rt_mq_message *)mq->msg_queue_free;
    mq->msg_queue_free = msg;
    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(mq->parent.parent)));

    return RT_EOK;
}
RTM_EXPORT(rt_mq_recv);

/*
 * @brief 消息队列对象参数的修改
 *
 * 该函数可以获取或设置消息队列对象的一些额外属性。
 *
 * @param mq 消息队列对象句柄
 * @param cmd 执行的命令
 * @param arg 执行命令相关的参数
 *
 * @return 错误代码
 */
rt_err_t rt_mq_control(rt_mq_t mq, int cmd, void *arg)
{
    rt_ubase_t level;
    struct rt_mq_message *msg;

    RT_ASSERT(mq != RT_NULL);

    if (cmd == RT_IPC_CMD_RESET)
    {
        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        /* resume all waiting thread */
        rt_ipc_list_resume_all(&mq->parent.suspend_thread);

        /* release all message in the queue */
        while (mq->msg_queue_head != RT_NULL)
        {
            /* get message from queue */
            msg = (struct rt_mq_message *)mq->msg_queue_head;

            /* move message queue head */
            mq->msg_queue_head = msg->next;
            /* reach queue tail, set to NULL */
            if (mq->msg_queue_tail == msg)
                mq->msg_queue_tail = RT_NULL;

            /* put message to free list */
            msg->next = (struct rt_mq_message *)mq->msg_queue_free;
            mq->msg_queue_free = msg;
        }

        /* clean entry */
        mq->entry = 0;

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        rt_schedule();

        return RT_EOK;
    }

    return -RT_ERROR;
}
RTM_EXPORT(rt_mq_control);

/**@}*/
#endif /* end of RT_USING_MESSAGEQUEUE */

/**@}*/
