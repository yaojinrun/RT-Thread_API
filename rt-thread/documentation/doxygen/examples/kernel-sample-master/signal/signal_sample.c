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
 * �����嵥���ź�����
 *
 * ������ӻᴴ��һ���̣߳��̰߳�װ�źţ�Ȼ�������̷߳����źš�
 *
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

/* �߳�1���źŴ����� */
void thread1_signal_handler(int sig)
{
    rt_kprintf("thread1 received signal %d\n", sig);
}

/* �߳�1����ں��� */
static void thread1_entry(void *parameter)
{
    int cnt = 0;

    /* ��װ�ź� */
    rt_signal_install(SIGUSR1, thread1_signal_handler);
    rt_signal_unmask(SIGUSR1);

    /* ����10�� */
    while (cnt < 10)
    {
        /* �߳�1���õ����ȼ����У�һֱ��ӡ����ֵ */
        rt_kprintf("thread1 count : %d\n", cnt);

        cnt++;
        rt_thread_mdelay(100);
    }
}

/* �ź�ʾ���ĳ�ʼ�� */
int signal_sample(void)
{
    /* �����߳�1 */
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    
    if (tid1 != RT_NULL)       
        rt_thread_startup(tid1);

    rt_thread_mdelay(300);

    /* �����ź� SIGUSR1 ���߳�1 */
    rt_thread_kill(tid1, SIGUSR1);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(signal_sample, signal sample);
