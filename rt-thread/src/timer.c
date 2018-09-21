/*
 * File      : timer.c
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
 * 2006-03-12     Bernard      first version
 * 2006-04-29     Bernard      implement thread timer
 * 2006-06-04     Bernard      implement rt_timer_control
 * 2006-08-10     Bernard      fix the periodic timer bug
 * 2006-09-03     Bernard      implement rt_timer_detach
 * 2009-11-11     LiJin        add soft timer
 * 2010-05-12     Bernard      fix the timer check bug.
 * 2010-11-02     Charlie      re-implement tick overflow issue
 * 2012-12-15     Bernard      fix the next timeout issue in soft timer
 * 2014-07-12     Bernard      does not lock scheduler when invoking soft-timer
 *                             timeout function.
 */

#include <rtthread.h>
#include <rthw.h>

/* hard timer list */
static rt_list_t rt_timer_list[RT_TIMER_SKIP_LIST_LEVEL];

#ifdef RT_USING_TIMER_SOFT
#ifndef RT_TIMER_THREAD_STACK_SIZE
#define RT_TIMER_THREAD_STACK_SIZE     512
#endif

#ifndef RT_TIMER_THREAD_PRIO
#define RT_TIMER_THREAD_PRIO           0
#endif

/* soft timer list */
static rt_list_t rt_soft_timer_list[RT_TIMER_SKIP_LIST_LEVEL];
static struct rt_thread timer_thread;
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t timer_thread_stack[RT_TIMER_THREAD_STACK_SIZE];
#endif

#ifdef RT_USING_HOOK
extern void (*rt_object_take_hook)(struct rt_object *object);
extern void (*rt_object_put_hook)(struct rt_object *object);
static void (*rt_timer_timeout_hook)(struct rt_timer *timer);

/**
 * @addtogroup Hook
 */

/**@{*/

/**
 * �ú���������һ�����Ӻ���������ʱ����ʱʱ�����øù��Ӻ�����
 *
 * @param hook ���Ӻ���
 */
void rt_timer_timeout_sethook(void (*hook)(struct rt_timer *timer))
{
    rt_timer_timeout_hook = hook;
}

/**@}*/
#endif

static void _rt_timer_init(rt_timer_t timer,
                           void (*timeout)(void *parameter),
                           void      *parameter,
                           rt_tick_t  time,
                           rt_uint8_t flag)
{
    int i;

    /* set flag */
    timer->parent.flag  = flag;

    /* set deactivated */
    timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;

    timer->timeout_func = timeout;
    timer->parameter    = parameter;

    timer->timeout_tick = 0;
    timer->init_tick    = time;

    /* initialize timer list */
    for (i = 0; i < RT_TIMER_SKIP_LIST_LEVEL; i++)
    {
        rt_list_init(&(timer->row[i]));
    }
}

/* the fist timer always in the last row */
static rt_tick_t rt_timer_list_next_timeout(rt_list_t timer_list[])
{
    struct rt_timer *timer;

    if (rt_list_isempty(&timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]))
        return RT_TICK_MAX;

    timer = rt_list_entry(timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1].next,
                          struct rt_timer, row[RT_TIMER_SKIP_LIST_LEVEL - 1]);

    return timer->timeout_tick;
}

rt_inline void _rt_timer_remove(rt_timer_t timer)
{
    int i;

    for (i = 0; i < RT_TIMER_SKIP_LIST_LEVEL; i++)
    {
        rt_list_remove(&timer->row[i]);
    }
}

#if RT_DEBUG_TIMER
static int rt_timer_count_height(struct rt_timer *timer)
{
    int i, cnt = 0;

    for (i = 0; i < RT_TIMER_SKIP_LIST_LEVEL; i++)
    {
        if (!rt_list_isempty(&timer->row[i]))
            cnt++;
    }
    return cnt;
}

void rt_timer_dump(rt_list_t timer_heads[])
{
    rt_list_t *list;

    for (list = timer_heads[RT_TIMER_SKIP_LIST_LEVEL - 1].next;
         list != &timer_heads[RT_TIMER_SKIP_LIST_LEVEL - 1];
         list = list->next)
    {
        struct rt_timer *timer = rt_list_entry(list,
                                               struct rt_timer,
                                               row[RT_TIMER_SKIP_LIST_LEVEL - 1]);
        rt_kprintf("%d", rt_timer_count_height(timer));
    }
    rt_kprintf("\n");
}
#endif

