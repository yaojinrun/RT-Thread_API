/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-25     misonyo      first edition.
 */
/* 
 * �����嵥������һ���������Ź��豸ʹ������
 * ���̵����� iwdg_sample ��������ն�
 * ������ø�ʽ��iwdg_sample iwg
 * ������ͣ�����ڶ���������Ҫʹ�õĿ��Ź��豸���ƣ�Ϊ����ʹ������Ĭ�ϵĿ��Ź��豸
 * �����ܣ�����ͨ���豸���Ʋ��ҿ��Ź��豸��Ȼ���ʼ���豸�����ÿ��Ź��豸���ʱ�䡣
 *           Ȼ�����ÿ����̻߳ص��������ڻص��������ι����
*/

#include <rtthread.h>
#include <rtdevice.h>

#define IWDG_DEVICE_NAME    "iwg"

static rt_device_t wdg_dev;

static void idle_hook(void)
{
    /* �ڿ����̵߳Ļص�������ι�� */
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}

static int iwdg_sample(int argc,char *argv[])
{
    rt_err_t result = RT_EOK;
    rt_uint32_t timeout = 1000;    /* ��ʱʱ��Ϊ1000ms*/
    char device_name[RT_NAME_MAX];

    /* �ж������в����Ƿ�������豸���� */
    if (argc == 2)
    {
        rt_strncpy(device_name, argv[1], RT_NAME_MAX);
    }
    else
    {
        rt_strncpy(device_name, IWDG_DEVICE_NAME, RT_NAME_MAX);
    }
    /* �����豸���Ʋ��ҿ��Ź��豸����ȡ�豸��� */
    wdg_dev = rt_device_find(device_name);
    if (!wdg_dev)
    {
        rt_kprintf("find %s failed!\n",device_name);
        return RT_ERROR;
    }
    /* ��ʼ���豸 */
    result = rt_device_init(wdg_dev);
    if (result != RT_EOK)
    {
        rt_kprintf("initialize %s failed!\n",device_name);
        return RT_ERROR;
    }
    /* ���ÿ��Ź����ʱ�� */
    result = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, (void*)timeout);
    if (result != RT_EOK)
    {
        rt_kprintf("set %s timeout failed!\n",device_name);
        return RT_ERROR;
    }
    /* ���ÿ����̻߳ص����� */
    rt_thread_idle_sethook(idle_hook);

    return result;
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(iwdg_sample, iwdg sample);
