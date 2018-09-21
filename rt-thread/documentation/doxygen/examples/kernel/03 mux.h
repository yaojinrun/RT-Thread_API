/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup example
 */
/*@{*/


 /**
 * @defgroup mux ����������
 *
 * ��������һ�ֱ���������Դ�ķ�������һ���߳�ӵ�л�������ʱ��
 * ���Ա���������Դ���������߳��ƻ����߳�1��2��number�ֱ���м�1������
 * �߳�2Ҳ���2��number�ֱ���м�1������ʹ�û�������֤2��numberֵ����һ�¡�
 *
 * **Դ��**
  \code{.c}

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

  \endcode
 */


 /**
 * @defgroup pri_invers ���������ȼ���ת
 *
 * ������ӽ����� 3 ����̬�߳��Լ����л�����ʱ�����е��߳����ȼ��Ƿ�
 * ���������ȴ��߳����ȼ��е�������ȼ���
 *
 * �߳� 1��2��3 �����ȼ��Ӹߵ��ͷֱ𱻴�����
 * �߳� 3 �ȳ��л������������߳� 2 ��ͼ���л���������ʱ�߳� 3 �����ȼ�Ӧ��
 * ������Ϊ���߳� 2 �����ȼ���ͬ���߳� 1 ���ڼ���߳� 3 �����ȼ��Ƿ�����
 * Ϊ���߳� 2�����ȼ���ͬ��
 *
 * **Դ��**
  \code{.c}
#include <rtthread.h>

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_mutex_t mutex = RT_NULL;


#define THREAD_PRIORITY       10
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      5

/* �߳� 1 ��� */
static void thread1_entry(void *parameter)
{
    /* ���õ����ȼ��߳����� */
    rt_thread_mdelay(100);

    /* ��ʱ thread3 ���� mutex������ thread2 �ȴ����� mutex */

    /* ��� rt_kprintf("the producer generates a number: %d\n", array[set%MAXSEM]); �� thread3 �����ȼ���� */
    if (tid2->current_priority != tid3->current_priority)
    {
        /* ���ȼ�����ͬ������ʧ�� */
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

/* �߳� 2 ��� */
static void thread2_entry(void *parameter)
{
    rt_err_t result;

    rt_kprintf("the priority of thread2 is: %d\n", tid2->current_priority);

    /* ���õ����ȼ��߳����� */
    rt_thread_mdelay(50);


    /*
     * ��ͼ���л���������ʱ thread3 ���У�Ӧ�� thread3 �����ȼ�����
     * �� thread2 ��ͬ�����ȼ�
     */
    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);

    if (result == RT_EOK)
    {
        /* �ͷŻ����� */
        rt_mutex_release(mutex);
    }
}

/* �߳� 3 ��� */
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

    /* ��һ����ʱ���ѭ����500ms */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 2)) ;

    rt_mutex_release(mutex);
}

int pri_inversion(void)
{
    /* ���������� */
    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    /* �����߳� 1 */
    tid1 = rt_thread_create("thread1",
                            thread1_entry, 
                            RT_NULL,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
         rt_thread_startup(tid1);
 
    /* �����߳� 2 */
    tid2 = rt_thread_create("thread2",
                            thread2_entry, 
                            RT_NULL, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    /* �����߳� 3 */
    tid3 = rt_thread_create("thread3",
                            thread3_entry, 
                            RT_NULL, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(pri_inversion, pri_inversion sample);

  \endcode
 */

/*@}*/

