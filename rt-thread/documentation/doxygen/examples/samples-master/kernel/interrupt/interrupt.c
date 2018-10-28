/* �����嵥���ر��жϽ���ȫ�ֱ����ķ��� */
#include <rthw.h>
#include <rtthread.h>

/* ͬʱ���ʵ�ȫ�ֱ��� */
static rt_uint32_t cnt;
void thread_entry(void* parameter)
{
    rt_uint32_t no;
    rt_uint32_t level;

    no = (rt_uint32_t) parameter;
    while(1)
    {
        /* �ر��ж� */
        level = rt_hw_interrupt_disable();
        cnt += no;
        /* �ָ��ж� */
        rt_hw_interrupt_enable(level);

        rt_kprintf("thread[%d]'s counter is %d\n", no, cnt);
        rt_thread_delay(no);
    }
}

/* �û�Ӧ�ó������ */
void interrupt_sample_init()
{
    rt_thread_t thread;

    /* ����t1�߳� */
    thread = rt_thread_create("t1", thread_entry, (void*)10,
        512, 10, 5);
    if (thread != RT_NULL) rt_thread_startup(thread);

    /* ����t2�߳� */
    thread = rt_thread_create("t2", thread_entry, (void*)20,
        512, 20, 5);
    if (thread != RT_NULL) rt_thread_startup(thread);
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(interrupt_sample_init, interrupt sample);