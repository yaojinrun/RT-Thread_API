/*
 * File      : device.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2007-01-21     Bernard      the first version
 * 2010-05-04     Bernard      add rt_device_init implementation
 * 2012-10-20     Bernard      add device check in register function,
 *                             provided by Rob <rdent@iinet.net.au>
 * 2012-12-25     Bernard      return RT_EOK if the device interface not exist.
 * 2013-07-09     Grissiom     add ref_count support
 * 2016-04-02     Bernard      fix the open_flag initialization issue.
 */

#include <rtthread.h>
#if defined(RT_USING_POSIX)
#include <rtdevice.h> /* for wqueue_init */
#endif

#ifdef RT_USING_DEVICE

/**
 * @addtogroup Device
 */

/**@{*/

#ifdef RT_USING_DEVICE_OPS
#define device_init     (dev->ops->init)
#define device_open     (dev->ops->open)
#define device_close    (dev->ops->close)
#define device_read     (dev->ops->read)
#define device_write    (dev->ops->write)
#define device_control  (dev->ops->control)
#else
#define device_init     (dev->init)
#define device_open     (dev->open)
#define device_close    (dev->close)
#define device_read     (dev->read)
#define device_write    (dev->write)
#define device_control  (dev->control)
#endif

/**
 * @brief ע���豸
 *
 * �˺���ע�����ָ�����Ƶ��豸������
 *
 * @param dev ָ���豸�������ƿ��ָ��
 * @param name �豸����
 * @param flags �豸�Ĺ��ܱ�־λ
 *
 * @return ʧ�ܷ��ش�����룬ע��ɹ��򷵻� RT_EOK ��
 */
rt_err_t rt_device_register(rt_device_t dev,
                            const char *name,
                            rt_uint16_t flags)
{
    if (dev == RT_NULL)
        return -RT_ERROR;

    if (rt_device_find(name) != RT_NULL)
        return -RT_ERROR;

    rt_object_init(&(dev->parent), RT_Object_Class_Device, name);
    dev->flag = flags;
    dev->ref_count = 0;
    dev->open_flag = 0;

#if defined(RT_USING_POSIX)
    dev->fops = RT_NULL;
    rt_wqueue_init(&(dev->wait_queue));
#endif

    return RT_EOK;
}
RTM_EXPORT(rt_device_register);

/**
 * @brief �豸ɾ��
 *
 * �ú�����ɾ����ע����豸��������
 *
 * @param dev ָ���豸�������ƿ��ָ��
 *
 * @return ʧ�ܷ��ش�����룬ע��ɹ��򷵻� RT_EOK ��
 */
rt_err_t rt_device_unregister(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    rt_object_detach(&(dev->parent));

    return RT_EOK;
}
RTM_EXPORT(rt_device_unregister);

/*
 * @brief �����豸��ʼ��
 *
 * �˺�����ʼ��������ע����豸��������
 *
 * @return ʧ�ܷ��ش�����룬ע��ɹ��򷵻� RT_EOK ��
 *
 * @deprecated ��1.2.x��ʼ���˹��ܽ�����ʹ�ã���Ϊ��Ӧ�ó�����豸ʱ��ִ���豸��ʼ����
 */
rt_err_t rt_device_init_all(void)
{
    return RT_EOK;
}

/**
 * @brief �豸���Һ���
 *
 * �ú���ͨ��ָ�������Ʋ����豸
 *
 * @param name �豸������
 *
 * @return �ɹ��򷵻���ע����豸���ƿ�, ʧ���򷵻� RT_NULL ��
 */
rt_device_t rt_device_find(const char *name)
{
    struct rt_object *object;
    struct rt_list_node *node;
    struct rt_object_information *information;

    /* enter critical */
    if (rt_thread_self() != RT_NULL)
        rt_enter_critical();

    /* try to find device object */
    information = rt_object_get_information(RT_Object_Class_Device);
    RT_ASSERT(information != RT_NULL);
    for (node  = information->object_list.next;
         node != &(information->object_list);
         node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (rt_strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            if (rt_thread_self() != RT_NULL)
                rt_exit_critical();

            return (rt_device_t)object;
        }
    }

    /* leave critical */
    if (rt_thread_self() != RT_NULL)
        rt_exit_critical();

    /* not found */
    return RT_NULL;
}
RTM_EXPORT(rt_device_find);

