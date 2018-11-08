/*
 * File      : clock.c
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
 * 2006-05-27     Bernard      add support for same priority thread schedule
 * 2006-08-10     Bernard      remove the last rt_schedule in rt_tick_increase
 * 2010-03-08     Bernard      remove rt_passed_second
 * 2010-05-20     Bernard      fix the tick exceeds the maximum limits
 * 2010-07-13     Bernard      fix rt_tick_from_millisecond issue found by kuronca
 * 2011-06-26     Bernard      add rt_tick_set function.
 */

#include <rthw.h>
#include <rtthread.h>

static rt_tick_t rt_tick = 0;

extern void rt_timer_check(void);

/*
 * �ú�������ʼ��ϵͳ����ʱ�Ӳ�����ֵ����Ϊ0��
 * @ingroup SystemInit
 *
 * @deprecated ��1.1.0��ʼ������Ҫ��ϵͳ��ʼ��ʱ���ô˺�����
 */
void rt_system_tick_init(void)
{
}

/**
 * @addtogroup Clock
 */

/**@{*/

/**
 * @brief ��ȡϵͳʱ�ӽ���ֵ
 *
 * �ú����������Բ���ϵͳ������������ǰ��ϵͳʱ�Ӽ���ֵ
 *
 * @return ��ǰϵͳ���ļ���ֵ
 */
rt_tick_t rt_tick_get(void)
{
    /* return the global tick */
    return rt_tick;
}
RTM_EXPORT(rt_tick_get);

/**
 * @brief ����ϵͳʱ�ӽ���ֵ
 *
 * �ú������趨��ǰ��ϵͳʱ�ӽ��ļ���ֵ��
 */
void rt_tick_set(rt_tick_t tick)
{
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    rt_tick = tick;
    rt_hw_interrupt_enable(level);
}

/**
 * @brief ����ϵͳʱ�ӽ���ֵ
 *
 * �ú�����֪ͨ�ں˾�����һ��ϵͳʱ�ӵ�ʱ�䣬ͨ���ú�����ʱ���ж�����á�
 */
void rt_tick_increase(void)
{
    struct rt_thread *thread;

    /* increase the global tick */
    ++ rt_tick;

    /* check time slice */
    thread = rt_thread_self();

    -- thread->remaining_tick;
    if (thread->remaining_tick == 0)
    {
        /* change to initialized tick */
        thread->remaining_tick = thread->init_tick;

        /* yield */
        rt_thread_yield();
    }

    /* check timer */
    rt_timer_check();
}

/**
 * @brief ���ĵ������ת��
 *
 * �ú����Ѻ���ת��Ϊϵͳʱ�ӽ��ļ���ֵ��
 *
 * @param ms ָ����ʱ�䣨���룩
 *           - Ϊ����ʱ����Զ�ȴ�
 *           - Ϊ0ʱ���ȴ���������
 *           - ���ֵΪ 0x7fffffff
 *
 * @return ������ϵͳʱ�ӽ�����
 */
int rt_tick_from_millisecond(rt_int32_t ms)
{
    int tick;

    if (ms < 0)
        tick = RT_WAITING_FOREVER;
    else
        tick = (RT_TICK_PER_SECOND * ms + 999) / 1000;

    /* return the calculated tick */
    return tick;
}
RTM_EXPORT(rt_tick_from_millisecond);

/**@}*/