/**
 * @addtogroup Timer
 */

/**@{*/

/**
 * @brief ��ʼ����ʱ��
 *
 * �ú�������ʼ��һ����ʱ����ͨ���ú������ڳ�ʼ��һ����̬��ʱ������
 *
 * @param timer ��̬��ʱ��������
 * @param name ��ʱ��������
 * @param timeout ��ʱ�ص�����
 * @param parameter ��ʱ�ص������Ĵ������
 * @param time ��ʱ���ĳ�ʱ�δ���
 * @param flag ��ʱ������ʱ�Ĳ�����֧�ֵ�ֵ�������ζ�ʱ�����ڶ�ʱ��
  * Ӳ����ʱ���������ʱ���������� ���� ��ϵȡ���ֵ��
 */
void rt_timer_init(rt_timer_t  timer,
                   const char *name,
                   void (*timeout)(void *parameter),
                   void       *parameter,
                   rt_tick_t   time,
                   rt_uint8_t  flag)
{
    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* timer object initialization */
    rt_object_init((rt_object_t)timer, RT_Object_Class_Timer, name);

    _rt_timer_init(timer, timeout, parameter, time, flag);
}
RTM_EXPORT(rt_timer_init);

/**
 * @brief ���붨ʱ��
 *
 * ��һ����̬��ʱ������Ҫ��ʹ��ʱ������ʹ�øú����ӿڰѶ�ʱ�������ϵͳ�����Ķ�ʱ��������ɾ����
 * ���Ƕ�ʱ��������ռ�е��ڴ治�ᱻ�ͷš�
 *
 * @param timer ��̬��ʱ��������
 *
 * @return �ɹ� OK; �����򷵻� RT_ERROR
 */
rt_err_t rt_timer_detach(rt_timer_t timer)
{
    register rt_base_t level;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    _rt_timer_remove(timer);

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    rt_object_detach((rt_object_t)timer);

    return -RT_EOK;
}
RTM_EXPORT(rt_timer_detach);

#ifdef RT_USING_HEAP
/**
 * @brief ������ʱ��
 *
 * ���øú����ӿں��ں����ȴӶ�̬�ڴ���з���һ����ʱ�����ƿ飬Ȼ��Ըÿ��ƿ���л����ĳ�ʼ����
 *
 * @param name ��ʱ������
 * @param timeout ��ʱ�ص�����
 * @param parameter ��ʱ�ص������Ĵ������
 * @param time ��ʱ��ʱʱ��
 * @param flag ��ʱ���ı�־λ
 *
 * @return �����Ķ�ʱ�����
 */
rt_timer_t rt_timer_create(const char *name,
                           void (*timeout)(void *parameter),
                           void       *parameter,
                           rt_tick_t   time,
                           rt_uint8_t  flag)
{
    struct rt_timer *timer;

    /* allocate a object */
    timer = (struct rt_timer *)rt_object_allocate(RT_Object_Class_Timer, name);
    if (timer == RT_NULL)
    {
        return RT_NULL;
    }

    _rt_timer_init(timer, timeout, parameter, time, flag);

    return timer;
}
RTM_EXPORT(rt_timer_create);

/**
 * @brief ɾ����ʱ��
 *
 * ϵͳ����ʹ�ö�̬��ʱ��ʱ����ʹ�øú����ӿڣ�ϵͳ��������ʱ���� 
 * rt_timer_list ������ɾ����Ȼ���ͷ���Ӧ�Ķ�ʱ�����ƿ�ռ�е��ڴ档
 *
 * @param timer ��Ҫ��ɾ���Ķ�ʱ�����
 *
 * @return �ɹ� RT_EOK ; �����򷵻� -RT_ERROR
 */
rt_err_t rt_timer_delete(rt_timer_t timer)
{
    register rt_base_t level;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    _rt_timer_remove(timer);

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    rt_object_delete((rt_object_t)timer);

    return -RT_EOK;
}
RTM_EXPORT(rt_timer_delete);
#endif

/**
 * @brief ������ʱ��
 *
 * ����ʱ�����������߳�ʼ���Ժ󣬲����ᱻ���������������ڵ��øú����ӿں󣬲ſ�ʼ������
 *
 * @param timer ��Ҫ�������Ķ�ʱ��
 *
 * @return �ɹ� RT_EOK ; �����򷵻� -RT_ERROR
 */
