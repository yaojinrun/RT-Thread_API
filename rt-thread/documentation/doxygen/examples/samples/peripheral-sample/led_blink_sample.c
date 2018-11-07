/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-25     misonyo      first edition.
 */
/* 
 * �����嵥������һ��ͨ��PIN�ſ���LED�����ʹ������
 * ���̵����� led_sample ��������ն�
 * ������ø�ʽ��led_sample 41
 * ������ͣ�����ڶ���������Ҫʹ�õ�PIN�ű�ţ�Ϊ����ʹ������Ĭ�ϵ����ű�š�
 * �����ܣ����򴴽�һ��led�̣߳��߳�ÿ��1000ms�ı�PIN��״̬���ﵽ����led��
 *            �����Ч����
*/

#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>

/* PIN�ű�ţ��鿴�����ļ�drv_gpio.cȷ�� */
#define LED_PIN_NUM    41
static int pin_num;

static void led_entry(void *parameter)
{
    int count = 0;
    /* ����PIN��ģʽΪ��� */
    rt_pin_mode(pin_num, PIN_MODE_OUTPUT);

    while (1)
    {
        count++;
        rt_kprintf("thread run count : %d\r\n", count);
        /* ����PIN�� */
        rt_pin_write(pin_num, PIN_LOW);
        rt_kprintf("led on!\r\n");
        /* ��ʱ1000ms */
        rt_thread_mdelay(1000);

        /* ����PIN�� */
        rt_pin_write(pin_num, PIN_HIGH);
        rt_kprintf("led off!\r\n");
        rt_thread_mdelay(1000);
    }
}

static void led_sample(int argc,char *argv[])
{
    rt_thread_t tid;

    /* �ж������в����Ƿ������PIN�ű�� */
    if (argc == 2)
    {
        pin_num = atoi(argv[1]);
    }
    else
    {
        pin_num = LED_PIN_NUM;
    }

    tid = rt_thread_create("led",
                           led_entry,
                           RT_NULL,
                           512,
                           RT_THREAD_PRIORITY_MAX/3,
                           20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(led_sample, led sample);
