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
 * @brief IPC�����ʼ��
 *
 * �ú�������ʼ��һ�� IPC ����
 *
 * @param ipc ָ���� IPC ����
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
 * @brief ���̹߳��𵽶�������
 *
 * �ú���������һ���̵߳�ָ�������� IPC�����ĳЩ˫���ж�������ȣ�������������
 *
 * @param list ��IPC�Ϲ�����߳�����
 * @param thread Ҫ������̶߳���
 * @param flag IPC����ı�־λ��ӦΪRT_IPC_FLAG_FIFO / RT_IPC_FLAG_PRIO��
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
 * @brief �ָ�IPC�����Ϲ���ĵ�һ���߳�
 *
 *�˺������ָ�IPC�����б��еĵ�һ���̣߳�
 * - ��IPC����Ĺ��������ɾ���߳�
 * - ���̷߳���ϵͳ��������
 *
 * @param list ��������߳�����
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
 * @brief �ָ�IPC�����Ϲ���������߳�
 *
 *�˺������ָ��б������й�����̣߳�����IPC����Ĺ�������������˽������ȡ�
 *
 * @param list ��Ҫ�ָ����߳�����
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
 * @brief ��ʼ���ź���
 *
 * �ú�������ʼ���ź��������������ں˹������Ŀ���֮�¡�
 *
 * @param sem ָ�����ź���������
 * @param name �ź���������
 * @param value �ź�����ʼ��ʱ��ֵ
 * @param flag �ź����ı�־λ
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
 * @brief �����ź���
 *
 * �ú�������ָ���ľ�̬�ź������ں˶�����������Ƴ���
 *
 * @param sem ָ�����ź�������
 *
 * @return ����ɹ����� RT_EOK
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
 * @brief �����ź���
 *
 * ���øú���ʱ��ϵͳ���ȴӶ���������з���һ��semaphore���󣬲���ʼ���������
 * Ȼ���ʼ�� IPC �����Լ��� semaphore ��صĲ��֡���
 *
 * @param name �ź���������
 * @param value �ź�����ʼ��ʱ��ֵ
 * @param flag �ź����ı�־λ
 *
 * @return ���ش������ź������ƿ�, ������ʱ���� RT_NULL
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
 * @brief ɾ���ź���
 *
 * ϵͳ����ʹ���ź���ʱ����ͨ��ɾ���ź������ͷ�ϵͳ��Դ�������ڶ�̬�������ź�����
 * ���øú�����ɾ��һ���ź������󣬲��ͷ���ռ�õ��ڴ�ռ䡣
 *
 * @param sem �ź���������
 *
 * @return RT_EOKɾ���ɹ�
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
 * @brief ��ȡ�ź���
 *
 * �߳�ͨ�����øú�����ȡ�ź���������ź�����Դʵ�������ź���ֵ������ʱ���߳̽�����ź�����
 * ������Ӧ���ź���ֵ��� 1������ź�����ֵ�����㣬��ô˵����ǰ�ź�����Դʵ�������ã�����
 * ���ź������߳̽����� time ���������ѡ��ֱ�ӷ��ء������ȴ�һ��ʱ�䡢�����õȴ���ֱ��
 * �����̻߳��ж��ͷŸ��ź�����
 *
 * @param sem �ź�������ľ��
 * @param time ָ���ĵȴ�ʱ�䣬��λ�ǲ���ϵͳʱ�ӽ��ģ�OS Tick��
 *
 * @return RT_EOK �ɹ�����ź�����-RT_ETIMEOUT ��ʱ��Ȼδ����ź�����-RT_ERROR ��������
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
 * @brief �޵ȴ���ȡ�ź���
 *
 * ���û�������������ź����Ϲ����߳̽��еȴ�ʱ�����Ե��øú������޵ȴ���ʽ��ȡ�ź�����
 *
 * @param sem �ź�������ľ��
 *
 * @return RT_EOK �ɹ�����ź�����-RT_ETIMEOUT ��ȡʧ��
 */
rt_err_t rt_sem_trytake(rt_sem_t sem)
{
    return rt_sem_take(sem, 0);
}
RTM_EXPORT(rt_sem_trytake);

