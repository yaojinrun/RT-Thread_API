/*
 * File      : thread.c
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
 * 2006-03-28     Bernard      first version
 * 2006-04-29     Bernard      implement thread timer
 * 2006-04-30     Bernard      added THREAD_DEBUG
 * 2006-05-27     Bernard      fixed the rt_thread_yield bug
 * 2006-06-03     Bernard      fixed the thread timer init bug
 * 2006-08-10     Bernard      fixed the timer bug in thread_sleep
 * 2006-09-03     Bernard      changed rt_timer_delete to rt_timer_detach
 * 2006-09-03     Bernard      implement rt_thread_detach
 * 2008-02-16     Bernard      fixed the rt_thread_timeout bug
 * 2010-03-21     Bernard      change the errno of rt_thread_delay/sleep to
 *                             RT_EOK.
 * 2010-11-10     Bernard      add cleanup callback function in thread exit.
 * 2011-09-01     Bernard      fixed rt_thread_exit issue when the current
 *                             thread preempted, which reported by Jiaxing Lee.
 * 2011-09-08     Bernard      fixed the scheduling issue in rt_thread_startup.
 * 2012-12-29     Bernard      fixed compiling warning.
 * 2016-08-09     ArdaFu       add thread suspend and resume hook.
 * 2017-04-10     armink       fixed the rt_thread_delete and rt_thread_detach
                               bug when thread has not startup.
 */

#include <rtthread.h>
#include <rthw.h>

extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];
extern struct rt_thread *rt_current_thread;
extern rt_list_t rt_thread_defunct;

#ifdef RT_USING_HOOK

static void (*rt_thread_suspend_hook)(rt_thread_t thread);
static void (*rt_thread_resume_hook) (rt_thread_t thread);
static void (*rt_thread_inited_hook) (rt_thread_t thread);

/**
 * @ingroup Hook
 * @brief 设置线程挂起钩子函数
 *
 * 钩子函数会在系统挂起线程时调用。
 *
 * @param hook 指定的钩子函数
 *
 * @note 钩子函数必须简单，并且永远不会被阻塞或挂起。
 */
void rt_thread_suspend_sethook(void (*hook)(rt_thread_t thread))
{
    rt_thread_suspend_hook = hook;
}

/**
 * @ingroup Hook
 * @brief 设置线程恢复钩子函数
 *
 * 钩子函数会在系统恢复线程调用。
 *
 * @param hook 指定的钩子函数
 *
 * @note 钩子函数必须简单，并且永远不会被阻塞或挂起。
 */
void rt_thread_resume_sethook(void (*hook)(rt_thread_t thread))
{
    rt_thread_resume_hook = hook;
}

/**
 * @brief 设置线程初始化钩子函数
 *
 * @ingroup Hook
 * 钩子函数会在线程初始化的时候调用。
 *
 * @param hook 指定的钩子函数
 */
void rt_thread_inited_sethook(void (*hook)(rt_thread_t thread))
{
    rt_thread_inited_hook = hook;
}

#endif

