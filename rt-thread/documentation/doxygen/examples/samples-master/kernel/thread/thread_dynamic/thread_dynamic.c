/*
 * �����嵥��������̬�߳�
 *
 * �������ᴴ��2����̬�̣߳�
 * ����ӵ�й�ͬ����ں�������ͬ�����ȼ�
 * �������ǵ���ڲ�������ͬ
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
/* �߳���� */
static void thread_entry(void* parameter)
{
    rt_uint32_t count = 0;
    rt_uint32_t no = (rt_uint32_t) parameter; /* ����̵߳���ڲ��� */

    while (1)
    {
        /* ��ӡ�̼߳���ֵ��� */
        rt_kprintf("thread%d count: %d\n", no, count ++);

        /* ����10��OS Tick */
        rt_thread_delay(10);
    }
}

/* ��̬�߳��߳�ʾ���ĳ�ʼ�� */
int thread_dynamic_init()
{
    /* �����߳�1 */
    tid1 = rt_thread_create("t1",
                            thread_entry, (void*)1, /* �߳������thread_entry, ��ڲ�����1 */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
    else
        return -1;

    /* �����߳�2 */
    tid2 = rt_thread_create("t2",
                            thread_entry, (void*)2, /* �߳������thread_entry, ��ڲ�����2 */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);
    else
        return -1;

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(thread_dynamic_init, thread dynamic init);
