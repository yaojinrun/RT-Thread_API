/*
 * �����嵥���߳��ó�������
 * ����������У�������������ͬ���ȼ����̣߳� ���ǻ�ͨ��rt_thread_yield
 * �ӿڰѴ������໥�ø��Է�����ִ�С�
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
/* �߳�1��� */
static void thread1_entry(void* parameter)
{
    rt_uint32_t count = 0;

    while (1)
    {
        /* ��ӡ�߳�1����� */
        rt_kprintf("thread1: count = %d\n", count ++);

        /* ִ��yield��Ӧ���л���thread2ִ�� */
        rt_thread_yield();
    }
}

/* �߳�2��� */
static void thread2_entry(void* parameter)
{
    rt_uint32_t count = 0;

    while (1)
    {
        /* ��ӡ�߳�2����� */
        rt_kprintf("thread2: count = %d\n", count ++);

        /* ִ��yield��Ӧ���л���thread1ִ�� */
        rt_thread_yield();
    }
}

/* �߳��ó�������ʾ���ĳ�ʼ�� */
int thread_yield_init(void)
{
    /* �����߳�1 */
    tid1 = rt_thread_create("thread",
                            thread1_entry,      /* �߳������thread1_entry */
                            RT_NULL,            /* ��ڲ�����RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    /* �����߳�2 */
    tid2 = rt_thread_create("thread",
                            thread2_entry,      /* �߳������thread2_entry */
                            RT_NULL,            /* ��ڲ�����RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(thread_yield_init, thread yield);
