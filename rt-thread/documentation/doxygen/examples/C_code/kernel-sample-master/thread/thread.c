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
 * 程序清单：创建/初始化线程
 */
#include <rtthread.h>

#define THREAD_STACK_SIZE       512
#define THREAD_PRIORITY         25
#define THREAD_TIMESLICE        5

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;

static void thread_entry(void *parameter)
{
	while(1)
	{
		// ...
	}
}
int thread_sample ()
{
    rt_thread_t tid;

    /* 初始化线程1，名称是thread1，入口是thread1_entry */
    rt_thread_init(&thread1,
                   "thread1",
                   thread_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack),
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);
				   
    /* 创建线程2 */
    tid = rt_thread_create("thread2", 
                            thread_entry, (void*)1, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE); 
	
    /* 如果获得线程控制块，启动这个线程 */
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_sample, thread sample);