/**
 * @brief �ͷ��ź���
 *
 * �ú������ͷ�һ���ź��������ź�����ֵ������ʱ���������̵߳ȴ�����ź���ʱ��
 * �ͷ��ź��������ѵȴ��ڸ��ź����̶߳����еĵ�һ���̣߳�������ȡ�ź���������
 * �����ź�����ֵ��һ��
 *
 * @param sem �ź�������ľ��
 *
 * @return RT_EOK �ɹ��ͷ��ź���
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
 * @brief �ź�������
 *
 * �ú������Ի�ȡ�������ź��������һЩ�������ԡ�
 *
 * @param sem �ź�������ľ��
 * @param cmd ִ������
 * @param arg ִ������Ĳ���
 *
 * @return �������
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
 * @brief ��ʼ��������
 *
 * �ú�������ʼ�������������������ں˹������Ŀ���֮��
 *
 * @param mutex ����������ľ��
 * @param name ������������
 * @param flag �������ı�־λ
 *
 * @return RT_EOK ��ʼ���ɹ�
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
 * @brief ���뻥����
 *
 * �ú������ѻ�����������ں˶����������ɾ���������ھ�̬��ʼ���Ļ�������
 *
 * @param mutex ����������ľ��
 *
 * @return RT_EOK �ɹ�
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
 * @brief ����������
 *
 * ���øú�������һ��������ʱ���ں����ȴ���һ�����������ƿ飬Ȼ����ɶԸÿ��ƿ�ĳ�ʼ��������
 *
 * @param name ������������
 * @param flag �������ı�־λ
 *
 * @return ���ش����Ļ�������ʧ��ʱ����RT_NULL
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
 * @brief ɾ��������
 *
 * ��ϵͳ����ʹ�û�����ʱ��ͨ�����øú���ɾ�����������ͷ�ϵͳ��Դ�������ڶ�̬�����Ļ�������
 *
 * @param mutex ����������ľ��
 *
 * @return RT_EOK ɾ���ɹ�
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
 * @brief ��ȡ������
 *
 * �ú�������ȡһ�������������������û�б������߳̿��ƣ���ô����û��������߳̽��ɹ���øû�������
 * ����������Ѿ�����ǰ�߳��߳̿��ƣ���û������ĳ��м����� 1����ǰ�߳�Ҳ�������ȴ������������
 * �Ѿ��������߳�ռ�У���ǰ�߳��ڸû������Ϲ���ȴ���ֱ�������߳��ͷ������ߵȴ�ʱ�䳬��ָ����
 * ��ʱʱ�䡣
 *
 * @param mutex ����������ľ��
 * @param time ָ���ĵȴ�ʱ��
 *
 * @return RT_EOK �ɹ���û�������-RT_ETIMEOUT ��ʱ��-RT_ERROR ��ȡʧ��
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
 * @brief �ͷŻ�����
 *
 * ʹ�øú����ӿ�ʱ��ֻ���Ѿ�ӵ�л���������Ȩ���̲߳����ͷ�����ÿ�ͷ�һ�θû�������
 * ���ĳ��м����ͼ� 1�����û������ĳ��м���Ϊ��ʱ���������߳��Ѿ��ͷ����еĳ��в�������
 * ����Ϊ���ã��ȴ��ڸ��ź����ϵ��߳̽������ѡ�����̵߳��������ȼ���������������
 * ��ô�����������ͷź��ָ̻߳�Ϊ���л�����ǰ�����ȼ���
 *
 * @param mutex ����������ľ��
 *
 * @return RT_EOK �ɹ�
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
 * @brief �����������޸�
 *
 * �ú������Ի�ȡ�����û�������һЩ�������ԡ�
 *
 * @param mutex ����������ľ��
 * @param cmd ִ������
 * @param arg ִ��������صĲ���
 *
 * @return �������
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
 * @brief ��ʼ���¼���
 *
 * �ú�������ʼ���¼������󣬲����뵽ϵͳ���������н��й���
 *
 * @param event �¼�������ľ��
 * @param name �¼���������
 * @param flag �¼����ı�־��������ȡֵ��RT_IPC_FLAG_FIFO��RT_IPC_FLAG_PRIO
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
 * @brief �����¼���
 *
 * �ú�������һ���¼����ں˹�����������
 *
 * @param event �¼�����ľ��
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
 * @brief �����¼���
 *
 * ���øú����ӿ�ʱ��ϵͳ��Ӷ���������з����¼�����Ȼ����ж���ĳ�ʼ����
 * IPC�����ʼ��������set���ó�0��
 *
 * @param name �¼���������
 * @param flag �¼����ı�־��������ȡֵ��RT_IPC_FLAG_FIFO��RT_IPC_FLAG_PRIO
 *
 * @return ���ش������¼�����ľ��������ʧ�ܷ��� RT_NULL
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
 * @brief ɾ���¼���
 *
 * ϵͳ����ʹ��rt_event_create() �������¼�������ʱ��ͨ�����øú���ɾ���¼�������
 * ���ƿ����ͷ�ϵͳ��Դ��
 *
 * @param event �¼�������ľ��
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
 * @brief �����¼�
 *
 * ʹ�øú����ӿ�ʱ��ͨ������ set ָ�����¼���־���趨 event �¼���������¼���־ֵ��
 * Ȼ������ȴ���event�¼��������ϵĵȴ��߳������ж��Ƿ����̵߳��¼�����Ҫ���뵱ǰ 
 * event�����¼���־ֵƥ�䣬����У����Ѹ��̡߳�
 *
 * @param event �¼�������ľ��
 * @param set ���͵�һ�������¼��ı�־ֵ
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
 * @brief �����¼�
 *
 * ���û���������ӿ�ʱ��ϵͳ���ȸ���set�����ͽ���ѡ��option���ж���Ҫ���յ��¼��Ƿ�����
 * ����Ѿ�����������ݲ���option���Ƿ������� RT_EVENT_FLAG_CLEAR �������Ƿ������¼�����Ӧ
 * ��־λ��Ȼ�󷵻أ�����recved���������յ����¼��������û�з�������ѵȴ���set��option
 * ���������̱߳���Ľṹ�У�Ȼ����̹߳����ڴ��¼��ϣ�ֱ����ȴ����¼�����������ȴ�ʱ��
 * ����ָ���ĳ�ʱʱ�䡣�����ʱʱ������Ϊ�㣬���ʾ���߳�Ҫ���ܵ��¼�û��������Ҫ��ʱ�Ͳ�
 * �ȴ�����ֱ�ӷ���-RT_ETIMEOUT��
 *
 * @param event �¼�������ľ��
 * @param set �����̸߳���Ȥ���¼�
 * @param option ����ѡ�Ӧ���ý���ѡ��RT_EVENT_FLAG_AND��RT_EVENT_FLAG_OR��
 * @param timeout ָ����ʱʱ��
 * @param recved ָ���յ����¼������������Ȥ����������Ϊ RT_NULL��
 *
 * @return RT_EOK �ɹ���-RT_ETIMEOUT ��ʱ��-RT_ERROR ����
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
 * @brief �¼��������޸�
 *
 * �ú������Ի�ȡ�������¼������һЩ�������ԡ�
 *
 * @param event �¼�����ľ��
 * @param cmd ִ������
 * @param arg ִ������Ĳ���
 *
 * @return �������
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
* @brief ��ʼ������
 *
 * �ú�������ʼ�����䲢���������ں˹������Ŀ���֮�¡�
 *
 * @param mb �������ľ��
 * @param name ���������
 * @param msgpool ������ָ��
 * @param size ��������
 * @param flag �����־��������ȡ��ֵ��RT_IPC_FLAG_FIFO��RT_IPC_FLAG_PRIO
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
* @brief ��������
 *
 * �ú��������Ѿ�̬��ʼ�������������ں˶����������ɾ����ʹ�øú����ӿں�
 * �ں��Ȼ������й��ڸ������ϵ��̣߳��̻߳�÷���ֵ�� - RT_ERROR ����Ȼ��
 * �����������ں˶����������ɾ����
 *
 * @param mb �������ľ��
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
 * @brief ��������
 *
 * ���øú��������������ʱ���ȴӶ���������з���һ���������Ȼ������䶯̬����
 * һ���ڴ�ռ���������ʼ�������ڴ�Ĵ�С�����ʼ���С��4 �ֽڣ������������ĳ˻���
 * ���ų�ʼ�������ʼ���Ŀ�ͷ����ʼ��������е�ƫ������
 *
 * @param name ���������
 * @param size ��������
 * @param flag �����־��������ȡֵ��RT_IPC_FLAG_FIFO �� RT_IPC_FLAG_PRIO
 *
* @return ���ش�����������󣬴���ʧ��ʱ���� RT_NULL
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
 * @brief ɾ������
 *
 * ����rt_mb_create()���������䲻�ٱ�ʹ��ʱ��Ӧ�õ��øú���ɾ�������ͷ���Ӧ��ϵͳ��Դ��
 * һ��������ɣ����佫�������Ե�ɾ����ɾ������ʱ��������̱߳������ڸ���������ϣ�
 * �ں��Ȼ��ѹ����ڸ������ϵ������̣߳��̷߳���ֵ��- RT_ERROR����Ȼ�����ͷ�����ʹ�õ�
 * �ڴ棬���ɾ���������
 *
 * @param mb �������ľ��
 *
 * @return �������
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
 * @brief �ȴ���ʽ�����ʼ�
 *
 * �û�Ҳ����ͨ�����øú����ӿ���ָ�����䷢���ʼ������� rt_mb_send()���������ڣ�
 * ��������Ѿ����ˣ���ô�����߳̽������趨��timeout�����ȴ���������Ϊ��ȡ�ʼ�
 * ���ճ��ռ䡣������õĳ�ʱʱ�䵽����Ȼû�пճ��ռ䣬��ʱ�����߳̽������ѷ���
 * �����롣
 *
 * @param mb �������ľ��
 * @param value �ʼ�����
 * @param timeout ��ʱʱ��
 *
 * @return RT_EOK ���ͳɹ���-RT_ETIMEOUT ��ʱ��-RT_ERROR ʧ�ܣ����ش����
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
 * @brief �����ʼ�
 *
 * �̻߳����жϷ���������ͨ�����øú����������̷߳����ʼ������͵��ʼ������� 32 λ
 * �����ʽ�����ݣ�һ������ֵ����һ��ָ�򻺳�����ָ�롣�������е��ʼ��Ѿ���ʱ������
 * �ʼ����̻߳����жϳ�����յ�- RT_EFULL �ķ���ֵ��
 *
 * @param mb �������ľ��
 * @param value �ʼ�����
 *
 * @return RT_EOK ���ͳɹ���-RT_EFULL �����Ѿ�����
 */
