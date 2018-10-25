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
 * �����嵥����ͬ���ȼ��̰߳���ʱ��Ƭ�ַ�����
 *
 * ��������н����������̣߳�ÿһ���̶߳��ڴ�ӡ��Ϣ
 * 
 */

#include <rtthread.h>

#define THREAD_STACK_SIZE	1024
#define THREAD_PRIORITY	    20
#define THREAD_TIMESLICE    10

/* �߳���� */
static void thread_entry(void* parameter)
{
    rt_uint32_t value;
    rt_uint32_t count = 0;

    value = (rt_uint32_t)parameter;
    while (1)
    {
        if(0 == (count % 5))
        {           
            rt_kprintf("thread %d is running ,thread %d count = %d\n", value , value , count);      

            if(count > 200)
                return;            
        }
         count++;
     }  
}

int timeslice_sample(void)
{
    rt_thread_t tid;
    /* �����߳�1 */
    tid = rt_thread_create("thread1", 
                            thread_entry, (void*)1, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE); 
    if (tid != RT_NULL) 
        rt_thread_startup(tid);


    /* �����߳�2 */
    tid = rt_thread_create("thread2", 
                            thread_entry, (void*)2,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE-5);
    if (tid != RT_NULL) 
        rt_thread_startup(tid);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(timeslice_sample, timeslice sample);

