/*
 * �����嵥����̬�߳�����
 *
 * ������ӻ��ʼ��������̬�߳�(t1��t2)����t2�л��t1�������������
 * t1����󽫲������У�״̬Ҳ����Ϊ��ʼ״̬
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* �߳�1���ƿ� */
static struct rt_thread thread1;
/* �߳�1ջ */
static rt_uint8_t thread1_stack[THREAD_STACK_SIZE];
/* �߳�2���ƿ� */
static struct rt_thread thread2;
/* �߳�2ջ */
static rt_uint8_t thread2_stack[THREAD_STACK_SIZE];

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
    /* �߳�2ӵ�нϸߵ����ȼ�������ռ�߳�1�����ִ�� */

    /* �߳�2��������˯��10��OS Tick */
    rt_thread_delay(10);

    /*
     * �߳�2���Ѻ�ֱ��ִ���߳�1���룬�߳�1���Ӿ����̶߳�����ɾ��
     */
    rt_thread_detach(&thread1);

    /*
     * �߳�2��������10��OS TickȻ���˳�
     */
    rt_thread_delay(10);

    /*
     * �߳�2���н�����Ҳ���Զ����Ӿ���������ɾ�����������̶߳���
     */
}

/* �߳�����ʾ���ĳ�ʼ�� */
int thread_detach_init(void)
{
    rt_err_t result;

    /* ��ʼ���߳�1 */
    result = rt_thread_init(&thread1, "t1",     /* �߳�����t1 */
                            thread1_entry,      /* �̵߳������thread1_entr */
                            RT_NULL,            /* ��ڲ�����RT_NULL*/
                            &thread1_stack[0],  /* �߳�ջ��thread1_stack */
                            sizeof(thread1_stack),
                            THREAD_PRIORITY, 10);
    if (result == RT_EOK)                       /* ���������ȷ�������߳�1 */
        rt_thread_startup(&thread1);

    /* ��ʼ���߳�2 */
    result = rt_thread_init(&thread2, "t2",     /* �߳�����t2 */
                            thread2_entry,      /* �̵߳������thread2_entry */
                            RT_NULL,            /* ��ڲ�����RT_NULL*/
                            &thread2_stack[0],  /* �߳�ջ��thread2_stack */
                            sizeof(thread2_stack),
                            THREAD_PRIORITY - 1, 10);
    if (result == RT_EOK)                       /* ���������ȷ�������߳�2 */
        rt_thread_startup(&thread2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(thread_detach_init, thread detach);
