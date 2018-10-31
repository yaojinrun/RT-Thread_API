/*
 * File      : irq.c
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
 * 2006-02-24     Bernard      first version
 * 2006-05-03     Bernard      add IRQ_DEBUG
 * 2016-08-09     ArdaFu       add interrupt enter and leave hook.
 */

#include <rthw.h>
#include <rtthread.h>

#ifdef RT_USING_HOOK

static void (*rt_interrupt_enter_hook)(void);
static void (*rt_interrupt_leave_hook)(void);

/**
 * @ingroup Hook
 * @brief 设置进入中断钩子函数
 *
 * 该函数将设置一个中断钩子函数，当系统进入中断时会调用该函数。
 *
 * @note 钩子函数必须简单，永远不会被阻塞或挂起。
 */
void rt_interrupt_enter_sethook(void (*hook)(void))
{
    rt_interrupt_enter_hook = hook;
}
/**
 * @ingroup Hook
 * @brief 设置中断退出钩子函数
 *
 * 该函数将设置一个钩子函数，当系统退出中断时会调用该函数。
 *
 * @note 钩子函数必须简单，永远不会被阻塞或挂起。
 */
void rt_interrupt_leave_sethook(void (*hook)(void))
{
    rt_interrupt_leave_hook = hook;
}
#endif

/* #define IRQ_DEBUG */

/**
 * @addtogroup bsp
 */

/**@{*/

volatile rt_uint8_t rt_interrupt_nest;

/**
 * @brief 中断进入
 *
 * 当进入中断服务程序时，BSP将调用此函数
 *
 * @note 请不要在程序中调用此例程。
 *
 * @see rt_interrupt_leave
 */
void rt_interrupt_enter(void)
{
    rt_base_t level;

    RT_DEBUG_LOG(RT_DEBUG_IRQ, ("irq coming..., irq nest:%d\n",
                                rt_interrupt_nest));

    level = rt_hw_interrupt_disable();
    rt_interrupt_nest ++;
    RT_OBJECT_HOOK_CALL(rt_interrupt_enter_hook,());
    rt_hw_interrupt_enable(level);
}
RTM_EXPORT(rt_interrupt_enter);

/**
 * @brief 中断离开
 *
 * 当离开中断服务程序时，BSP将调用此函数
 *
 * @note 请不要在程序中调用此例程。
 *
 * @see rt_interrupt_enter
 */
void rt_interrupt_leave(void)
{
    rt_base_t level;

    RT_DEBUG_LOG(RT_DEBUG_IRQ, ("irq leave, irq nest:%d\n",
                                rt_interrupt_nest));

    level = rt_hw_interrupt_disable();
    rt_interrupt_nest --;
    RT_OBJECT_HOOK_CALL(rt_interrupt_leave_hook,());
    rt_hw_interrupt_enable(level);
}
RTM_EXPORT(rt_interrupt_leave);

/**
 * @brief 中断嵌套层数获取
 *
 * 该函数将返回中断嵌套。
 *
 * 用户应用程序可以调用此函数来获取当前上下文是否是中断上下文。
 *
 * @return 嵌套中断的数量。
 */
rt_uint8_t rt_interrupt_get_nest(void)
{
    return rt_interrupt_nest;
}
RTM_EXPORT(rt_interrupt_get_nest);

RTM_EXPORT(rt_hw_interrupt_disable);
RTM_EXPORT(rt_hw_interrupt_enable);

/**@}*/

