/*
* �����嵥����ʱ������
*
* ������̻ᴴ��������̬��ʱ������һ���ǵ��ζ�ʱ��һ���������ԵĶ�ʱ
* */
#include <rtthread.h>

/* ��ʱ���Ŀ��ƿ� */
static rt_timer_t timer1;
static rt_timer_t timer2;
static int cnt = 0;

/* ��ʱ��1��ʱ���� */
static void timeout1(void *parameter)
{
    rt_tick_t timeout = 300;

    rt_kprintf("periodic timer is timeout\n");

    /* ����10�� */
    if (cnt++ >= 10)
    {
        rt_timer_control(timer1, RT_TIMER_CTRL_SET_ONESHOT, (void *)&timeout);
    }
}

/* ��ʱ��2��ʱ���� */
static void timeout2(void *parameter)
{
    rt_kprintf("one shot timer is timeout\n");
}

int timer_sample_init(void)
{
    /* ������ʱ��1 */
    timer1 = rt_timer_create("timer1", /* ��ʱ�������� timer1 */
                             timeout1, /* ��ʱʱ�ص��Ĵ����� */
                             RT_NULL, /* ��ʱ��������ڲ��� */
                             10, /* ��ʱ���ȣ���OS TickΪ��λ����10��OS Tick */
                             RT_TIMER_FLAG_PERIODIC); /* �����Զ�ʱ�� */

    /* ������ʱ�� */
    if (timer1 != RT_NULL) rt_timer_start(timer1);

    /* ������ʱ��2 */
    timer2 = rt_timer_create("timer2", /* ��ʱ�������� timer2 */
                             timeout2, /* ��ʱʱ�ص��Ĵ����� */
                             RT_NULL, /* ��ʱ��������ڲ��� */
                             30, /* ��ʱ����Ϊ30��OS Tick */
                             RT_TIMER_FLAG_ONE_SHOT); /* ���ζ�ʱ�� */

    /* ������ʱ�� */
    if (timer2 != RT_NULL) rt_timer_start(timer2);
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(timer_sample_init, timer sample);
