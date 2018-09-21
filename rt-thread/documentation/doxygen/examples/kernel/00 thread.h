/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup example
 */
/*@{*/

 /**
 * @defgroup thread 线程管理
 *
 * 创建/删除、初始化线程
 *
 * 这个例子会创建两个线程，一个动态线程，一个静态线程。
 * 一个线程在运行完毕后自动被系统删除，另一个线程一直打印计数。
 *
 * **源码**
 \code{.c}
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_TIMESLICE        5

/* 指向信号量的指针 */
static rt_sem_t dynamic_sem = RT_NULL;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
static void rt_thread1_entry(void *parameter)
{
    static rt_uint8_t count = 0;
  
    while(1)
    {
        if(count <= 100)
        {
            count++;           
        }
        else
            return; 
        
        /* count每计数10次，就释放一次信号量 */
         if(0 == (count % 10))
        {
            rt_kprintf("t1 release a dynamic semaphore.\n" ); 
            rt_sem_release(dynamic_sem);            
        }
    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
static void rt_thread2_entry(void *parameter)
{
    static rt_err_t result;
    static rt_uint8_t number = 0;
    while(1)
    {
        /* 永久方式等待信号量，获取到信号量，则执行number自加的操作 */
        result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);
        if (result != RT_EOK)
        {        
            rt_kprintf("t2 take a dynamic semaphore, failed.\n");
            rt_sem_delete(dynamic_sem);
            return;
        }
        else
        {      
            number++;             
            rt_kprintf("t2 take a dynamic semaphore. number = %d\n" ,number);                        
        }
    }   
}

/* 信号量示例的初始化 */
int semaphore_sample()
{
    /* 创建一个动态信号量，初始值是0 */
    dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);
    if (dynamic_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. dynamic semaphore value = 0.\n");
    }

    rt_thread_init(&thread1,
                   "thread1",
                   rt_thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack), 
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);
                   
    rt_thread_init(&thread2,
                   "thread2",
                   rt_thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack), 
                   THREAD_PRIORITY-1, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);

    return 0;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(semaphore_sample, semaphore sample);


  \endcode
 */

 /**
 * @defgroup slice 线程调度
 *
 * 相同优先级线程按照时间片轮番调度
 *
 * 这个例子中将创建两个线程，每一个线程都在打印信息
 * 
 *
 * **源码**
 \code{.c}
#include <rtthread.h>

#define THREAD_STACK_SIZE	1024
#define THREAD_PRIORITY	    20
#define THREAD_TIMESLICE    10

/* 线程入口 */
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
    /* 创建线程1 */
    tid = rt_thread_create("thread1", 
                            thread_entry, (void*)1, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE); 
    if (tid != RT_NULL) 
        rt_thread_startup(tid);


    /* 创建线程2 */
    tid = rt_thread_create("thread2", 
                            thread_entry, (void*)2,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE-5);
    if (tid != RT_NULL) 
        rt_thread_startup(tid);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(timeslice_sample, timeslice sample);


  \endcode
 */

 /**
 * @defgroup hook 线程调度器钩子
 *
 * 在线程调度器钩子中打印线程切换信息
 *
 * **源码**
 \code{.c}
#include <rtthread.h>

#define THREAD_STACK_SIZE	1024
#define THREAD_PRIORITY	    20
#define THREAD_TIMESLICE    10

/* 针对每个线程的计数器 */
volatile rt_uint32_t count[2];

/* 线程1、2共用一个入口，但入口参数不同 */
static void thread_entry(void* parameter)
{
    rt_uint32_t value;

    value = (rt_uint32_t)parameter;
    while (1)
    {
        rt_kprintf("thread %d is running\n", value);
        rt_thread_mdelay(1000); //延时一段时间
    }
}

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

static void hook_of_scheduler(struct rt_thread* from, struct rt_thread* to)
{
    rt_kprintf("from: %s -->  to: %s \n", from->name , to->name);
}

int scheduler_hook(void)
{   
    /* 设置调度器钩子 */
    rt_scheduler_sethook(hook_of_scheduler);
    
    /* 创建线程1 */
    tid1 = rt_thread_create("thread1", 
                            thread_entry, (void*)1, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE); 
    if (tid1 != RT_NULL) 
        rt_thread_startup(tid1);

    /* 创建线程2 */
    tid2 = rt_thread_create("thread2", 
                            thread_entry, (void*)2, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY,THREAD_TIMESLICE - 5);
    if (tid2 != RT_NULL) 
        rt_thread_startup(tid2);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(scheduler_hook, scheduler_hook sample);

  \endcode
 */


/*@}*/

