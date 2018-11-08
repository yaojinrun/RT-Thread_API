/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date           Author       Notes 
 * 2018-08-24     yangjie      the first version 
 */  

/*
 * �����嵥����������������
 *
 * ������̴���һ���̣߳�ͨ����ʱ������������ӣ����ڴ�ӡ������й��ӵĴ���
 */

#include <rtthread.h>
#include <rthw.h>

#define THREAD_PRIORITY      20
#define THREAD_STACK_SIZE    1024
#define THREAD_TIMESLICE     5

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid = RT_NULL;

/* ���к������Ӻ���ִ�д��� */
volatile static int hook_times = 0;

/* ���������Ӻ��� */
static void idle_hook()
{
    if (0 == (hook_times % 10000))
    {
        rt_kprintf("enter idle hook %d times.\n", hook_times);
    }

    rt_enter_critical();
    hook_times++;
    rt_exit_critical();
}

/* �߳���� */
static void thread_entry(void *parameter)
{
    int i = 5;
    while (i--)
    {
        rt_kprintf("enter thread1.\n");
        rt_enter_critical();
        hook_times = 0;
        rt_exit_critical();

        /* ����500ms */
        rt_kprintf("thread1 delay 50 OS Tick.\n", hook_times);
        rt_thread_mdelay(500);
    }
    rt_kprintf("delete idle hook.\n");
    
    /* ɾ�����й��Ӻ��� */
    rt_thread_idle_delhook(idle_hook);
    rt_kprintf("thread1 finish.\n");
}

int idle_hook_sample(void)
{
    /* ���ÿ����̹߳��� */
    rt_thread_idle_sethook(idle_hook);

    /* �����߳� */
    tid = rt_thread_create("thread1",
                           thread_entry, RT_NULL, 
                           THREAD_STACK_SIZE, 
                           THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(idle_hook_sample, idle hook sample);
