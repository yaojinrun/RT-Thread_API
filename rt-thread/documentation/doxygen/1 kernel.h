/*
 * This file is only used for doxygen document generation.
 */
 
/**
 * @defgroup Kernel �ں�
 *
 * @brief RT-Thread �ں�API��RT-Thread�ĺ���API
 *
 * ��֧�����¹��ܣ�
 * - ���̹߳���
 * - ͬ������
 * - �̼߳�ͨ��
 * - �ڴ����
 * - �첽��ʱ��
 */

/**
 * @addtogroup Kernel
 */
/*@{*/

/**
 * @defgroup BasicDef ��������
 *
 * @brief RT-Thread RTOS�еĻ�����������
 *
 * ��Щ��RT-Thread RTOS��ʹ�õĻ������塣 ͨ����RT-Thread�ں�ʹ���Լ��Ļ����������Ͷ��壬
 * ����rt_uint32_t��rt_uint8_t�ȣ����ǲ������ڱ���������ϵ�ṹ��
 */
 
/**
 * @defgroup SystemInit ϵͳ��ʼ��
 *
 * @brief RT-Thread ����������
 *
 * RT-Thread ���������룬���¿��Է�Ϊ�ĸ����֣�
 *
 * ��1����ʼ����ϵͳ��ص�Ӳ����
 *
 * ��2����ʼ��ϵͳ�ں˶������綨ʱ�������������źţ�
 *
 * ��3������main�̣߳���main�߳��жԸ���ģ�����ν��г�ʼ����
 *
 * ��4����ʼ����ʱ���̡߳������̣߳���������������

 * RT-Thread��������������ͼ��ʾ��
 *
 * @image html rtthread_startup.png "ϵͳ��������"
 * 
 */
 
/**
 * @defgroup component �����ʼ��
 *
 * @brief RT-Thread ���Զ���ʼ������
 *
 * һ����˵����ϵͳ������µĹ���ģ���ʱ����ʹ��ǰ�������ȳ�ʼ����
 * ͨ����������������������ǰ�ֶ���ӵ��ó�ʼ��������
 * �� RT-Thread �ṩ����һ�ֵ���ϸ��ھ۵ĳ�ʼ����ʽ��������Ҫ������
 * �ֶ���ӵ��ó�ʼ��������������ϵͳ����ǰ�Զ���ɸ�ģ��ĳ�ʼ����
 * 
 * �����ʼ����ʽҪ���ʼ����������ͨ��һЩ��ӿڽ�����������������
 * �Զ��ռ����б������ĳ�ʼ���������ŵ��ض������ݶ��У����ݶ��е�
 * ���к�����ϵͳ��ʼ��ʱ�ᱻ�Զ����á�
 *
 * �����ʼ�������к�ӿڼ��䱻��ʼ����˳�������ʾ��

 * ��ʼ��˳�� | �ӿ� 
 * ---|---
 *  1  | INIT_BOARD_EXPORT(fn); 
 *  2 | INIT_PREV_EXPORT(fn); 
 *  3  | INIT_DEVICE_EXPORT(fn);
 *  4  | INIT_COMPONENT_EXPORT(fn);
 *  5  | INIT_ENV_EXPORT(fn);
 *  6  | INIT_APP_EXPORT(fn);
 */
 
 
/**
 * @defgroup KernelObject �ں˶������
 * @brief �ں˶������ӿ�
 *
 * �ں˶���ϵͳ���Է��ʺ͹��������ں˶���
 *
 * �ں˶�������ں��еĴ�����豸��
 * - �߳�
 * - �ź����ͻ�����
 * - �¼�/�����¼������䣬��Ϣ����
 * - �ڴ��
 * - ��ʱ��
 * @image html Kernel_Object.png "Figure 2: Kernel Object"
 * @image rtf  Kernel_Object.png "Figure 2: Kernel Object"
 *
 * �ں˶�������Ǿ�̬�������ڴ��ڱ���ʱ���ѷ��䡣
 * Ҳ�����Ƕ�̬�������ڴ���������ʱ��ϵͳ���з��䡣
 */

