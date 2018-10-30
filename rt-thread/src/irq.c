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
 * @brief �����жϹ��Ӻ�������
 *
 * �ú���������һ���жϹ��Ӻ�������ϵͳ�����ж�ʱ���øú�����
 *
 * @note ���Ӻ�������򵥣���Զ���ᱻ���������
 */
void rt_interrupt_enter_sethook(void (*hook)(void))
{
    rt_interrupt_enter_hook = hook;
}
/**
 * @ingroup Hook
 * @brief �ж��˳����Ӻ�������
 *
 * �ú���������һ�����Ӻ�������ϵͳ�˳��ж�ʱִ�иú�����
 *
 * @note ���Ӻ�������򵥣���Զ���ᱻ���������
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
 * @brief �����ж�
 *
 * �������жϷ������ʱ����Ҫ���ô˺���֪ͨ�ں˵�ǰ�Ѿ����뵽�ж�״̬��
 * ���жϷ�������У�����������ں���صĺ��������ͷ��ź����Ȳ�������
 * �����ͨ���жϵ�ǰ�ж�״̬�����ں˼�ʱ������Ӧ����Ϊ��
 * ���磺���ж����ͷ���һ���ź�����������ĳ�̣߳���ͨ���жϷ��ֵ�ǰϵͳ�����ж������Ļ����У�
 * ��ô�ڽ����߳��л�ʱӦ�ò�ȡ�ж����߳��л��Ĳ��ԣ����������������л���
 *
 * ����жϷ�����򲻻�����ں���صĺ������ͷ��ź����Ȳ�����������Բ��õ��ô˺�����
 *
 * @note �벻Ҫ�ڳ����е��ô����̡�
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
 * @brief �뿪�ж�
 *
 * ���뿪�жϷ������ʱ����Ҫ���ô˺�������֪ͨ�ں˵�ǰ�Ѿ��뿪���ж�״̬��
 *
 * @note �벻Ҫ�ڳ����е��ô����̡�
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
 * @brief ��ȡ�ж�Ƕ�ײ���
 *
 * �ú����������ж�Ƕ�ײ�����
 *
 * �û�Ӧ�ó�����Ե��ô˺�������ȡ����ǰ�Ѿ����뵽�ж�״̬��ǰǶ�׵��ж���ȡ�
 *
 * @return �ж�Ƕ�ײ�����
 */
rt_uint8_t rt_interrupt_get_nest(void)
{
    return rt_interrupt_nest;
}
RTM_EXPORT(rt_interrupt_get_nest);

RTM_EXPORT(rt_hw_interrupt_disable);
RTM_EXPORT(rt_hw_interrupt_enable);

/**@}*/