#ifdef RT_USING_HEAP
/**
 * @brief �豸����
 *
 * �˺��������û����ݴ�С����һ���豸����
 *
 * @param type ���豸���������
 * @param attach_size �û����ݵĴ�С
 *
 * @return �ɹ��������豸��������ʧ���򷵻�RT_NULL
 */
rt_device_t rt_device_create(int type, int attach_size)
{
    int size;
    rt_device_t device;

    size = RT_ALIGN(sizeof(struct rt_device), RT_ALIGN_SIZE);
    attach_size = RT_ALIGN(attach_size, RT_ALIGN_SIZE);
    /* use the totoal size */
    size += attach_size;

    device = (rt_device_t)rt_malloc(size);
    if (device)
    {
        rt_memset(device, 0x0, sizeof(struct rt_device));
        device->type = (enum rt_device_class_type)type;
    }

    return device;
}
RTM_EXPORT(rt_device_create);

/**
 * @brief �豸���ٺ���
 *
 * �ú���������ָ�����豸����
 *
 * @param device ָ�����豸����
 */
void rt_device_destroy(rt_device_t device)
{
    /* unregister device firstly */
    rt_device_unregister(device);

    /* release this device object */
    rt_free(device);
}
RTM_EXPORT(rt_device_destroy);
#endif

/**
 * @brief �豸��ʼ��
 *
 * �ú�������ʼ��ָ�����豸
 *
 * @param dev ָ���豸�����ṹ���ָ��
 *
 * @return the result
 */
rt_err_t rt_device_init(rt_device_t dev)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    /* get device init handler */
    if (device_init != RT_NULL)
    {
        if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
        {
            result = device_init(dev);
            if (result != RT_EOK)
            {
                rt_kprintf("To initialize device:%s failed. The error code is %d\n",
                           dev->parent.name, result);
            }
            else
            {
                dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return result;
}

/**
 * @brief ���豸
 *
 * This function will open a device
 *
 * @param dev ָ���豸�������ƿ�
 * @param oflag �豸�Ĵ򿪹��ܱ�־λ
 *
 * @return the result
 */
rt_err_t rt_device_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    /* if device is not initialized, initialize it. */
    if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
    {
        if (device_init != RT_NULL)
        {
            result = device_init(dev);
            if (result != RT_EOK)
            {
                rt_kprintf("To initialize device:%s failed. The error code is %d\n",
                           dev->parent.name, result);

                return result;
            }
        }

        dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
    }

    /* device is a stand alone device and opened */
    if ((dev->flag & RT_DEVICE_FLAG_STANDALONE) &&
        (dev->open_flag & RT_DEVICE_OFLAG_OPEN))
    {
        return -RT_EBUSY;
    }

    /* call device open interface */
    if (device_open != RT_NULL)
    {
        result = device_open(dev, oflag);
    }
    else
    {
        /* set open flag */
        dev->open_flag = (oflag & RT_DEVICE_OFLAG_MASK);
    }

    /* set open flag */
    if (result == RT_EOK || result == -RT_ENOSYS)
    {
        dev->open_flag |= RT_DEVICE_OFLAG_OPEN;

        dev->ref_count++;
        /* don't let bad things happen silently. If you are bitten by this assert,
         * please set the ref_count to a bigger type. */
        RT_ASSERT(dev->ref_count != 0);
    }

    return result;
}
RTM_EXPORT(rt_device_open);

/**
 * @brief �ر��豸
 *
 * �ú������ر�ָ�����豸
 *
 * @param dev ָ���豸�������ƿ�
 *
 * @return the result
 */
rt_err_t rt_device_close(rt_device_t dev)
{
    rt_err_t result = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    if (dev->ref_count == 0)
        return -RT_ERROR;

    dev->ref_count--;

    if (dev->ref_count != 0)
        return RT_EOK;

    /* call device close interface */
    if (device_close != RT_NULL)
    {
        result = device_close(dev);
    }

    /* set open flag */
    if (result == RT_EOK || result == -RT_ENOSYS)
        dev->open_flag = RT_DEVICE_OFLAG_CLOSE;

    return result;
}
RTM_EXPORT(rt_device_close);

/**
 * @brief �豸��ȡ
 *
 * �ú��������豸��ȡ����
 *
 * @param dev ָ���豸�������ƿ�
 * @param pos ��ȡ��ƫ����
 * @param buffer ���ڱ����ȡ���ݵ����ݻ�����
 * @param size �������Ĵ�С
 *
 * @return �ɹ�ʱ����ʵ�ʶ�ȡ�Ĵ�С�����򷵻ظ�����
 *
 * @note ��0.4.0��ʼ��size / pos������ǿ��豸�Ŀ顣
 */
rt_size_t rt_device_read(rt_device_t dev,
                         rt_off_t    pos,
                         void       *buffer,
                         rt_size_t   size)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ref_count == 0)
    {
        rt_set_errno(-RT_ERROR);
        return 0;
    }

    /* call device read interface */
    if (device_read != RT_NULL)
    {
        return device_read(dev, pos, buffer, size);
    }

    /* set error code */
    rt_set_errno(-RT_ENOSYS);

    return 0;
}
RTM_EXPORT(rt_device_read);