/**
 * @defgroup Thread �̹߳���
 * @brief �̹߳���ӿ�
 *
 * RT-Thread����ϵͳ�ǻ����̵߳��ȵĶ�����ϵͳ��
 * - ���ȹ�����һ����ȫ��ռʽ�Ļ������ȼ��ĵ����㷨��
 * - ֧��8/32/256���ȼ�������0��ʾ��ߣ�7/31/255��ʾ��͡�������ȼ�7/31/255���ȼ����ڿ����̡߳�
 * - ֧������ͬ���ȼ����е��̡߳� ����ʱ��Ƭѭ�������������������
 * - ������ѡ����һ���Ѿ�������������ȼ����߳����С�
 * - �̹߳���������״̬
 * - ����ʼ״̬����δ��ʼ���е�״̬��
 * - ��������/����
 * - ������
 * - ���ѽ���
 * - ϵͳ�е��߳����������ƣ�����RAM��С�йء�
 *
 * **������**
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
 *
 */

/**
 * @defgroup Clock ʱ�ӹ���
 * @brief ʱ�ӹ���
 *
 * RT-Threadʹ��ʱ�ӽ�����ʵ�ֹ���ʱ��Ƭ���ȡ�
 */


/**
 * @defgroup Timer ��ʱ������
 * @brief ��ʱ������ӿ�
 *
 * �̵߳Ķ�ʱ�����ɶ�ʱ�������� ��ʱ����������Ϊ���λ��ڳ�ʱ��
 *
 * **������**
  \code{.c}
#include <rtthread.h>

/* ��ʱ���Ŀ��ƿ� */
static rt_timer_t timer1;
static rt_timer_t timer2;
static int cnt = 0;

/* ��ʱ��1��ʱ���� */
static void timeout1(void *parameter)
{
    rt_kprintf("periodic timer is timeout %d\n", cnt);

    /* ���е�10�Σ�ֹͣ���ڶ�ʱ�� */
    if (cnt++ >= 9)
    {
        rt_timer_stop(timer1);
        rt_kprintf("periodic timer was stopped! \n");
    }
}

/* ��ʱ��2��ʱ���� */
static void timeout2(void *parameter)
{
    rt_kprintf("one shot timer is timeout\n");
}

