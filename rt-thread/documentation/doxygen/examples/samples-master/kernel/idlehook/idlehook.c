/*
 * �����嵥����������������
 *
 * �������������һ���������������ڼ���CPUʹ���ʣ�������һ���߳�ѭ����ӡCPUʹ����
 * ͨ���޸�CPUʹ���ʴ�ӡ�߳��е�����tickʱ����Կ�����ͬ��CPUʹ����
 */

#include <rtthread.h>
#include <rthw.h>

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid = RT_NULL;

#define CPU_USAGE_CALC_TICK    10
#define CPU_USAGE_LOOP        100

static rt_uint8_t  cpu_usage_major = 0, cpu_usage_minor = 0;

/* ��¼CPUʹ����Ϊ0ʱ����count�� */
static rt_uint32_t total_count = 0;

/* ���������Ӻ��� */
static void cpu_usage_idle_hook()
{
    rt_tick_t tick;
    rt_uint32_t count;
    volatile rt_uint32_t loop;

    if (total_count == 0)
    {
        /* ��ȡ total_count */
        rt_enter_critical();
        tick = rt_tick_get();
        while (rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
        {
            total_count ++;
            loop = 0;
            while (loop < CPU_USAGE_LOOP) loop ++;
        }
        rt_exit_critical();
    }

    count = 0;
    /* ����CPUʹ���� */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
    {
        count ++;
        loop  = 0;
        while (loop < CPU_USAGE_LOOP) loop ++;
    }

    /* ���������ٷֱ��������ֺ�С������ */
    if (count < total_count)
    {
        count = total_count - count;
        cpu_usage_major = (count * 100) / total_count;
        cpu_usage_minor = ((count * 100) % total_count) * 100 / total_count;
    }
    else
    {
        total_count = count;

        /* CPUʹ����Ϊ0 */
        cpu_usage_major = 0;
        cpu_usage_minor = 0;
    }
}

void cpu_usage_get(rt_uint8_t *major, rt_uint8_t *minor)
{
    RT_ASSERT(major != RT_NULL);
    RT_ASSERT(minor != RT_NULL);

    *major = cpu_usage_major;
    *minor = cpu_usage_minor;
}

/* CPUʹ���ʴ�ӡ�߳���� */
static void thread_entry(void *parameter)
{
    rt_uint8_t major, minor;

    while (1)
    {
        cpu_usage_get(&major, &minor);
        rt_kprintf("cpu usage: %d.%d%\n", major, minor);

        /* ����50��OS Tick */
        /* �ֶ��޸Ĵ˴����� tick ʱ�䣬����ģ��ʵ�ֲ�ͬ��CPUʹ���� */
        rt_thread_delay(50);
    }
}

int cpu_usage_init()
{
    /* ���ÿ����̹߳��� */
    rt_thread_idle_sethook(cpu_usage_idle_hook);

    /* �����߳� */
    tid = rt_thread_create("thread",
                           thread_entry, RT_NULL, /* �߳������thread_entry, ��ڲ�����RT_NULL */
                           THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(cpu_usage_init, idle hook sample);