rt_err_t rt_timer_start(rt_timer_t timer)
{
    unsigned int row_lvl;
    rt_list_t *timer_list;
    register rt_base_t level;
    rt_list_t *row_head[RT_TIMER_SKIP_LIST_LEVEL];
    unsigned int tst_nr;
    static unsigned int random_nr;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* stop timer firstly */
    level = rt_hw_interrupt_disable();
    /* remove timer from list */
    _rt_timer_remove(timer);
    /* change status of timer */
    timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;
    rt_hw_interrupt_enable(level);

    RT_OBJECT_HOOK_CALL(rt_object_take_hook, (&(timer->parent)));

    /*
     * get timeout tick,
     * the max timeout tick shall not great than RT_TICK_MAX/2
     */
    RT_ASSERT(timer->init_tick < RT_TICK_MAX / 2);
    timer->timeout_tick = rt_tick_get() + timer->init_tick;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

#ifdef RT_USING_TIMER_SOFT
    if (timer->parent.flag & RT_TIMER_FLAG_SOFT_TIMER)
    {
        /* insert timer to soft timer list */
        timer_list = rt_soft_timer_list;
    }
    else
#endif
    {
        /* insert timer to system timer list */
        timer_list = rt_timer_list;
    }

    row_head[0]  = &timer_list[0];
    for (row_lvl = 0; row_lvl < RT_TIMER_SKIP_LIST_LEVEL; row_lvl++)
    {
        for (; row_head[row_lvl] != timer_list[row_lvl].prev;
             row_head[row_lvl]  = row_head[row_lvl]->next)
        {
            struct rt_timer *t;
            rt_list_t *p = row_head[row_lvl]->next;

            /* fix up the entry pointer */
            t = rt_list_entry(p, struct rt_timer, row[row_lvl]);

            /* If we have two timers that timeout at the same time, it's
             * preferred that the timer inserted early get called early.
             * So insert the new timer to the end the the some-timeout timer
             * list.
             */
            if ((t->timeout_tick - timer->timeout_tick) == 0)
            {
                continue;
            }
            else if ((t->timeout_tick - timer->timeout_tick) < RT_TICK_MAX / 2)
            {
                break;
            }
        }
        if (row_lvl != RT_TIMER_SKIP_LIST_LEVEL - 1)
            row_head[row_lvl + 1] = row_head[row_lvl] + 1;
    }

    /* Interestingly, this super simple timer insert counter works very very
     * well on distributing the list height uniformly. By means of "very very
     * well", I mean it beats the randomness of timer->timeout_tick very easily
     * (actually, the timeout_tick is not random and easy to be attacked). */
    random_nr++;
    tst_nr = random_nr;

    rt_list_insert_after(row_head[RT_TIMER_SKIP_LIST_LEVEL - 1],
                         &(timer->row[RT_TIMER_SKIP_LIST_LEVEL - 1]));
    for (row_lvl = 2; row_lvl <= RT_TIMER_SKIP_LIST_LEVEL; row_lvl++)
    {
        if (!(tst_nr & RT_TIMER_SKIP_LIST_MASK))
            rt_list_insert_after(row_head[RT_TIMER_SKIP_LIST_LEVEL - row_lvl],
                                 &(timer->row[RT_TIMER_SKIP_LIST_LEVEL - row_lvl]));
        else
            break;
        /* Shift over the bits we have tested. Works well with 1 bit and 2
         * bits. */
        tst_nr >>= (RT_TIMER_SKIP_LIST_MASK + 1) >> 1;
    }

    timer->parent.flag |= RT_TIMER_FLAG_ACTIVATED;

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

#ifdef RT_USING_TIMER_SOFT
    if (timer->parent.flag & RT_TIMER_FLAG_SOFT_TIMER)
    {
        /* check whether timer thread is ready */
        if ((timer_thread.stat & RT_THREAD_STAT_MASK) != RT_THREAD_READY)
        {
            /* resume timer thread to check soft timer */
            rt_thread_resume(&timer_thread);
            rt_schedule();
        }
    }
#endif

    return -RT_EOK;
}
RTM_EXPORT(rt_timer_start);

