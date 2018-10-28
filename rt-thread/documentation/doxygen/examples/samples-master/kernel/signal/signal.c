/*
 * �����嵥���ź�����
 *
 * ������ӻᴴ�������߳��߳�1���߳�2��ÿ���̻߳�ֱ�װ�źţ�Ȼ����������̷߳����źš�
 *
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/*
 * Ϊ����һ���߳��з�����һ���̵߳Ŀ��ƿ飬���԰��߳̿�ָ���з���
 * ��һ���̵߳Ŀ��ƿ飬���԰��߳̿�ָ��������ȫ�������Թ�ȫ�ַ���
 */
static rt_thread_t tid1 = RT_NULL, tid2 = RT_NULL;

/* �߳�1���źŴ����� */
void thread1_signal_handler(int sig)
{
    rt_kprintf("thread1 received signal %d\n", sig);
}

/* �߳�2���źŴ����� */
void thread2_signal_handler(int sig)
{
    rt_kprintf("thread2 received signal %d\n", sig);
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
        rt_thread_delay(10);
    }
}

/* �߳�2����ں��� */
static void thread2_entry(void *parameter)
{
    int cnt = 0;

    /* ��װ�ź� */
    rt_signal_install(SIGUSR2, thread2_signal_handler);
    rt_signal_unmask(SIGUSR2);

    /* ����10�� */
    while (cnt < 10)
    {
        rt_kprintf("thread2 count : %d\n", cnt);

        cnt++;
        rt_thread_delay(20);
    }
}

/* �ź�ʾ���ĳ�ʼ�� */
int signal_sample_init()
{
    /* �����߳�1 */
    tid1 = rt_thread_create("t1", /* �߳�1��������t1 */
                            thread1_entry, RT_NULL,   /* �����thread1_entry��������RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);

    if (tid1 != RT_NULL) /* �������߳̿��ƿ飬��������߳� */
        rt_thread_startup(tid1);

    /* �����߳�2 */
    tid2 = rt_thread_create("t2", /* �߳�2��������t2 */
                            thread2_entry, RT_NULL,   /* �����thread2_entry��������RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);

    if (tid2 != RT_NULL) /* �������߳̿��ƿ飬��������߳� */
        rt_thread_startup(tid2);

    rt_thread_delay(50);

    /* �����ź� SIGUSR1 ���߳�1 */
    rt_thread_kill(tid1, SIGUSR1);
    /* �����ź� SIGUSR2 ���߳�2 */
    rt_thread_kill(tid2, SIGUSR2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(signal_sample_init, signal sample);
