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
 * �����嵥����̬�ڴ��������
 *
 * �������ᴴ��һ����̬���̣߳�����̻߳ᶯ̬�����ڴ沢�ͷ�
 * ÿ�����������ڴ棬�����벻����ʱ��ͽ���
 */
#include <rtthread.h>

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* �߳���� */
void thread1_entry(void *parameter)
{
    int i;
    char *ptr = RT_NULL; /* �ڴ���ָ�� */

    for (i = 0; ; i++)
    {
        /* ÿ�η��� (1 << i) ��С�ֽ������ڴ�ռ� */
        ptr = rt_malloc(1 << i);

        /* �������ɹ� */
        if (ptr != RT_NULL)
        {
            rt_kprintf("get memory :%d byte\n", (1 << i));
            /* �ͷ��ڴ�� */
            rt_free(ptr);
            rt_kprintf("free memory :%d byte\n", (1 << i));
            ptr = RT_NULL;
        }
        else
        {
            rt_kprintf("try to get %d byte memory failed!\n", (1 << i));
            return;
        }
    }
}

int dynmem_sample(void)
{
    rt_thread_t tid = RT_NULL;

    /* �����߳�1 */
    tid = rt_thread_create("thread1",
                           thread1_entry, RT_NULL,
                           THREAD_STACK_SIZE,
                           THREAD_PRIORITY,
                           THREAD_TIMESLICE);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(dynmem_sample, dynmem sample);