rt_err_t rt_mb_send(rt_mailbox_t mb, rt_uint32_t value)
{
    return rt_mb_send_wait(mb, value, 0);
}
RTM_EXPORT(rt_mb_send);

/**
 * @brief �����ʼ�
 *
 * ֻ�е������߽��յ����������ʼ�ʱ�������߲�������ȡ���ʼ������� RT_EOK �ķ���ֵ��
 * ��������̻߳���ݳ�ʱʱ�����ã������������ĵȴ��̶߳����ϣ���ֱ�ӷ��ء�
 *
 * @param mb �������ľ��
 * @param value �ʼ�����
 * @param timeout ��ʱʱ��
 *
 * @return RT_EOK ���ͳɹ���-RT_ETIMEOUT ��ʱ��-RT_ERROR ʧ�ܣ����ش����
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
 * @brief ����������޸�
 *
 * �ú������Ի�ȡ��������������һЩ�������ԡ�
 *
 * @param mb �������ľ��
 * @param cmd ִ������
 * @param arg ִ��������صĲ���
 *
 * @return �������
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
 * @brief ��ʼ����Ϣ����
 *
 * �ú�������ʼ����Ϣ���в����������ں˹������Ŀ���֮�¡�
 *
 * @param mq ��Ϣ���ж�����
 * @param name ��Ϣ���е�����
 * @param msgpool ���ڴ����Ϣ�Ļ�����ָ��
 * @param msg_size ��Ϣ������һ����Ϣ����󳤶ȣ���λ�ֽ�
 * @param pool_size �����Ϣ�Ļ�������С
 * @param flag ��Ϣ���в��õĵȴ���ʽ��������ȡֵ��RT_IPC_FLAG_FIFO��RT_IPC_FLAG_PRIO
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
 * @brief ������Ϣ����
 *
 * �ú�����ʹ��Ϣ���ж��󱻴��ں˶����������ɾ����ʹ�øú����ӿں�
 * �ں��Ȼ������й��ڸ���Ϣ�ȴ����ж����ϵ��̣߳�����ֵ�� - RT_ERROR ����
 * Ȼ�󽫸���Ϣ���ж�����ں˶����������ɾ����
 *
 * @param mq ��Ϣ���ж�����
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
 * @brief ������Ϣ����
 *
 * ���øú���������Ϣ����ʱ�ȴӶ���������з���һ����Ϣ���ж���Ȼ��
 * ����Ϣ���ж������һ���ڴ�ռ䣬��֯�ɿ�����Ϣ��������ڴ�Ĵ�С= 
 * [��Ϣ��С+��Ϣͷ�������������ӣ�] *��Ϣ�����������������ٳ�ʼ��
 * ��Ϣ���У���ʱ��Ϣ����Ϊ�ա�
 *
 * @param name ��Ϣ���е�����
 * @param msg_size ��Ϣ������һ����Ϣ����󳤶ȣ���λ�ֽ�
 * @param max_msgs ��Ϣ���е�������
 * @param flag ��Ϣ���в��õĵȴ���ʽ��������ȡֵ��RT_IPC_FLAG_FIFO��RT_IPC_FLAG_PRIO
 *
 * @return ���ش�������Ϣ���У�����������ʱ���� RT_NULL
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
 * @brief ɾ����Ϣ����
 *
 * ����Ϣ���в��ٱ�ʹ��ʱ��Ӧ�õ��øú����ӿ�ɾ�������ͷ�ϵͳ��Դ��һ��������ɣ�
 * ��Ϣ���н��������Ե�ɾ����ɾ����Ϣ����ʱ��������̱߳������ڸ���Ϣ���еȴ������ϣ�
 * ���ں��Ȼ��ѹ����ڸ���Ϣ�ȴ������ϵ������̣߳�����ֵ�� -RT_ERROR����Ȼ�����ͷ�
 * ��Ϣ����ʹ�õ��ڴ棬���ɾ����Ϣ���ж���
 *
 * @param mq ��Ϣ���ж�����
 *
 * @return RT_EOK �ɹ�
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
 * @brief ������Ϣ
 *
 * �̻߳����жϷ�����򶼿��Ե��øú�������Ϣ���з�����Ϣ����������Ϣʱ��
 * ��Ϣ���ж����ȴӿ�����Ϣ������ȡ��һ��������Ϣ�飬���̻߳����жϷ���
 * �����͵���Ϣ���ݸ��Ƶ���Ϣ���ϣ�Ȼ��Ѹ���Ϣ��ҵ���Ϣ���е�β����
 * ���ҽ���������Ϣ�������п��õĿ�����Ϣ��ʱ�������߲��ܳɹ�������Ϣ��
 * ��������Ϣ�������޿�����Ϣ�飬˵����Ϣ������������ʱ��������Ϣ�ĵ�
 * �̻߳����жϳ�����յ�һ�������루-RT_EFULL����
 *
 * @param mq ��Ϣ���ж�����
 * @param buffer ��Ϣ����
 * @param size ��Ϣ��С
 *
 * @return RT_EOK �ɹ���-RT_EFULL ��Ϣ����������-RT_ERROR ʧ�ܣ���ʾ���͵�
 * ��Ϣ���ȴ�����Ϣ��������Ϣ����󳤶�
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
 * @brief ���ͽ�����Ϣ
 *
 * ���ͽ�����Ϣ�Ĺ����뷢����Ϣ����һ����Ψһ�Ĳ�ͬ�ǣ������ͽ�����Ϣʱ��
 * �ӿ�����Ϣ������ȡ��������Ϣ�鲻�ǹҵ���Ϣ���еĶ�β�����ǹҵ����ף�
 * �����������߾��ܹ����Ƚ��յ�������Ϣ���Ӷ���ʱ������Ϣ����
 *
 * @param mq ��Ϣ���ж�����
 * @param buffer ��Ϣ����
 * @param size ��Ϣ��С
 *
 * @return RT_EOK �ɹ���-RT_EFULL ��Ϣ����������-RT_ERROR ʧ��
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
 * @brief ������Ϣ
 *
 * ����Ϣ����������Ϣʱ�������߲��ܽ�����Ϣ����������߻���ݳ�ʱʱ�����ã�
 * ���������Ϣ���еĵȴ��̶߳����ϣ���ֱ�ӷ��ء�
 *
 * @param mq ��Ϣ���ж�����
 * @param buffer ��Ϣ����
 * @param size ��Ϣ��С
 * @param timeout ָ���ĳ�ʱʱ��
 *
 * @return RT_EOK �ɹ���-RT_ETIMEOUT ��ʱ��-RT_ERROR ʧ��
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
 * @brief ��Ϣ���ж���������޸�
 *
 * �ú������Ի�ȡ��������Ϣ���ж����һЩ�������ԡ�
 *
 * @param mq ��Ϣ���ж�����
 * @param cmd ִ�е�����
 * @param arg ִ��������صĲ���
 *
 * @return �������
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
