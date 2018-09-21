/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup example
 */
/*@{*/


 /**
 * @defgroup mux 互斥锁例程
 *
 * 互斥锁是一种保护共享资源的方法。当一个线程拥有互斥锁的时候，
 * 可以保护共享资源不被其他线程破坏。线程1对2个number分别进行加1操作，
 * 线程2也会对2个number分别进行加1操作。使用互斥量保证2个number值保持一致。
 *
 * **源码**
  \code{.c}

#include <rtthread.h>

#define THREAD_PRIORITY         8
#define THREAD_TIMESLICE        5

/* 指向互斥量的指针 */
static rt_mutex_t dynamic_mutex = RT_NULL;
static rt_uint8_t number1,number2 = 0;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
static void rt_thread_entry1(void *parameter)
{
      while(1)
      {
          /* 线程1获取到互斥量后，先后对number1、number2进行加1操作，然后释放互斥量 */
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
          /* 线程2获取到互斥量后，检查number1、number2的值是否相同，相同则表示mutex起到了锁的作用 */
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

/* 互斥量示例的初始化 */
int mutex_sample(void)
{
    /* 创建一个动态互斥量 */
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

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(mutex_sample, mutex sample);

  \endcode
 */


 /**
 * @defgroup pri_invers 互斥量优先级反转
 *
 * 这个例子将创建 3 个动态线程以检查持有互斥量时，持有的线程优先级是否
 * 被调整到等待线程优先级中的最高优先级。
 *
 * 线程 1，2，3 的优先级从高到低分别被创建，
 * 线程 3 先持有互斥量，而后线程 2 试图持有互斥量，此时线程 3 的优先级应该
 * 被提升为和线程 2 的优先级相同。线程 1 用于检查线程 3 的优先级是否被提升
 * 为与线程 2的优先级相同。
 *
 * **源码**
  \code{.c}
#include <rtthread.h>

/* 指向线程控制块的指针 */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_mutex_t mutex = RT_NULL;


#define THREAD_PRIORITY       10
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      5

/* 线程 1 入口 */
static void thread1_entry(void *parameter)
{
    /* 先让低优先级线程运行 */
    rt_thread_mdelay(100);

    /* 此时 thread3 持有 mutex，并且 thread2 等待持有 mutex */

    /* 检查 rt_kprintf("the producer generates a number: %d\n", array[set%MAXSEM]); 与 thread3 的优先级情况 */
    if (tid2->current_priority != tid3->current_priority)
    {
        /* 优先级不相同，测试失败 */
        rt_kprintf("the priority of thread2 is: %d\n", tid2->current_priority);
        rt_kprintf("the priority of thread3 is: %d\n", tid3->current_priority);
        rt_kprintf("test failed.\n");
        return;
    }
    else
    {
        rt_kprintf("the priority of thread2 is: %d\n", tid2->current_priority);
        rt_kprintf("the priority of thread3 is: %d\n", tid3->current_priority);
        rt_kprintf("test OK.\n");
    }
}

/* 线程 2 入口 */
static void thread2_entry(void *parameter)
{
    rt_err_t result;

    rt_kprintf("the priority of thread2 is: %d\n", tid2->current_priority);

    /* 先让低优先级线程运行 */
    rt_thread_mdelay(50);


    /*
     * 试图持有互斥锁，此时 thread3 持有，应把 thread3 的优先级提升
     * 到 thread2 相同的优先级
     */
    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);

    if (result == RT_EOK)
    {
        /* 释放互斥锁 */
        rt_mutex_release(mutex);
    }
}

/* 线程 3 入口 */
static void thread3_entry(void *parameter)
{
    rt_tick_t tick;
    rt_err_t result;

    rt_kprintf("the priority of thread3 is: %d\n", tid3->current_priority);

    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        rt_kprintf("thread3 take a mutex, failed.\n");
    }

    /* 做一个长时间的循环，500ms */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 2)) ;

    rt_mutex_release(mutex);
}

int pri_inversion(void)
{
    /* 创建互斥锁 */
    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    /* 创建线程 1 */
    tid1 = rt_thread_create("thread1",
                            thread1_entry, 
                            RT_NULL,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
         rt_thread_startup(tid1);
 
    /* 创建线程 2 */
    tid2 = rt_thread_create("thread2",
                            thread2_entry, 
                            RT_NULL, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    /* 创建线程 3 */
    tid3 = rt_thread_create("thread3",
                            thread3_entry, 
                            RT_NULL, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(pri_inversion, pri_inversion sample);

  \endcode
 */

/*@}*/

