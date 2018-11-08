/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-15     misonyo      first implementation.
 */
/* 
 * �����嵥������һ�� PIN �豸ʹ������
 * ���̵����� pin_beep_sample ��������ն�
 * ������ø�ʽ��pin_beep_sample
 * �����ܣ�ͨ���������Ʒ�������Ӧ���ŵĵ�ƽ״̬���Ʒ�����
*/

#include <rtthread.h>
#include <rtdevice.h>

/* ���ű�ţ�ͨ���鿴�����ļ�drv_gpio.cȷ�� */
#ifndef BEEP_PIN_NUM
#define BEEP_PIN_NUM            35  /* PB0 */
#endif
#ifndef KEY0_PIN_NUM
#define KEY0_PIN_NUM            55  /* PD8 */
#endif
#ifndef KEY1_PIN_NUM
#define KEY1_PIN_NUM            56  /* PD9 */
#endif

void beep_on(void *args)
{
    rt_kprintf("turn on beep!\n");
    
    rt_pin_write(BEEP_PIN_NUM, PIN_HIGH);
}

void beep_off(void *args)
{
    rt_kprintf("turn off beep!\n");
    
    rt_pin_write(BEEP_PIN_NUM, PIN_LOW);
}

static void pin_beep_sample(void)
{
    /* ����������Ϊ���ģʽ */
    rt_pin_mode(BEEP_PIN_NUM, PIN_MODE_OUTPUT);
    /* Ĭ�ϵ͵�ƽ */
    rt_pin_write(BEEP_PIN_NUM, PIN_LOW);
    
    /* ����0����Ϊ����ģʽ */
    rt_pin_mode(KEY0_PIN_NUM, PIN_MODE_INPUT_PULLUP);
    /* ���жϣ�������ģʽ���ص�������Ϊbeep_on */
    rt_pin_attach_irq(KEY0_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_on, RT_NULL);
    /* ʹ���ж� */
    rt_pin_irq_enable(KEY0_PIN_NUM, PIN_IRQ_ENABLE); 
    
    /* ����1����Ϊ����ģʽ */
    rt_pin_mode(KEY1_PIN_NUM, PIN_MODE_INPUT_PULLUP);
    /* ���жϣ�������ģʽ���ص�������Ϊbeep_off */
    rt_pin_attach_irq(KEY1_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_off, RT_NULL);
    /* ʹ���ж� */
    rt_pin_irq_enable(KEY1_PIN_NUM, PIN_IRQ_ENABLE);
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(pin_beep_sample, pin beep sample);
