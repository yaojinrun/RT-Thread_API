/*
 * �����嵥������������������
 *
 * ��������н����������߳�����ʵ������������������
 */
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
        rt_thread_delay(50);
    }

    rt_kprintf("the producer exit!\n");
}

/* �������߳���� */
void consumer_thread_entry(void *parameter)
{
    rt_uint32_t no;
    rt_uint32_t sum;

    /* ��n���̣߳�����ڲ��������� */
    no = (rt_uint32_t)parameter;

    sum = 0;
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
        rt_thread_delay(10);
    }

    rt_kprintf("the consumer[%d] sum is %d \n ", no, sum);
    rt_kprintf("the consumer[%d] exit!\n");
}

int semaphore_producer_consumer_init()
{
	set = 0;
	get = 0;

    /* ��ʼ��3���ź��� */
    rt_sem_init(&sem_lock, "lock",     1,      RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_empty, "empty",    MAXSEM, RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_full, "full",     0,      RT_IPC_FLAG_FIFO);

    /* �����߳�1 */
    producer_tid = rt_thread_create("producer",
                                    producer_thread_entry, RT_NULL, /* �߳������producer_thread_entry, ��ڲ�����RT_NULL */
                                    THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (producer_tid != RT_NULL)
        rt_thread_startup(producer_tid);

    /* �����߳�2 */
    consumer_tid = rt_thread_create("consumer",
                                    consumer_thread_entry, RT_NULL, /* �߳������consumer_thread_entry, ��ڲ�����RT_NULL */
                                    THREAD_STACK_SIZE, THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (consumer_tid != RT_NULL)
        rt_thread_startup(consumer_tid);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(semaphore_producer_consumer_init, producer_consumer sample);
