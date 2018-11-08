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
 * �����嵥������������
 *
 * ��������һ�ֱ���������Դ�ķ�������һ���߳�ӵ�л�������ʱ��
 * ���Ա���������Դ���������߳��ƻ����߳�1��2��number�ֱ���м�1����
 * �߳�2Ҳ���2��number�ֱ���м�1������ʹ�û�������֤2��numberֵ����һ��
 */
#include <rtthread.h>

#define THREAD_PRIORITY         8
#define THREAD_TIMESLICE        5

/* ָ�򻥳�����ָ�� */
static rt_mutex_t dynamic_mutex = RT_NULL;
static rt_uint8_t number1,number2 = 0;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
static void rt_thread_entry1(void *parameter)
{
      while(1)
      {
          /* �߳�1��ȡ�����������Ⱥ��number1��number2���м�1������Ȼ���ͷŻ����� */
          rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);          
          number1++;
          rt_thread_mdelay(10);
          number2++;          
          rt_mutex_release(dynamic_mutex);
       }	    
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
static void rt_thread_entry2(void *parameter)
{     
      while(1)
      {
          /* �߳�2��ȡ���������󣬼��number1��number2��ֵ�Ƿ���ͬ����ͬ���ʾmutex������������ */
          rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
          if(number1 != number2)
          {
            rt_kprintf("not protect.number1 = %d, mumber2 = %d \n",number1 ,number2);
          }
          else
          {
            rt_kprintf("mutex protect ,number1 = mumber2 is %d\n",number1);            
          }
          
           number1++;
           number2++;
           rt_mutex_release(dynamic_mutex);
          
          if(number1 >=50)
              return;      
      }	  
}

/* ������ʾ���ĳ�ʼ�� */
int mutex_sample(void)
{
    /* ����һ����̬������ */
    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
    if (dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    rt_thread_init(&thread1,
                   "thread1",
                   rt_thread_entry1,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack), 
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);
    
    rt_thread_init(&thread2,
                   "thread2",
                   rt_thread_entry2,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack), 
                   THREAD_PRIORITY-1, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(mutex_sample, mutex sample);
