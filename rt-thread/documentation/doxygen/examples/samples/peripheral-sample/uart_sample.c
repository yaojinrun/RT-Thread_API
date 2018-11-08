/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-15     misonyo      first implementation.
 */
/* 
 * �����嵥������һ�� ���� �豸ʹ������
 * ���̵����� uart_sample ��������ն�
 * ������ø�ʽ��uart_sample uart2
 * ������ͣ�����ڶ���������Ҫʹ�õĴ����豸���ƣ�Ϊ����ʹ��Ĭ�ϵĴ����豸
 * �����ܣ�ͨ����������ַ���"hello RT-Thread!"��Ȼ���λ���������ַ�
*/

#include <rtthread.h>

#define SAMPLE_UART_NAME       "uart2"

/* ���ڽ�����Ϣ���ź��� */
static struct rt_semaphore rx_sem;
static char uart_name[RT_NAME_MAX];
/* �������ݻص����� */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

static void serial_thread_entry(void* parameter)
{
    char ch;
    rt_device_t serial;
    char str[] = "hello RT-Thread!\r\n";

    /* ����ϵͳ�еĴ����豸 */
    serial = rt_device_find(uart_name);

    if (serial != RT_NULL)
    {
        rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
        /* �Զ�д���жϽ��շ�ʽ�򿪴����豸 */
        rt_device_open(serial, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
        /* ���ý��ջص����� */
        rt_device_set_rx_indicate(serial, uart_input);
        /* �����ַ��� */
        rt_device_write(serial, 0, str, (sizeof(str) - 1));
        
        while (1)
        {
            /* �Ӵ��ڶ�ȡһ���ֽڵ����ݣ�û�ж�ȡ����ȴ������ź��� */
            while (rt_device_read(serial, -1, &ch, 1) != 1)
            {
                /* �����ȴ������ź������ȵ��ź������ٴζ�ȡ���� */
                rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
            }
            /* ��ȡ��������ͨ�����ڴ�λ��� */
            ch = ch + 1;
            rt_device_write(serial, 0, &ch, 1);
        }
    }
    else
    {
        rt_kprintf("uart sample run failed! can't find %s device!\n",uart_name);
    }
}

static void uart_sample(int argc,char *argv[])
{
    if (argc == 2)
    {
        rt_strncpy(uart_name, argv[1], RT_NAME_MAX);
    }
    else
    {
        rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);
    }

    /* ���� serial �߳� */
    rt_thread_t thread = rt_thread_create("serial",serial_thread_entry, RT_NULL, 1024, 25, 10);
    /* �����ɹ��������߳� */
    if (thread!= RT_NULL)
    {
        rt_thread_startup(thread);
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(uart_sample, uart device sample);