/**
 * @brief ֹͣ��ʱ��
 *
 * ������ʱ���Ժ�����ʹ��ֹͣ�����Ե��øú����ӿڡ�
 *
 * @param timer ��ʱ�������ָ��Ҫֹͣ�Ķ�ʱ�����ƿ�
 *
 * @return �ɹ��򷵻� RT_EOK ; ���� -RT_ERROR ʱ��˵���Ѿ�����ֹͣ״̬
 */
rt_err_t rt_timer_stop(rt_timer_t timer)
{
    register rt_base_t level;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);
    if (!(timer->parent.flag & RT_TIMER_FLAG_ACTIVATED))
        return -RT_ERROR;

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(timer->parent)));

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    _rt_timer_remove(timer);

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    /* change stat */
    timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;

    return RT_EOK;
}
RTM_EXPORT(rt_timer_stop);

/**
 * @brief ���ƶ�ʱ��
 *
 * �ú����ɸ����������Ͳ��������鿴��ı䶨ʱ�������á�
 *
 * @param timer ��ʱ�������ָ��Ҫ���ƵĶ�ʱ�����ƿ�
 * @param cmd ��������
 * @param arg ����������صĲ���
 *
 * @return RT_EOK
 */
rt_err_t rt_timer_control(rt_timer_t timer, int cmd, void *arg)
{
    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    switch (cmd)
    {
    case RT_TIMER_CTRL_GET_TIME:
        *(rt_tick_t *)arg = timer->init_tick;
        break;

    case RT_TIMER_CTRL_SET_TIME:
        timer->init_tick = *(rt_tick_t *)arg;
        break;

    case RT_TIMER_CTRL_SET_ONESHOT:
        timer->parent.flag &= ~RT_TIMER_FLAG_PERIODIC;
        break;

    case RT_TIMER_CTRL_SET_PERIODIC:
        timer->parent.flag |= RT_TIMER_FLAG_PERIODIC;
        break;
    }

    return RT_EOK;
}
RTM_EXPORT(rt_timer_control);

/*
 * �ú�������鶨ʱ������������ж�ʱ����ʱ�¼���������ô��Ӧ�ĳ�ʱ�����������С�
 *
 * @note �ú���Ӧ����ϵͳ��ʱ���ж��е���
 */
void rt_timer_check(void)
{
    struct rt_timer *t;
    rt_tick_t current_tick;
    register rt_base_t level;

    RT_DEBUG_LOG(RT_DEBUG_TIMER, ("timer check enter\n"));

    current_tick = rt_tick_get();

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    while (!rt_list_isempty(&rt_timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]))
    {
        t = rt_list_entry(rt_timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1].next,
                          struct rt_timer, row[RT_TIMER_SKIP_LIST_LEVEL - 1]);

        /*
         * It supposes that the new tick shall less than the half duration of
         * tick max.
         */
        if ((current_tick - t->timeout_tick) < RT_TICK_MAX / 2)
        {
            RT_OBJECT_HOOK_CALL(rt_timer_timeout_hook, (t));

            /* remove timer from timer list firstly */
            _rt_timer_remove(t);

            /* call timeout function */
            t->timeout_func(t->parameter);

            /* re-get tick */
            current_tick = rt_tick_get();

            RT_DEBUG_LOG(RT_DEBUG_TIMER, ("current tick: %d\n", current_tick));

            if ((t->parent.flag & RT_TIMER_FLAG_PERIODIC) &&
                (t->parent.flag & RT_TIMER_FLAG_ACTIVATED))
            {
                /* start it */
                t->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;
                rt_timer_start(t);
            }
            else
            {
                /* stop timer */
                t->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;
            }
        }
        else
            break;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    RT_DEBUG_LOG(RT_DEBUG_TIMER, ("timer check leave\n"));
}

/*
 * �ú��������ض�ʱ������һ����ʱʱ�䣨ϵͳ�δ����ֵ��
 *
 * @return һ�¸���ʱ���ĳ�ʱϵͳ�δ����ֵ
 */
rt_tick_t rt_timer_next_timeout_tick(void)
{
    return rt_timer_list_next_timeout(rt_timer_list);
}

#ifdef RT_USING_TIMER_SOFT
/**
 * �ú�������鶨ʱ������������ж�ʱ����ʱ�¼���������ô��Ӧ�ĳ�ʱ�����������С�
 */
