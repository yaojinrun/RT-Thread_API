/*
 * �����嵥���ڴ������
 *
 * �������ᴴ��һ����̬���ڴ�ض���2����̬�̡߳������̻߳���ͼ�ֱ���ڴ���л��
 * �ڴ��
 */
#include <rtthread.h>

static rt_uint8_t *ptr[48];
static rt_uint8_t mempool[4096];
static struct rt_mempool mp;

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

/* �߳�1��� */
static void thread1_entry(void *parameter)
{
    int i, count = 0;
    char *block;

    while (1)
    {
        for (i = 0; i < 48; i++)
        {
            /* �����ڴ�� */
            if (ptr[i] == RT_NULL)
            {
                ptr[i] = rt_mp_alloc(&mp, RT_WAITING_FOREVER);
                rt_kprintf("allocate No.%d\n", i);
            }
        }

        /* ��������һ���ڴ�飬��Ϊ�Ѿ�û���ڴ�飬�߳�Ӧ�ñ����� */
        block = rt_mp_alloc(&mp, RT_WAITING_FOREVER);
        rt_kprintf("allocate the block mem\n");
        /* �ͷ�����ڴ�� */
        rt_mp_free(block);
        block = RT_NULL;

        /* ����10��OS Tick */
        rt_thread_delay(10);

        /* ѭ��10�κ��˳��߳�1 */
        if (count++ >= 10) break;
    }
}

/* �߳�2��ڣ��߳�2�����ȼ����߳�1�ͣ�Ӧ���߳�1�Ȼ��ִ�С�*/
static void thread2_entry(void *parameter)
{
    int i, count = 0;

    while (1)
    {
        rt_kprintf("try to release block\n");

        for (i = 0 ; i < 48; i ++)
        {
            /* �ͷ����з���ɹ����ڴ�� */
            if (ptr[i] != RT_NULL)
            {
                rt_kprintf("release block %d\n", i);

                rt_mp_free(ptr[i]);
                ptr[i] = RT_NULL;
            }
        }

        /* ����10��OS Tick */
        rt_thread_delay(10);

        /* ѭ��10�κ��˳��߳�2 */
        if (count++ >= 10) break;
    }
}

int mempool_sample_init()
{
    int i;
    for (i = 0; i < 48; i ++) ptr[i] = RT_NULL;

    /* ��ʼ���ڴ�ض��� */
    rt_mp_init(&mp, "mp1", &mempool[0], sizeof(mempool), 80);

    /* �����߳�1 */
    tid1 = rt_thread_create("t1",
                            thread1_entry, RT_NULL, /* �߳������thread1_entry, ��ڲ�����RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    /* �����߳�2 */
    tid2 = rt_thread_create("t2",
                            thread2_entry, RT_NULL, /* �߳������thread2_entry, ��ڲ�����RT_NULL */
                            THREAD_STACK_SIZE, THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(mempool_sample_init, mempool sample);
