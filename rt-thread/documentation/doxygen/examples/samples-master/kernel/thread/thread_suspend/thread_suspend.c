/*
 * �����嵥�������߳�
 *
 * ��������н�����������̬�߳�(t1��t2)
 * �����ȼ��߳�t1��������һֱ�������У�
 * �����ȼ��߳�t2��һ��ʱ�̺��������ȼ��߳�t1��
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
        /* �߳�1���õ����ȼ����У�һֱ��ӡ����ֵ */
        rt_kprintf("thread count: %d\n", count ++);
    }
}

/* �߳�2��� */
static void thread2_entry(void* parameter)
{
    /* ��ʱ10��OS Tick */
    rt_thread_delay(10);

    /* �����߳�1 */
    rt_thread_suspend(tid1);

    /* ��ʱ10��OS Tick */
    rt_thread_delay(10);

    /* �߳�2�Զ��˳� */
}

/* �����߳�ʾ���ĳ�ʼ�� */
int thread_suspend_init(void)
{
    /* �����߳�1 */
    tid1 = rt_thread_create("t1",
                            thread1_entry,  /* �߳������thread1_entry */
                            RT_NULL,        /* ��ڲ�����RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    /* �����߳�2 */
    tid2 = rt_thread_create("t2",
                            thread2_entry,  /* �߳������thread2_entry */
                            RT_NULL,        /* ��ڲ�����RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(thread_suspend_init, thread suspend);
