/*
 * �����嵥����ʼ����̬�߳�
 *
 * ���������ʼ��2����̬�̣߳�����ӵ�й�ͬ����ں���������������ͬ
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* �߳�1���ƿ� */
static struct rt_thread thread1;
/* �߳�1ջ */
ALIGN(4)
static rt_uint8_t thread1_stack[THREAD_STACK_SIZE];
/* �߳�2���ƿ� */
static struct rt_thread thread2;
/* �߳�2ջ */
ALIGN(4)
static rt_uint8_t thread2_stack[THREAD_STACK_SIZE];

/* �߳���� */
static void thread_entry(void* parameter)
{
    rt_uint32_t count = 0;
    rt_uint32_t no = (rt_uint32_t) parameter; /* �����ȷ����ڲ��� */

    while (1)
    {
        /* ��ӡ�̼߳���ֵ��� */
        rt_kprintf("thread%d count: %d\n", no, count ++);

        /* ����10��OS Tick */
        rt_thread_delay(10);
    }
}

/* ��̬�߳�ʾ���ĳ�ʼ�� */
int thread_static_init()
{
    rt_err_t result;

    /* ��ʼ���߳�1 */
    result = rt_thread_init(&thread1, "t1", /* �߳�����t1 */
                            thread_entry, (void*)1, /* �̵߳������thread_entry����ڲ�����1 */
                            &thread1_stack[0], sizeof(thread1_stack), /* �߳�ջ��thread1_stack */
                            THREAD_PRIORITY, 10);
    if (result == RT_EOK) /* ���������ȷ�������߳�1 */
        rt_thread_startup(&thread1);
    else
        return -1;

    /* ��ʼ���߳�2 */
    result = rt_thread_init(&thread2, "t2", /* �߳�����t2 */
                            thread_entry, (void*)2, /* �̵߳������thread_entry����ڲ�����2 */
                            &thread2_stack[0], sizeof(thread2_stack), /* �߳�ջ��thread2_stack */
                            THREAD_PRIORITY + 1, 10);
    if (result == RT_EOK) /* ���������ȷ�������߳�2 */
        rt_thread_startup(&thread2);
    else
        return -1;

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(thread_static_init, thread static init);
