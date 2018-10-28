/*
 * �����嵥��������ɾ���߳�
 *
 * ������ӻᴴ�������̣߳���һ���߳���ɾ������һ���̡߳�
 */

#include <rtthread.h>

#define THREAD_PRIORITY     25
#define THREAD_STACK_SIZE   512
#define THREAD_TIMESLICE    5

/*
 * �߳�ɾ��(rt_thread_delete)�������ʺ��ڶ�̬�̣߳�Ϊ����һ���߳�
 * �з�����һ���̵߳Ŀ��ƿ飬���԰��߳̿�ָ��������ȫ�������Թ�ȫ
 * �ַ���
 */
static rt_thread_t tid1 = RT_NULL, tid2 = RT_NULL;
/* �߳�1����ں��� */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count = 0;

    while (1)
    {
        /* �߳�1���õ����ȼ����У�һֱ��ӡ����ֵ */
        rt_kprintf("thread count: %d\n", count ++);
    }
}

static void thread1_cleanup(struct rt_thread *tid)
{
    if (tid != tid1)
    {
        return ;
    }
    rt_kprintf("thread1 end\n");
    tid1 = RT_NULL;
}

/* �߳�2����ں��� */
static void thread2_entry(void *parameter)
{
    /* �߳�2ӵ�нϸߵ����ȼ�������ռ�߳�1�����ִ�� */

    /* �߳�2��������˯��10��OS Tick */
    rt_thread_delay(10);

    /*
     * �߳�2���Ѻ�ֱ��ɾ���߳�1��ɾ���߳�1���߳�1�Զ���������߳�
     * ����
     */
    rt_thread_delete(tid1);

    /*
     * �߳�2��������10��OS TickȻ���˳����߳�2���ߺ�Ӧ�л���idle�߳�
     * idle�߳̽�ִ���������߳�1���ƿ���߳�ջ��ɾ��
     */
    rt_thread_delay(10);
}

static void thread2_cleanup(struct rt_thread *tid)
{
    /*
     * �߳�2���н�����Ҳ���Զ���ɾ��(�߳̿��ƿ���߳�ջ��idle��
     * �����ͷ�)
     */

    if (tid != tid2)
    {
        return ;
    }
    rt_kprintf("thread2 end\n");
    tid2 = RT_NULL;
}

/* �߳�ʾ���ĳ�ʼ�� */
int thread_sample_init()
{
    /* �����߳�1 */
    tid1 = rt_thread_create("t1", /* �߳�1��������t1 */
                            thread1_entry, RT_NULL,   /* �����thread1_entry��������RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL) /* �������߳̿��ƿ飬��������߳� */
    {
        tid1->cleanup = thread1_cleanup;
        rt_thread_startup(tid1);
    }

    /* �����߳�1 */
    tid2 = rt_thread_create("t2", /* �߳�1��������t2 */
                            thread2_entry, RT_NULL,   /* �����thread2_entry��������RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid2 != RT_NULL) /* �������߳̿��ƿ飬��������߳� */
    {
        tid2->cleanup = thread2_cleanup;
        rt_thread_startup(tid2);
    }

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(thread_sample_init, run thread sample);
