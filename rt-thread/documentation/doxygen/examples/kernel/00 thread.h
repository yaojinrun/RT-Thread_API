/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup example
 */
/*@{*/

 /**
 * @defgroup thread �̹߳���
 *
 * ����/ɾ������ʼ���߳�
 *
 * ������ӻᴴ�������̣߳�һ����̬�̣߳�һ����̬�̡߳�
 * һ���߳���������Ϻ��Զ���ϵͳɾ������һ���߳�һֱ��ӡ������
 *
 * **Դ��**
 \code{.c}
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_TIMESLICE        5

/* ָ���ź�����ָ�� */
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
        
        /* countÿ����10�Σ����ͷ�һ���ź��� */
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
        /* ���÷�ʽ�ȴ��ź�������ȡ���ź�������ִ��number�ԼӵĲ��� */
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

/* �ź���ʾ���ĳ�ʼ�� */
int semaphore_sample()
{
    /* ����һ����̬�ź�������ʼֵ��0 */
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
/* ������ msh �����б��� */
MSH_CMD_EXPORT(semaphore_sample, semaphore sample);


  \endcode
 */

 /**
 * @defgroup slice �̵߳���
 *
 * ��ͬ���ȼ��̰߳���ʱ��Ƭ�ַ�����
 *
 * ��������н����������̣߳�ÿһ���̶߳��ڴ�ӡ��Ϣ
 * 
 *
 * **Դ��**
 \code{.c}
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


  \endcode
 */

 /**
 * @defgroup hook �̵߳���������
 *
 * ���̵߳����������д�ӡ�߳��л���Ϣ
 *
 * **Դ��**
 \code{.c}
#include <rtthread.h>

#define THREAD_STACK_SIZE	1024
#define THREAD_PRIORITY	    20
#define THREAD_TIMESLICE    10

/* ���ÿ���̵߳ļ����� */
volatile rt_uint32_t count[2];

/* �߳�1��2����һ����ڣ�����ڲ�����ͬ */
static void thread_entry(void* parameter)
{
    rt_uint32_t value;

    value = (rt_uint32_t)parameter;
    while (1)
    {
        rt_kprintf("thread %d is running\n", value);
        rt_thread_mdelay(1000); //��ʱһ��ʱ��
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
    /* ���õ��������� */
    rt_scheduler_sethook(hook_of_scheduler);
    
    /* �����߳�1 */
    tid1 = rt_thread_create("thread1", 
                            thread_entry, (void*)1, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE); 
    if (tid1 != RT_NULL) 
        rt_thread_startup(tid1);

    /* �����߳�2 */
    tid2 = rt_thread_create("thread2", 
                            thread_entry, (void*)2, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY,THREAD_TIMESLICE - 5);
    if (tid2 != RT_NULL) 
        rt_thread_startup(tid2);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(scheduler_hook, scheduler_hook sample);

  \endcode
 */


/*@}*/