void rt_soft_timer_check(void)
{
    rt_tick_t current_tick;
    rt_list_t *n;
    struct rt_timer *t;

    RT_DEBUG_LOG(RT_DEBUG_TIMER, ("software timer check enter\n"));

    current_tick = rt_tick_get();

    /* lock scheduler */
    rt_enter_critical();

    for (n = rt_soft_timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1].next;
         n != &(rt_soft_timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]);)
    {
        t = rt_list_entry(n, struct rt_timer, row[RT_TIMER_SKIP_LIST_LEVEL - 1]);

        /*
         * It supposes that the new tick shall less than the half duration of
         * tick max.
         */
        if ((current_tick - t->timeout_tick) < RT_TICK_MAX / 2)
        {
            RT_OBJECT_HOOK_CALL(rt_timer_timeout_hook, (t));

            /* move node to the next */
            n = n->next;

            /* remove timer from timer list firstly */
            _rt_timer_remove(t);

            /* not lock scheduler when performing timeout function */
            rt_exit_critical();
            /* call timeout function */
            t->timeout_func(t->parameter);

            /* re-get tick */
            current_tick = rt_tick_get();

            RT_DEBUG_LOG(RT_DEBUG_TIMER, ("current tick: %d\n", current_tick));

            /* lock scheduler */
            rt_enter_critical();

            if ((t->parent.flag & RT_TIMER_FLAG_PERIODIC) &&
                (t->parent.flag & RT_TIMER_FLAG_ACTIVATED))
            {
                /* start it */
                t->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;
                rt_timer_start(t);
            }
            else
            {
                /* stop timer */
                t->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;
            }
        }
        else break; /* not check anymore */
    }

    /* unlock scheduler */
    rt_exit_critical();

    RT_DEBUG_LOG(RT_DEBUG_TIMER, ("software timer check leave\n"));
}

/* system timer thread entry */
static void rt_thread_timer_entry(void *parameter)
{
    rt_tick_t next_timeout;

    while (1)
    {
        /* get the next timeout tick */
        next_timeout = rt_timer_list_next_timeout(rt_soft_timer_list);
        if (next_timeout == RT_TICK_MAX)
        {
            /* no software timer exist, suspend self. */
            rt_thread_suspend(rt_thread_self());
            rt_schedule();
        }
        else
        {
            rt_tick_t current_tick;

            /* get current tick */
            current_tick = rt_tick_get();

            if ((next_timeout - current_tick) < RT_TICK_MAX / 2)
            {
                /* get the delta timeout tick */
                next_timeout = next_timeout - current_tick;
                rt_thread_delay(next_timeout);
            }
        }

        /* check software timer */
        rt_soft_timer_check();
    }
}
#endif

/**
 * @ingroup SystemInit
 *
 * @brief ��ʼ��ϵͳϵͳ��ʱ��
 *
 *
 * �ú�������ʼ��ϵͳ��ʱ��
 */
void rt_system_timer_init(void)
{
    int i;

    for (i = 0; i < sizeof(rt_timer_list) / sizeof(rt_timer_list[0]); i++)
    {
        rt_list_init(rt_timer_list + i);
    }
}

/**
 * @ingroup SystemInit
 *
 * @brief ��ʼ��ϵͳ��ʱ���߳�
 *
 * ���ݳ�ʱ����ִ��ʱ�����������Ļ���������ʱ������ΪSOFT_TIMERģʽʱ��
 * �ú�������ʼ��ϵͳ�Ķ�ʱ���̡߳�
 *
 */
void rt_system_timer_thread_init(void)
{
#ifdef RT_USING_TIMER_SOFT
    int i;

    for (i = 0;
         i < sizeof(rt_soft_timer_list) / sizeof(rt_soft_timer_list[0]);
         i++)
    {
        rt_list_init(rt_soft_timer_list + i);
    }

    /* start software timer thread */
    rt_thread_init(&timer_thread,
                   "timer",
                   rt_thread_timer_entry,
                   RT_NULL,
                   &timer_thread_stack[0],
                   sizeof(timer_thread_stack),
                   RT_TIMER_THREAD_PRIO,
                   10);

    /* startup */
    rt_thread_startup(&timer_thread);
#endif
}

/**@}*/