void rt_thread_exit(void)
{
    struct rt_thread *thread;
    register rt_base_t level;

    /* get current thread */
    thread = rt_current_thread;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    /* remove from schedule */
    rt_schedule_remove_thread(thread);
    /* change stat */
    thread->stat = RT_THREAD_CLOSE;

    /* remove it from timer list */
    rt_timer_detach(&thread->thread_timer);

    if ((rt_object_is_systemobject((rt_object_t)thread) == RT_TRUE) &&
        thread->cleanup == RT_NULL)
    {
        rt_object_detach((rt_object_t)thread);
    }
    else
    {
        /* insert to defunct thread list */
        rt_list_insert_after(&rt_thread_defunct, &(thread->tlist));
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    /* switch to next task */
    rt_schedule();
}

static rt_err_t _rt_thread_init(struct rt_thread *thread,
                                const char       *name,
                                void (*entry)(void *parameter),
                                void             *parameter,
                                void             *stack_start,
                                rt_uint32_t       stack_size,
                                rt_uint8_t        priority,
                                rt_uint32_t       tick)
{
    /* init thread list */
    rt_list_init(&(thread->tlist));

    thread->entry = (void *)entry;
    thread->parameter = parameter;

    /* stack init */
    thread->stack_addr = stack_start;
    thread->stack_size = stack_size;

    /* init thread stack */
    rt_memset(thread->stack_addr, '#', thread->stack_size);
    thread->sp = (void *)rt_hw_stack_init(thread->entry, thread->parameter,
                                          (void *)((char *)thread->stack_addr + thread->stack_size - 4),
                                          (void *)rt_thread_exit);

    /* priority init */
    RT_ASSERT(priority < RT_THREAD_PRIORITY_MAX);
    thread->init_priority    = priority;
    thread->current_priority = priority;

    thread->number_mask = 0;
#if RT_THREAD_PRIORITY_MAX > 32
    thread->number = 0;
    thread->high_mask = 0;
#endif

    /* tick init */
    thread->init_tick      = tick;
    thread->remaining_tick = tick;

    /* error and flags */
    thread->error = RT_EOK;
    thread->stat  = RT_THREAD_INIT;

    /* initialize cleanup function and user data */
    thread->cleanup   = 0;
    thread->user_data = 0;

    /* init thread timer */
    rt_timer_init(&(thread->thread_timer),
                  thread->name,
                  rt_thread_timeout,
                  thread,
                  0,
                  RT_TIMER_FLAG_ONE_SHOT);

#ifdef RT_USING_MODULE
    thread->module_id = RT_NULL;
#endif

    /* initialize signal */
#ifdef RT_USING_SIGNALS
    thread->sig_mask    = 0x00;
    thread->sig_pending = 0x00;

    thread->sig_ret     = RT_NULL;
    thread->sig_vectors = RT_NULL;
    thread->si_list     = RT_NULL;
#endif

    RT_OBJECT_HOOK_CALL(rt_thread_inited_hook, (thread));

    return RT_EOK;
}

/**
 * @addtogroup Thread
 */

/**@{*/

/**
 * @brief 初始化线程
 *
 * 此函数将初始化一个线程，通常用于初始化一个静态线程对象。
 *
 * @param thread 线程句柄。线程句柄由用户提供出来，并指向对应的线程控制块内存地址。
 * @param name 线程的名称；线程名称的最大长度由 rtconfig.h 中定义的 RT_NAME_MAX 宏指定，多余部分会被自动截掉。
 * @param entry 线程的入口函数
 * @param parameter 入口函数的传入参数
 * @param stack_start 线程堆栈的起始地址
 * @param stack_size 线程栈大小，单位是字节。在大多数系统中需要做栈空间地址对齐（例如 ARM 体系结构中需要向 4 字节地址对齐）。
 * @param priority 线程的优先级。优先级范围根据系统配置情况（rtconfig.h 中的 RT_THREAD_PRIORITY_MAX 宏定义），如果支持的是 256 级优先级，那么范围是从 0 ～ 255，数值越小优先级越高，0 代表最高优先级。
 * @param tick 线程的时间片大小。当系统中存在相同优先级线程时，这个参数指定线程一次调度能够运行的最大时间长度。
 *
 * @return 成功返回 RT_EOK, 失败则返回 -RT_ERROR
 */
rt_err_t rt_thread_init(struct rt_thread *thread,
                        const char       *name,
                        void (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        rt_uint32_t       stack_size,
                        rt_uint8_t        priority,
                        rt_uint32_t       tick)
{
    /* thread check */
    RT_ASSERT(thread != RT_NULL);
    RT_ASSERT(stack_start != RT_NULL);

    /* init thread object */
    rt_object_init((rt_object_t)thread, RT_Object_Class_Thread, name);

    return _rt_thread_init(thread,
                           name,
                           entry,
                           parameter,
                           stack_start,
                           stack_size,
                           priority,
                           tick);
}
RTM_EXPORT(rt_thread_init);

/**
 * @brief 获得当前线程
 *
 * 该函数将返回当前线程的线程对象句柄
 *
 * @return 成功则返回当前线程对象句柄，如果调度器还未启动，则返回RT_NULL。
 *
 */
rt_thread_t rt_thread_self(void)
{
    return rt_current_thread;
}
RTM_EXPORT(rt_thread_self);

/**
 * @brief 启动线程
 *
 * 此函数将启动一个线程并将其放入系统就绪队列
 *
 * @param thread 被要被启动的线程句柄
 *
 * @return 成功返回 RT_EOK, 失败则返回 -RT_ERROR
 */
rt_err_t rt_thread_startup(rt_thread_t thread)
{
    /* thread check */
    RT_ASSERT(thread != RT_NULL);
    RT_ASSERT((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_INIT);

    /* set current priority to init priority */
    thread->current_priority = thread->init_priority;

    /* calculate priority attribute */
#if RT_THREAD_PRIORITY_MAX > 32
    thread->number      = thread->current_priority >> 3;            /* 5bit */
    thread->number_mask = 1L << thread->number;
    thread->high_mask   = 1L << (thread->current_priority & 0x07);  /* 3bit */
#else
    thread->number_mask = 1L << thread->current_priority;
#endif

    RT_DEBUG_LOG(RT_DEBUG_THREAD, ("startup a thread:%s with priority:%d\n",
                                   thread->name, thread->init_priority));
    /* change thread stat */
    thread->stat = RT_THREAD_SUSPEND;
    /* then resume it */
    rt_thread_resume(thread);
    if (rt_thread_self() != RT_NULL)
    {
        /* do a scheduling */
        rt_schedule();
    }

    return RT_EOK;
}
RTM_EXPORT(rt_thread_startup);

/**
 * @brief 脱离线程
 *
 * 该函数将脱离一个线程。 线程对象将从线程队列中删除，并从系统对象管理中脱离/删除。
 *
 * @param thread 线程句柄，它应该是由 rt_thread_init 进行初始化的线程句柄。
 *
 * @return 成功返回 RT_EOK, 失败则返回 -RT_ERROR
 */
rt_err_t rt_thread_detach(rt_thread_t thread)
{
    rt_base_t lock;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_INIT)
    {
        /* remove from schedule */
        rt_schedule_remove_thread(thread);
    }

    /* release thread timer */
    rt_timer_detach(&(thread->thread_timer));

    /* change stat */
    thread->stat = RT_THREAD_CLOSE;

    /* detach object */
    rt_object_detach((rt_object_t)thread);

    if (thread->cleanup != RT_NULL)
    {
        /* disable interrupt */
        lock = rt_hw_interrupt_disable();

        /* insert to defunct thread list */
        rt_list_insert_after(&rt_thread_defunct, &(thread->tlist));

        /* enable interrupt */
        rt_hw_interrupt_enable(lock);
    }

    return RT_EOK;
}
RTM_EXPORT(rt_thread_detach);


#ifdef RT_USING_HEAP
/**
 * @brief 创建线程
 *
 * 该函数将创建一个线程对象并分配线程对象内存和堆栈。
 *
 * @param name 线程的名称；线程名称的最大长度由 rtconfig.h 中的宏RT_NAME_MAX 指定，多余部分会被自动截掉。
 * @param entry 线程的入口函数
 * @param parameter 入口函数的传入参数
 * @param stack_size 线程堆栈的大小
 * @param priority 线程的优先级
 * @param tick 线程的时间片大小。当系统中存在相同优先级线程时，这个参数指定线程一次调度能够运行的最大时间长度。
 *
 * @return 被创建的线程对象句柄
 *
 */
rt_thread_t rt_thread_create(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick)
{
    struct rt_thread *thread;
    void *stack_start;

    thread = (struct rt_thread *)rt_object_allocate(RT_Object_Class_Thread,
                                                    name);
    if (thread == RT_NULL)
        return RT_NULL;

    stack_start = (void *)RT_KERNEL_MALLOC(stack_size);
    if (stack_start == RT_NULL)
    {
        /* allocate stack failure */
        rt_object_delete((rt_object_t)thread);

        return RT_NULL;
    }

    _rt_thread_init(thread,
                    name,
                    entry,
                    parameter,
                    stack_start,
                    stack_size,
                    priority,
                    tick);

    return thread;
}
RTM_EXPORT(rt_thread_create);

/**
 * @brief 删除线程
 *
 * 该函数将删除一个线程。 线程对象将从线程队列中删除，并从空闲线程中的系统对象管理中删除。
 *
 * @param thread 要删除的线程句柄
 *
 * @return 成功返回 RT_EOK, 失败则返回 -RT_ERROR
 */
rt_err_t rt_thread_delete(rt_thread_t thread)
{
    rt_base_t lock;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_INIT)
    {
        /* remove from schedule */
        rt_schedule_remove_thread(thread);
    }

    /* release thread timer */
    rt_timer_detach(&(thread->thread_timer));

    /* change stat */
    thread->stat = RT_THREAD_CLOSE;

    /* disable interrupt */
    lock = rt_hw_interrupt_disable();

    /* insert to defunct thread list */
    rt_list_insert_after(&rt_thread_defunct, &(thread->tlist));

    /* enable interrupt */
    rt_hw_interrupt_enable(lock);

    return RT_EOK;
}
RTM_EXPORT(rt_thread_delete);
#endif

/**
 * @brief 使线程让出处理器资源
 *
 * 该函数将使当前线程出让处理器资源，它将不再占有处理器，调度器会选择相同优先级的下一个线程执行。
 * 线程调用这个接口后，这个线程仍然在就绪队列中。
 *
 * @return RT_EOK
 */
rt_err_t rt_thread_yield(void)
{
    register rt_base_t level;
    struct rt_thread *thread;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    /* set to current thread */
    thread = rt_current_thread;

    /* if the thread stat is READY and on ready queue list */
    if ((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_READY &&
        thread->tlist.next != thread->tlist.prev)
    {
        /* remove thread from thread list */
        rt_list_remove(&(thread->tlist));

        /* put thread to end of ready queue */
        rt_list_insert_before(&(rt_thread_priority_table[thread->current_priority]),
                              &(thread->tlist));

        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        rt_schedule();

        return RT_EOK;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}
RTM_EXPORT(rt_thread_yield);

/**
 * @brief 线程睡眠
 *
 * 该函数将使当前线程睡眠几个系统始终节拍的时间
 *
 * @param tick 线程睡眠的时间
 *
 * @return RT_EOK
 */
rt_err_t rt_thread_sleep(rt_tick_t tick)
{
    register rt_base_t temp;
    struct rt_thread *thread;

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();
    /* set to current thread */
    thread = rt_current_thread;
    RT_ASSERT(thread != RT_NULL);

    /* suspend thread */
    rt_thread_suspend(thread);

    /* reset the timeout of thread timer and start it */
    rt_timer_control(&(thread->thread_timer), RT_TIMER_CTRL_SET_TIME, &tick);
    rt_timer_start(&(thread->thread_timer));

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    rt_schedule();

    /* clear error number of this thread to RT_EOK */
    if (thread->error == -RT_ETIMEOUT)
        thread->error = RT_EOK;

    return RT_EOK;
}

/**
 * @brief 线程延时
 *
 * 该函数将使当前线程延时几个系统始终节拍的时间。
 *
 * @param tick 延时的系统节拍数
 *
 * @return RT_EOK
 */
rt_err_t rt_thread_delay(rt_tick_t tick)
{
    return rt_thread_sleep(tick);
}
RTM_EXPORT(rt_thread_delay);

/**
 * @brief 线程毫秒延时
 *
 * 此函数将使当前线程延迟几毫秒。
 *
 * @param ms 延时时间
 *
 * @return RT_EOK
 */
rt_err_t rt_thread_mdelay(rt_int32_t ms)
{
    rt_tick_t tick;

    tick = rt_tick_from_millisecond(ms);

    return rt_thread_sleep(tick);
}
RTM_EXPORT(rt_thread_mdelay);

/**
 * @brief 控制线程
 *
 * 该函数将根据命令修改线程的一些配置参数。
 *
 * @param thread 被指定控制的线程
 * @param cmd 控制命令, 包含以下参数：
 *  RT_THREAD_CTRL_CHANGE_PRIORITY 改变线程的优先级
 *  RT_THREAD_CTRL_STARTUP 启动线程
 *  RT_THREAD_CTRL_CLOSE 删除线程
 * @param arg 控制命令相关的参数
 *
 * @return RT_EOK
 */
rt_err_t rt_thread_control(rt_thread_t thread, int cmd, void *arg)
{
    register rt_base_t temp;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);

    switch (cmd)
    {
    case RT_THREAD_CTRL_CHANGE_PRIORITY:
        /* disable interrupt */
        temp = rt_hw_interrupt_disable();

        /* for ready thread, change queue */
        if ((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_READY)
        {
            /* remove thread from schedule queue first */
            rt_schedule_remove_thread(thread);

            /* change thread priority */
            thread->current_priority = *(rt_uint8_t *)arg;

            /* recalculate priority attribute */
#if RT_THREAD_PRIORITY_MAX > 32
            thread->number      = thread->current_priority >> 3;            /* 5bit */
            thread->number_mask = 1 << thread->number;
            thread->high_mask   = 1 << (thread->current_priority & 0x07);   /* 3bit */
#else
            thread->number_mask = 1 << thread->current_priority;
#endif

            /* insert thread to schedule queue again */
            rt_schedule_insert_thread(thread);
        }
        else
        {
            thread->current_priority = *(rt_uint8_t *)arg;

            /* recalculate priority attribute */
#if RT_THREAD_PRIORITY_MAX > 32
            thread->number      = thread->current_priority >> 3;            /* 5bit */
            thread->number_mask = 1 << thread->number;
            thread->high_mask   = 1 << (thread->current_priority & 0x07);   /* 3bit */
#else
            thread->number_mask = 1 << thread->current_priority;
#endif
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);
        break;

    case RT_THREAD_CTRL_STARTUP:
        return rt_thread_startup(thread);

#ifdef RT_USING_HEAP
    case RT_THREAD_CTRL_CLOSE:
        return rt_thread_delete(thread);
#endif

    default:
        break;
    }

    return RT_EOK;
}
RTM_EXPORT(rt_thread_control);

/**
 * @brief 挂起线程
 *
 * 该函数将挂起指定的线程。
 *
 * @param thread 要被挂起的线程
 *
 * @return 成功返回 RT_EOK, 失败则返回 -RT_ERROR
 *
 * @note 如果挂起的是当前线程，该函数运行之后必须调用rt_schedule()函数。
 * 用户只需要了解该接口的作用，不推荐使用该接口。
 */
rt_err_t rt_thread_suspend(rt_thread_t thread)
{
    register rt_base_t temp;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);

    RT_DEBUG_LOG(RT_DEBUG_THREAD, ("thread suspend:  %s\n", thread->name));

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_READY)
    {
        RT_DEBUG_LOG(RT_DEBUG_THREAD, ("thread suspend: thread disorder, 0x%2x\n",
                                       thread->stat));

        return -RT_ERROR;
    }

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* change thread stat */
    thread->stat = RT_THREAD_SUSPEND | (thread->stat & ~RT_THREAD_STAT_MASK);
    rt_schedule_remove_thread(thread);

    /* stop thread timer anyway */
    rt_timer_stop(&(thread->thread_timer));

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    RT_OBJECT_HOOK_CALL(rt_thread_suspend_hook, (thread));
    return RT_EOK;
}
RTM_EXPORT(rt_thread_suspend);

/**
 * @brief 使线程恢复运行
 *
 * 线程恢复就是让挂起的线程重新进入就绪状态，并将线程放入系统的就绪队列中；
 * 如果被恢复线程在所有就绪态线程中，位于最高优先级链表的第一位，那么系统
 * 将进行线程上下文的切换。
 *
 * @param thread 将要被恢复的线程
 *
 * @return 成功返回 RT_EOK, 失败则返回 -RT_ERROR
 */
rt_err_t rt_thread_resume(rt_thread_t thread)
{
    register rt_base_t temp;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);

    RT_DEBUG_LOG(RT_DEBUG_THREAD, ("thread resume:  %s\n", thread->name));

    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_SUSPEND)
    {
        RT_DEBUG_LOG(RT_DEBUG_THREAD, ("thread resume: thread disorder, %d\n",
                                       thread->stat));

        return -RT_ERROR;
    }

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* remove from suspend list */
    rt_list_remove(&(thread->tlist));

    rt_timer_stop(&thread->thread_timer);

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    /* insert to schedule ready list */
    rt_schedule_insert_thread(thread);

    RT_OBJECT_HOOK_CALL(rt_thread_resume_hook, (thread));
    return RT_EOK;
}
RTM_EXPORT(rt_thread_resume);