/**
 * @brief �豸д��
 *
 * �ú��������豸д������
 *
 * @param dev ָ���豸�������ƿ�
 * @param pos д���ƫ����
 * @param buffer Ҫд���豸�����ݻ�����
* @param size ���ͻ������Ĵ�С
 *
 * @return �ɹ�ʱ����ʵ��д�����ݵĴ�С�����򷵻ظ�����
 *
 * @note ��0.4.0��ʼ��size / pos������ǿ��豸�Ŀ顣
 */
rt_size_t rt_device_write(rt_device_t dev,
                          rt_off_t    pos,
                          const void *buffer,
                          rt_size_t   size)
{
    RT_ASSERT(dev != RT_NULL);

    if (dev->ref_count == 0)
    {
        rt_set_errno(-RT_ERROR);
        return 0;
    }

    /* call device write interface */
    if (device_write != RT_NULL)
    {
        return device_write(dev, pos, buffer, size);
    }

    /* set error code */
    rt_set_errno(-RT_ENOSYS);

    return 0;
}
RTM_EXPORT(rt_device_write);

/**
 * @brief �豸����
 *
 * �ú��������豸��ִ�и��ֿ��ƹ��ܡ�
 *
 * @param dev ָ���豸�������ƿ�
* @param cmd ���͸��豸������
 * @param arg ����������صĲ���
 *
 * @return the result
 */
rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)
{
    RT_ASSERT(dev != RT_NULL);

    /* call device write interface */
    if (device_control != RT_NULL)
    {
        return device_control(dev, cmd, arg);
    }

    return -RT_ENOSYS;
}
RTM_EXPORT(rt_device_control);

/**
 * @brief �����豸���ջص�����
 *
 * �˺��������ý���ָʾ�ص������� �����豸��������ʱ�������ô˻ص�������
 *
 * @param dev ָ���豸�������ƿ�
 * @param rx_ind ָʾ�ص�����
 *
 * @return RT_EOK
 */
rt_err_t
rt_device_set_rx_indicate(rt_device_t dev,
                          rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size))
{
    RT_ASSERT(dev != RT_NULL);

    dev->rx_indicate = rx_ind;

    return RT_EOK;
}
RTM_EXPORT(rt_device_set_rx_indicate);

/**
 * @brief ���ô�����ɻص�����
 *
 * �˺��������÷������ָʾ�ص������� ����ʵ�ʵ������豸���д������ʱ�������ô˻ص�������
 *
 * @param dev ָ���豸�������ƿ�
 * @param tx_done ָʾ�ص�����
 *
 * @return RT_EOK
 */
rt_err_t
rt_device_set_tx_complete(rt_device_t dev,
                          rt_err_t (*tx_done)(rt_device_t dev, void *buffer))
{
    RT_ASSERT(dev != RT_NULL);

    dev->tx_complete = tx_done;

    return RT_EOK;
}
RTM_EXPORT(rt_device_set_tx_complete);


/**@}*/
#endif
