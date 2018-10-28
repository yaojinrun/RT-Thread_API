/*
 * �����嵥�����������
 *
 * ����ƴ������򵥵����ӣ���������ÿ�ֱ�������г���Ա�Ӵ��ĵ�һ������
 * Hello Worldһ��������������Ӿʹ�����ƿ�ʼ������һ���̣߳�������ʱ�ض�
 * LED���и��£�������
 */

#include <rtthread.h>
#include <rtdevice.h>

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[ 512 ];
/* �̵߳�TCB���ƿ� */
static struct rt_thread led_thread;

static void led_thread_entry(void *parameter)
{
    unsigned int count = 0;
    /* ���� PIN ��ģʽ */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while (1)
    {
        /* ���� led */
        rt_kprintf("led on, count : %d\n", count);
        rt_pin_write(LED_PIN, 0);
        rt_thread_delay(rt_tick_from_millisecond(500));

        /* ���� led */
        rt_kprintf("led off\n");
        rt_pin_write(LED_PIN, 1);
        rt_thread_delay(rt_tick_from_millisecond(500));

        count++;
    }
}

int led_sample_init(void)
{
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t *)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }
    return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(led_sample_init, led sample);