/*
 * @brief 线程超时回调函数
 *
 * 此函数是线程的超时函数，通常在线程超时等待某些资源时调用。
 *
 * @param parameter 线程超时函数的传入参数
 */
void rt_thread_timeout(void *parameter)
{
    struct rt_thread *thread;

    thread = (struct rt_thread *)parameter;

    /* thread check */
    RT_ASSERT(thread != RT_NULL);
    RT_ASSERT((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_SUSPEND);

    /* set error number */
    thread->error = -RT_ETIMEOUT;

    /* remove from suspend list */
    rt_list_remove(&(thread->tlist));

    /* insert to schedule ready list */
    rt_schedule_insert_thread(thread);

    /* do schedule */
    rt_schedule();
}
RTM_EXPORT(rt_thread_timeout);

/*
 * @brief 线程查找函数
 *
 * 该函数将根据名称查找指定线程。
 *
 * @param name 需要查找的线程名称
 *
 * @return 查找到的线程
 *
 * @note 请不要在中断上下文中调用此函数。
 */
rt_thread_t rt_thread_find(char *name)
{
    struct rt_object_information *information;
    struct rt_object *object;
    struct rt_list_node *node;

    /* enter critical */
    if (rt_thread_self() != RT_NULL)
        rt_enter_critical();

    /* try to find device object */
    information = rt_object_get_information(RT_Object_Class_Thread);
    RT_ASSERT(information != RT_NULL);
    for (node  = information->object_list.next;
         node != &(information->object_list);
         node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (rt_strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            if (rt_thread_self() != RT_NULL)
                rt_exit_critical();

            return (rt_thread_t)object;
        }
    }

    /* leave critical */
    if (rt_thread_self() != RT_NULL)
        rt_exit_critical();

    /* not found */
    return RT_NULL;
}
RTM_EXPORT(rt_thread_find);

/**@}*/
