/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup example
 */
/*@{*/


 /**
 * @defgroup int �ر��жϽ���ȫ�ֱ����ķ���
 *
 *
 * **Դ��**
  \code{.c}
#include <rthw.h>
#include <rtthread.h>

#define THREAD_PRIORITY      20
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* ͬʱ���ʵ�ȫ�ֱ��� */
static rt_uint32_t cnt;
void thread_entry(void *parameter)
{
    rt_uint32_t no;
    rt_uint32_t level;

    no = (rt_uint32_t) parameter;
    while (1)
    {
        /* �ر��ж� */
        level = rt_hw_interrupt_disable();
        cnt += no;
        /* �ָ��ж� */
        rt_hw_interrupt_enable(level);

        rt_kprintf("protect thread[%d]'s counter is %d\n", no, cnt);
        rt_thread_mdelay(no * 10);
    }
}

/* �û�Ӧ�ó������ */
int interrupt_sample(void)
{
    rt_thread_t thread;

    /* ����t1�߳� */
    thread = rt_thread_create("thread1", thread_entry, (void *)10,
                              THREAD_STACK_SIZE,
                              THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL)
        rt_thread_startup(thread);


    /* ����t2�߳� */
    thread = rt_thread_create("thread2", thread_entry, (void *)20,
                              THREAD_STACK_SIZE,
                              THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL)
        rt_thread_startup(thread);

    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(interrupt_sample, interrupt sample);

  \endcode
 */


/*@}*/

