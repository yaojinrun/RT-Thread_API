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
 * 该函数将初始化系统节拍时钟并将其值设置为0。
 * @ingroup SystemInit
 *
 * @deprecated 从1.1.0开始，不需要在系统初始化时调用此函数。
 */
void rt_system_tick_init(void)
{
}

/**
 * @addtogroup Clock
 */

/**@{*/

/**
 * @brief 获取系统时钟节拍值
 *
 * 该函数将返回自操作系统启动以来到当前的系统时钟计数值
 *
 * @return 当前系统节拍计数值
 */
rt_tick_t rt_tick_get(void)
{
    /* return the global tick */
    return rt_tick;
}
RTM_EXPORT(rt_tick_get);

/**
 * @brief 设置系统时钟节拍值
 *
 * 该函数将设定当前的系统时钟节拍计数值。
 */
void rt_tick_set(rt_tick_t tick)
{
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    rt_tick = tick;
    rt_hw_interrupt_enable(level);
}

/**
 * @brief 增加系统时钟节拍值
 *
 * 该函数将通知内核经过了一个系统时钟的时间，通常该函数在时钟中断里调用。
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
 * @brief 节拍到毫秒的转换
 *
 * 该函数把毫秒转换为系统时钟节拍计数值。
 *
 * @param ms 指定的时间（毫秒）
 *           - 为负数时将永远等待
 *           - 为0时不等待立即返回
 *           - 最大值为 0x7fffffff
 *
 * @return 计算后的系统时钟节拍数
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