int timer_sample(void)
{
    /* ������ʱ��1  ���ڶ�ʱ�� */
    timer1 = rt_timer_create("timer1", timeout1,
                             RT_NULL, 10,
                             RT_TIMER_FLAG_PERIODIC);

    /* ������ʱ��1 */
    if (timer1 != RT_NULL) rt_timer_start(timer1);

    /* ������ʱ��2 ���ζ�ʱ�� */
    timer2 = rt_timer_create("timer2", timeout2,
                             RT_NULL,  30,
                             RT_TIMER_FLAG_ONE_SHOT);

    /* ������ʱ��2 */
    if (timer2 != RT_NULL) rt_timer_start(timer2);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(timer_sample, timer sample);

  \endcode
 */
 
/**
 * @defgroup IPC �̼߳�ͨѶ
 * @brief �̼߳�ͨѶ��ʩ
 *
 * RT-Thread����ϵͳ֧�ִ�ͳ���ź����ͻ�������
 * - �������ʹ�ü̳е����ȼ�����ֹ���ȼ���ת��
 * - �ź����ͷŲ������жϷ�������ǰ�ȫ�ġ�
 *
 * ���⣬�̻߳�ȡ�ź����򻥳������������п��԰����ȼ���FIFO������������־����ʾ���ֻ��ơ�
 * - RT_IPC_FLAG_FIFO
 * ����Դ����ʱ�������ڴ���Դ�Ϲ�����߳̽������Դ��
 * - RT_IPC_FLAG_PRIO
 * ����Դ����ʱ������������ȼ����߳̽������Դ��
 *
 * RT-Thread����ϵͳ֧���¼�/�����¼����������Ϣ���С�
 * - �¼�����ͨ������һ��������Ӧ��־λ����thead�¼�����ʱ�Ķ�������λ��
 * - �����¼�֧���¼��̶߳��С�һ������һλ�¼������Լ�ʱ���ҵ���Ӧ�������̣߳�Ȼ���ѡ�
 * - �������У��ʼ����ȹ̶�Ϊ4�ֽڣ������Ϣ���и���Ч��
 * - ͨ���豸�ķ��Ͷ��������жϷ������Ҳ�ǰ�ȫ�ġ�
 */
 
/**
 * @defgroup Pool �ڴ�ع���
 * @brief ��̬�ڴ�صĽӿ�
 *
 * �ڴ�أ�Memory Pool����һ���ڴ���䷽ʽ�����ڷ��������С��ͬ��С�ڴ�顣
 * �����Լ���ؼӿ��ڴ�������ͷŵ��ٶȣ����ܾ��������ڴ���Ƭ�������ڴ��Ϊ��ʱ��
 * ������ֹ������̣߳������������أ����ߵȴ�һ��ʱ�䷵�أ�����timeout����ȷ������
 * �������߳�����ڴ���ͷ��ڴ��ʱ�����������߳̽������ѡ�
 *
 *
 * **������**
  \code{.c}
#include <rtthread.h>

static rt_uint8_t *ptr[50];
static rt_uint8_t mempool[4096];
static struct rt_mempool mp;

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

/* �߳�1��� */
static void thread1_mp_alloc(void *parameter)
{
    int i;
    for (i = 0 ; i < 50 ; i++)
    {
        if (ptr[i] == RT_NULL)
        {
            /* ��ͼ�����ڴ��50�Σ������벻���ڴ��ʱ��
               �߳�1����ת���߳�2���� */
            ptr[i] = rt_mp_alloc(&mp, RT_WAITING_FOREVER);
            if (ptr[i] != RT_NULL)
                rt_kprintf("allocate No.%d\n", i);
        }
    }
}

/* �߳�2��ڣ��߳�2�����ȼ����߳�1�ͣ�Ӧ���߳�1�Ȼ��ִ�С�*/
static void thread2_mp_release(void *parameter)
{
    int i;

    rt_kprintf("thread2 try to release block\n");
    for (i = 0; i < 50 ; i++)
    {
        /* �ͷ����з���ɹ����ڴ�� */
        if (ptr[i] != RT_NULL)
        {
            rt_kprintf("release block %d\n", i);
            rt_mp_free(ptr[i]);
            ptr[i] = RT_NULL;
        }
    }
}

int mempool_sample(void)
{
    int i;
    for (i = 0; i < 50; i ++) ptr[i] = RT_NULL;

    /* ��ʼ���ڴ�ض��� */
    rt_mp_init(&mp, "mp1", &mempool[0], sizeof(mempool), 80);

    /* �����߳�1�������ڴ�� */
    tid1 = rt_thread_create("thread1", thread1_mp_alloc, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);


    /* �����߳�2���ͷ��ڴ��*/
    tid2 = rt_thread_create("thread2", thread2_mp_release, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(mempool_sample, mempool sample);

  \endcode
 */

/**
 * @defgroup MM ��̬�ڴ�Ĺ���
 * @brief ��̬�ڴ�ѵ�API�ӿ�
 *
 * ��̬�ڴ������һ����ʵ�Ķ��ڴ����ģ�飬�����ڵ�ǰ��Դ���������£������û�
 * ��������������С���ڴ�顣�����û�����Ҫ��ʹ����Щ�ڴ��ʱ���ֿ����ͷŻض���
 * ������Ӧ�÷���ʹ�á�RT-Thread ϵͳΪ�����㲻ͬ�������ṩ�����ײ�ͬ�Ķ�̬�ڴ�
 * �����㷨���ֱ���С���ڴ�����㷨�� SLAB �ڴ�����㷨��
 *
 * С���ڴ����ģ����Ҫ���ϵͳ��Դ�Ƚ��٣�һ������С��2MB�ڴ�ռ��ϵͳ����SLAB
 * �ڴ����ģ������Ҫ����ϵͳ��Դ�ȽϷḻʱ���ṩ��һ�ֽ��ƶ��ڴ�ع����㷨�Ŀ���
 * �㷨�������ڴ����ģ����ϵͳ����ʱֻ��ѡ������֮һ������ȫ��ʹ�ö�̬���ڴ����
 * ���������ֹ���ģ���ṩ��API�ӿ���ȫ��ͬ��
 *
 * ������֮�⣬RT-Thread����һ����Զ��ڴ�ѵĹ�����ƣ���memheap�ڴ����memheap����
 * ������ϵͳ���ڶ���ڴ�ѵ�����������Խ�����ڴ桰ճ������һ���γ�һ������ڴ�ѣ�
 * �û�ʹ��������е������ݡ�
 *
 * **������**
  \code{.c}
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
    rt_thread_t tid;

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

  \endcode
 */
 

/**
 * @defgroup bsp �жϹ���
 *
 * @brief �жϹ���ӿ�
 *
 * RT-Thread���жϹ�������Ҫ�ǹ����ж��豸���жϷ������̡��ж�Ƕ�ס��ж�ջ��ά����
 * �߳��л�ʱ���ֳ������ͻָ��ȡ�
 *
 * ��CPU���ڴ����ڲ�����ʱ����緢���˽��������Ҫ��CPU��ͣ��ǰ�Ĺ���תȥ��������첽�¼���
 * ������Ϻ��ٻص�ԭ�����жϵĵ�ַ������ԭ���Ĺ����������Ĺ��̳�Ϊ�жϡ�ʵ����һ���ܵ�
 * ϵͳ��Ϊ�ж�ϵͳ������CPU�жϵ�����Դ��Ϊ�ж�Դ��������ж�Դͬʱ��CPU�����ж�ʱ���ʹ���
 * һ���ж�����Ȩ�����⡣ͨ�������ж�Դ�����ȼ������ȴ���������¼����ж�����Դ��������
 * ��Ӧ������ߵ��ж�����
 *
 * ���жϲ���ʱ��CPU�������µ�˳��ִ�У�
 * - ��1�����浱ǰ�����״̬��Ϣ��
 * - ��2�������쳣���жϴ�������PC�Ĵ�����
 * - ��3���ѿ���Ȩת��������������ʼִ�У�
 * - ��4����������ִ�����ʱ���ָ�������״̬��Ϣ��
 * - ��5�����쳣���ж��з��ص�ǰһ������ִ�е㡣
 *
 * �ж�ʹ��CPU�������¼�����ʱ�Ž��д�����������CPU�������ϵز�ѯ�Ƿ�����Ӧ���¼�������
 *
 * **������**
  \code{.c}
#include <rthw.h>
#include <rtthread.h>

#define THREAD_PRIORITY      20
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* ͬʱ���ʵ�ȫ�ֱ��� */
static rt_uint32_t cnt;
void thread_entry(void *parameter)
{
    rt_uint32_t no;
    rt_uint32_t level;

    no = (rt_uint32_t) parameter;
    while (1)
    {
        /* �ر��ж� */
        level = rt_hw_interrupt_disable();
        cnt += no;
        /* �ָ��ж� */
        rt_hw_interrupt_enable(level);

        rt_kprintf("protect thread[%d]'s counter is %d\n", no, cnt);
        rt_thread_mdelay(no * 10);
    }
}

/* �û�Ӧ�ó������ */
int interrupt_sample(void)
{
    rt_thread_t thread;

    /* ����t1�߳� */
    thread = rt_thread_create("thread1", thread_entry, (void *)10,
                              THREAD_STACK_SIZE,
                              THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL)
        rt_thread_startup(thread);


    /* ����t2�߳� */
    thread = rt_thread_create("thread2", thread_entry, (void *)20,
                              THREAD_STACK_SIZE,
                              THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL)
        rt_thread_startup(thread);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(interrupt_sample, interrupt sample);

  \endcode
 */

/**
 * @defgroup Hook ϵͳ���Ӻ���
 * @brief ϵͳ���Ӻ���
 *
 * Ϊ��������ʱ���ٺͼ�¼RT-Thread�Ļ��������һ�ֹ��ӻ��ơ�
 *
 * ���Ӻ�������һЩ���������õ�һ�ຯ����
 * ���Ӻ���������
 * - �����Ӻ������ڴ�����ɾ������ȡ�ͷ��õȶ���ʱ���á�
 * - ���������Ӻ��������߳��л��Ϳ����߳�ѭ���е��á�
 * - �ڴ湳�Ӻ������ڷ�����ͷ��ڴ��ʱ���á�
 * - ��ʱ�����Ӻ������ڶ�ʱ����ʱʱ���á�
 *
 * **������**
  \code{.c}
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

  \endcode
 */

 
/**
 * @defgroup KernelService �����ں˷���
 * @brief �ں����������õķ���
 */

/**
 * @defgroup Error �������
 * @brief �������
 *
 * ������������ʶ�������ִ��� ����������ʱ�������õ�ǰ�̵߳Ĵ���š�
 * see @ref _rt_errno
 */
 
/**
 * @defgroup example �ں�����
 *
 * �ṩ�ں�API��ص�ʹ�����̣�
 */

/*@}*/



 
/**
 * @addtogroup IPC
 */
/*@{*/

 
 /**
 * @defgroup semaphore �ź���
 * @brief �ź���
 *
 * �ź�����һ�����͵����ڽ���̼߳�ͬ��������ں˶����߳̿��Ի�ȡ���ͷ�����
 * �Ӷ��ﵽͬ���򻥳��Ŀ�ġ��ź�������һ��Կ�ף���һ���ٽ�������ס��ֻ����
 * ��Կ�׵��߳̽��з��ʣ��߳��õ���Կ�ף��������������ٽ��������뿪���Կ��
 * ���ݸ��Ŷ��ں���ĵȴ��̣߳��ú����߳����ν����ٽ�����
 *
 * �ź�����һ�ַǳ�����ͬ����ʽ�����������ڶ��ֳ����С��γ�����ͬ������Դ����
 * �ȹ�ϵ��Ҳ�ܷ���������߳����̣߳��ж����̵߳�ͬ���С�
 *
 * **������**
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

	\code{.c}

#include <rtthread.h>

#define THREAD_PRIORITY       6
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      5

/* �������5��Ԫ���ܹ������� */
#define MAXSEM 5

/* ���ڷ����������������� */
rt_uint32_t array[MAXSEM];

/* ָ�������ߡ���������array�����еĶ�дλ�� */
static rt_uint32_t set, get;

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t producer_tid = RT_NULL;
static rt_thread_t consumer_tid = RT_NULL;

struct rt_semaphore sem_lock;
struct rt_semaphore sem_empty, sem_full;

/* �������߳���� */
void producer_thread_entry(void *parameter)
{
    int cnt = 0;

    /* ����10�� */
    while (cnt < 10)
    {
        /* ��ȡһ����λ */
        rt_sem_take(&sem_empty, RT_WAITING_FOREVER);

        /* �޸�array���ݣ����� */
        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        array[set % MAXSEM] = cnt + 1;
        rt_kprintf("the producer generates a number: %d\n", array[set % MAXSEM]);
        set++;
        rt_sem_release(&sem_lock);

        /* ����һ����λ */
        rt_sem_release(&sem_full);
        cnt++;

        /* ��ͣһ��ʱ�� */
        rt_thread_mdelay(20);
    }

    rt_kprintf("the producer exit!\n");
}

/* �������߳���� */
void consumer_thread_entry(void *parameter)
{
    rt_uint32_t sum = 0;

    while (1)
    {
        /* ��ȡһ����λ */
        rt_sem_take(&sem_full, RT_WAITING_FOREVER);

        /* �ٽ������������в��� */
        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        sum += array[get % MAXSEM];
        rt_kprintf("the consumer[%d] get a number: %d\n", (get % MAXSEM), array[get % MAXSEM]);
        get++;
        rt_sem_release(&sem_lock);

        /* �ͷ�һ����λ */
        rt_sem_release(&sem_empty);

        /* ������������10����Ŀ��ֹͣ���������߳���Ӧֹͣ */
        if (get == 10) break;

        /* ��ͣһС��ʱ�� */
        rt_thread_mdelay(50);
    }

    rt_kprintf("the consumer sum is: %d\n", sum);
    rt_kprintf("the consumer exit!\n");
}

int producer_consumer(void)
{
    set = 0;
    get = 0;

    /* ��ʼ��3���ź��� */
    rt_sem_init(&sem_lock, "lock",     1,      RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_empty, "empty",   MAXSEM, RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_full, "full",     0,      RT_IPC_FLAG_FIFO);

    /* �����������߳� */
    producer_tid = rt_thread_create("producer",
                                    producer_thread_entry, RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (producer_tid != RT_NULL)
    {
        rt_thread_startup(producer_tid);
    }
    else
    {
        rt_kprintf("create thread producer failed");
        return -1;
    }

    /* �����������߳� */
    consumer_tid = rt_thread_create("consumer",
                                    consumer_thread_entry, RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (consumer_tid != RT_NULL)
    {
        rt_thread_startup(consumer_tid);
    }
    else
    {
        rt_kprintf("create thread consumer failed");
        return -1;
    }

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(producer_consumer, producer_consumer sample);

	\endcode
 *
 */

 /**
 * @defgroup mutex ������
 * @brief ������
 *
 * �������ֽ��໥�ų���ź�������һ������Ķ�ֵ���ź����������ź�����ͬ���ǣ�
 * ��֧�ֻ���������Ȩ���ݹ�����Լ���ֹ���ȼ���ת�����ԡ�
 *
 * ��������ʹ�ñȽϵ�һ����Ϊ�����ź�����һ�֣�����������������ʽ���ڡ�
 * �ڳ�ʼ����ʱ�򣬻�������Զ�����ڿ�����״̬�������̳߳��е�ʱ��������תΪ
 * ������״̬�����������ʺ��ڣ�
 *
 * - �̶߳�γ��л�����������¡��������Ա���ͬһ�̶߳�εݹ���ж�������������⣻
 * - ���ܻ����ڶ��߳�ͬ����������ȼ���ת�������
 *
 * **������**
 \code{.c}

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
 \endcode
 *
 */

 /**
 * @defgroup event �¼�
 * @brief �¼�
 *
 * �¼���Ҫ�����̼߳��ͬ�������ź�����ͬ�������ص��ǿ���ʵ��һ�Զ࣬��Զ��ͬ����
 * �¼�����һ�������ǣ������߳̿ɵȴ������¼���������¼���Ӧһ���̻߳����̡߳�
 * ͬʱ�����̵߳ȴ��Ĳ�������ѡ���� ���߼��� �������� ���߼��롱 ������
 * 
 * RT-Thread ������¼��������ص㣺
 * 
 * - �¼�ֻ���߳���أ��¼����໥������ÿ���߳�ӵ�� 32 ���¼���־������һ�� 32 bit 
 * �޷������������м�¼��ÿһ�� bit ����һ���¼������ɸ��¼�����һ���¼�����
 * - �¼�������ͬ�������ṩ���ݴ��书�ܣ�
 * - �¼����Ŷ��ԣ���������̷߳���ͬһ�¼� (����̻߳�δ���ü�����)����Ч����ͬ��ֻ
 * ����һ�Ρ�
 *
 * **������**
  \code{.c}
#include <rtthread.h>

#define THREAD_PRIORITY      9
#define THREAD_TIMESLICE     5

#define EVENT_FLAG3 (1 << 3)
#define EVENT_FLAG5 (1 << 5)

/* �¼����ƿ� */
static struct rt_event event;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;

/* �߳�1��ں��� */
static void thread1_recv_event(void *param)
{
    rt_uint32_t e;

    /* ��һ�ν����¼����¼�3���¼�5����һ�����Դ����߳�1�������������¼���־ */
    if (rt_event_recv(&event, (EVENT_FLAG3 | EVENT_FLAG5),
                      RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER, &e) == RT_EOK)
    {
        rt_kprintf("thread1: OR recv event 0x%x\n", e);
    }

    rt_kprintf("thread1: delay 1s to prepare the second event\n");
    rt_thread_mdelay(1000);

    /* �ڶ��ν����¼����¼�3���¼�5������ʱ�ſ��Դ����߳�1�������������¼���־ */
    if (rt_event_recv(&event, (EVENT_FLAG3 | EVENT_FLAG5),
                      RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER, &e) == RT_EOK)
    {
        rt_kprintf("thread1: AND recv event 0x%x\n", e);
    }
    rt_kprintf("thread1 leave.\n");
}


ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;

/* �߳�2��� */
static void thread2_send_event(void *param)
{
    rt_kprintf("thread2: send event3\n");
    rt_event_send(&event, EVENT_FLAG3);
    rt_thread_mdelay(200);

    rt_kprintf("thread2: send event5\n");
    rt_event_send(&event, EVENT_FLAG5);
    rt_thread_mdelay(200);

    rt_kprintf("thread2: send event3\n");
    rt_event_send(&event, EVENT_FLAG3);
    rt_kprintf("thread2 leave.\n");
}

int event_sample(void)
{
    rt_err_t result;

    /* ��ʼ���¼����� */
    result = rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event failed.\n");
        return -1;
    }

    rt_thread_init(&thread1,
                   "thread1",
                   thread1_recv_event,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack),
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);

    rt_thread_init(&thread2,
                   "thread2",
                   thread2_send_event,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(event_sample, event sample);

  \endcode
 */

 /**
 * @defgroup mailbox ����
 * @brief ����
 *
 *������һ�ּ򵥵��̼߳���Ϣ���ݷ�ʽ���� RT-Thread ����ϵͳ��ʵ�����ܹ�һ�δ��� 4 �ֽ��ʼ���
 * ��������߱�һ���Ĵ洢���ܣ��ܹ�����һ���������ʼ��� (�ʼ����ɴ�������ʼ������ʱָ����
 * ��������)��������һ���ʼ�����󳤶��� 4 �ֽڣ����������ܹ����ڲ����� 4 �ֽڵ���Ϣ���ݣ�
 * �����͵���Ϣ���ȴ��������Ŀʱ�Ͳ����ٲ�������ķ�ʽ��
 *
 * **������**
  \code{.c}
#include <rtthread.h>

#define THREAD_PRIORITY      10
#define THREAD_TIMESLICE     5

/* ������ƿ� */
static struct rt_mailbox mb;
/* ���ڷ��ʼ����ڴ�� */
static char mb_pool[128];

static char mb_str1[] = "I'm a mail!";
static char mb_str2[] = "this is another mail!";
static char mb_str3[] = "over";

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;

/* �߳�1��� */
static void thread1_entry(void *parameter)
{
    char *str;

    while (1)
    {
        rt_kprintf("thread1: try to recv a mail\n");

        /* ����������ȡ�ʼ� */
        if (rt_mb_recv(&mb, (rt_uint32_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("thread1: get a mail from mailbox, the content:%s\n", str);
            if (str == mb_str3)
                break;

            /* ��ʱ100ms */
            rt_thread_mdelay(100);
        }
    }
    /* ִ������������� */
    rt_mb_detach(&mb);
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;

/* �߳�2��� */
static void thread2_entry(void *parameter)
{
    rt_uint8_t count;

    count = 0;
    while (count < 10)
    {
        count ++;
        if (count & 0x1)
        {
            /* ����mb_str1��ַ�������� */
            rt_mb_send(&mb, (rt_uint32_t)&mb_str1);
        }
        else
        {
            /* ����mb_str2��ַ�������� */
            rt_mb_send(&mb, (rt_uint32_t)&mb_str2);
        }

        /* ��ʱ200ms */
        rt_thread_mdelay(200);
    }

    /* �����ʼ������߳�1���߳�2�Ѿ����н��� */
    rt_mb_send(&mb, (rt_uint32_t)&mb_str3);
}

int mailbox_sample(void)
{
    rt_err_t result;

    /* ��ʼ��һ��mailbox */
    result = rt_mb_init(&mb,
                        "mbt",                      /* ������mbt */
                        &mb_pool[0],                /* �����õ����ڴ����mb_pool */
                        sizeof(mb_pool) / 4,        /* �����е��ʼ���Ŀ����Ϊһ���ʼ�ռ4�ֽ� */
                        RT_IPC_FLAG_FIFO);          /* ����FIFO��ʽ�����̵߳ȴ� */
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }

    rt_thread_init(&thread1,
                   "thread1",
                   thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);

    rt_thread_init(&thread2,
                   "thread2",
                   thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(mailbox_sample, mailbox sample);

  \endcode
 *
 */
 
 /**
 * @defgroup messagequeue ��Ϣ����
 * @brief ��Ϣ����
 *
 * ��Ϣ��������һ�ֳ��õ��̼߳�ͨѶ��ʽ�����ܹ����������̻߳��жϷ��������в��̶����ȵ���Ϣ��
 * ������Ϣ�������Լ����ڴ�ռ��С������߳�Ҳ�ܹ�����Ϣ�����ж�ȡ��Ӧ����Ϣ��������Ϣ������
 * �յ�ʱ�򣬿��Թ����ȡ�̡߳������µ���Ϣ����ʱ��������߳̽��������Խ��ղ�������Ϣ����Ϣ
 * ������һ���첽��ͨ�ŷ�ʽ��
 * 
 * ��Ϣ���п���Ӧ���ڷ��Ͳ�������Ϣ�ĳ��ϣ������߳����̼߳����Ϣ�������Լ��жϷ���������
 * ���͸��̵߳���Ϣ���жϷ������̲����ܽ�����Ϣ����
 *
 * **������**
  \code{.c}
#include <rtthread.h>

/* ��Ϣ���п��ƿ� */
static struct rt_messagequeue mq;
/* ��Ϣ�������õ��ķ�����Ϣ���ڴ�� */
static rt_uint8_t msg_pool[2048];

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
/* �߳�1��ں��� */
static void thread1_entry(void *parameter)
{
    char buf = 0;
    rt_uint8_t cnt = 0;

    while (1)
    {
        /* ����Ϣ�����н�����Ϣ */
        if (rt_mq_recv(&mq, &buf, sizeof(buf), RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("thread1: recv msg from msg queue, the content:%c\n", buf);
            if (cnt == 19)
            {
                break;
            }
        }
        /* ��ʱ50ms */
        cnt++;
        rt_thread_mdelay(50);
    }
    rt_kprintf("thread1: detach mq \n");
    rt_mq_detach(&mq);
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
/* �߳�2��� */
static void thread2_entry(void *parameter)
{
    int result;
    char buf = 'A';    
    rt_uint8_t cnt = 0;
    
    while (1)
    {
        if (cnt == 8)
        {
            /* ���ͽ�����Ϣ����Ϣ������ */
            result = rt_mq_urgent(&mq, &buf, 1);
            if (result != RT_EOK)
            {
                rt_kprintf("rt_mq_urgent ERR\n");
            }
            else
            {
                rt_kprintf("thread2: send urgent message - %c\n", buf);
            }
        }
        else if (cnt >= 20)/* ����20����Ϣ֮���˳� */
        {
            rt_kprintf("message queue stop send, thread2 quit\n");
            break;
        }
        else
        {
            /* ������Ϣ����Ϣ������ */
            result = rt_mq_send(&mq, &buf, 1);
            if (result != RT_EOK)
            {
                rt_kprintf("rt_mq_send ERR\n");
            }

            rt_kprintf("thread2: send message - %c\n", buf);
        }
        buf++;
        cnt++;
        /* ��ʱ5ms */
        rt_thread_mdelay(5);
    }
}

/* ��Ϣ����ʾ���ĳ�ʼ�� */
int msgq_sample(void)
{
    rt_err_t result;

    /* ��ʼ����Ϣ���� */
    result = rt_mq_init(&mq,
                        "mqt",
                        &msg_pool[0],               /* �ڴ��ָ��msg_pool */
                        1,                          /* ÿ����Ϣ�Ĵ�С�� 1 �ֽ� */
                        sizeof(msg_pool),           /* �ڴ�صĴ�С��msg_pool�Ĵ�С */
                        RT_IPC_FLAG_FIFO);          /* ����ж���̵߳ȴ������������ȵõ��ķ���������Ϣ */

    if (result != RT_EOK)
    {
        rt_kprintf("init message queue failed.\n");
        return -1;
    }

    rt_thread_init(&thread1,
                   "thread1",
                   thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack), 25, 5);
    rt_thread_startup(&thread1);

    rt_thread_init(&thread2,
                   "thread2",
                   thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack), 25, 5);
    rt_thread_startup(&thread2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(msgq_sample, msgq sample);

  \endcode
 */
 
 
 /**
 * @defgroup Signal �ź�
 * @brief �ź�
 *
 * ���ж��źţ�signal���ּ��Ϊ�źţ�����֪ͨ�̷߳������첽�¼��������������Ƕ��жϻ��Ƶ�һ��ģ�⣬
 * ��ԭ���ϣ�һ���߳��յ�һ���ź��봦�����յ�һ���ж��������˵��һ���ġ��ź����̼߳�ͨ�Ż�����Ψһ��
 * �첽ͨ�Ż��ƣ�һ���̲߳���ͨ���κβ������ȴ��źŵĵ����ʵ�ϣ��߳�Ҳ��֪���źŵ���ʲôʱ�򵽴
 * �߳�֮����Ի���ͨ������ kill �������ж��źš�
 *
 * �յ��źŵ��̶߳Ը����ź��в�ͬ�Ĵ����������������Է�Ϊ���ࣺ
 *
 * - ��һ���������жϵĴ�����򣬶�����Ҫ������źţ��߳̿���ָ�����������ɸú���������
 * - �ڶ��ַ����ǣ�����ĳ���źţ��Ը��źŲ����κδ�������δ������һ����
 * - �����ַ����ǣ��Ը��źŵĴ�����ϵͳ��Ĭ��ֵ��
 *
 * **������**
  \code{.c}
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

  \endcode
 */
 
/*@}*/




 
/**
 * @addtogroup KernelService
 */
/*@{*/


 /**
 * @defgroup str �ַ�������
 * @brief �ַ�������
 *
 *
 */

 /**
 * @defgroup mem �ڴ����
 * @brief �ڴ����
 *
 *
 */
 
 /**
 * @defgroup list ˫������
 * @brief ˫������
 *
 *
 */
 
 /**
 * @defgroup slist ��������
 * @brief ��������
 *
 *
 */
 

/*@}*/


