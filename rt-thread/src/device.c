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
 * @brief 注册设备
 *
 * 此函数注册具有指定名称的设备。设备需要注册到I/O设备管理器中，应用程序才能够访问。
 *
 * @param dev 设备句柄
 * @param name 设备名称
 * @param flags 设备模式标志位
 *
 * @return 注册成功返回 RT_EOK，失败则返回-RT_ERROR。
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
 * @brief 删除设备
 *
 * 该函数将删除已注册的设备。当设备注销后的，设备将从设备管理器中移除，
 * 也就不能再通过设备查找搜索到该设备。注销设备不会释放设备控制块占用的内存。
 *
 * @param dev 设备句柄
 *
 * @return 注册成功返回 RT_EOK。
 */
rt_err_t rt_device_unregister(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    rt_object_detach(&(dev->parent));

    return RT_EOK;
}
RTM_EXPORT(rt_device_unregister);

/*
 * @brief 所有设备初始化
 *
 * 此函数初始化所有已注册的设备驱动程序
 *
 * @return 失败返回错误代码，注册成功则返回 RT_EOK 。
 *
 * @deprecated 从1.2.x开始，此功能将不再使用，因为在应用程序打开设备时会执行设备初始化。
 */
rt_err_t rt_device_init_all(void)
{
    return RT_EOK;
}

/**
 * @brief 查找设备
 *
 * 该函数通过指定的名称查找设备。应用程序根据设备名称获取设备句柄，进而可以操作设备。
 *
 * @param name 设备的名称
 *
 * @return 成功则返回已注册的设备句柄, 失败则返回 RT_NULL 。
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
 * @brief 创建设备
 *
 * 此函数根据用户指定的设备类型动态的创建一个设备对象。
 * 调用该接口时，系统会从动态堆内存中分配一个设备控制块，
 * 大小为struct rt_device和attach_size的和，设备的类型由参数type设定。
 *
 * @param type 设备类型
 * @param attach_size 用户数据的大小
 *
 * @return 成功则返回设备句柄，失败则返回RT_NULL。
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
 * @brief 销毁设备
 *
 * 该函数将销毁动态创建的设备对象。
 *
 * @param device 设备句柄
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
 * @brief 初始化设备
 *
 * 该函数将初始化指定的设备。当一个设备已经初始化成功后，调用这个接口将不再重复做初始化。
 *
 * @param dev 设备句柄
 *
 * @return 成功返回RT_EOK；失败返回错误码。
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
 * @brief 打开设备
 *
 * 通过设备句柄，应用程序可以打开和关闭设备，
 * 打开设备时会检测设备是否已经初始化，没有初始化则会默认调用初始化接口初始化设备。
 *
 * @param dev 设备句柄
 * @param oflag 设备的打开模式标志位
 *
 * @return 成功返回RT_EOK；失败则返回错误码。
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
 * @brief 关闭设备
 *
 * 该函数将关闭指定的设备。
 *
 * @param dev 设备句柄
 *
 * @return 成功返回RT_EOK；失败则返回错误码。
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
 * @brief 读取设备
 *
 * 该函数将从设备读取数据。
 *
 * @param dev 设备句柄
 * @param pos 读取的偏移量
 * @param buffer 用于保存读取数据的数据缓冲区
 * @param size 缓冲区的大小
 *
 * @return 成功返回实际读取的大小，如果是字符设备，返回大小以字节为单位，如果是块设备，返回的大小以块为单位；失败则返回0。
 *
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
 * @brief 写设备
 *
 * 该函数将向设备写入数据。
 *
 * @param dev 设备句柄
 * @param pos 写入的偏移量
 * @param buffer 要写入设备的数据缓冲区
* @param size 写入数据的大小
 *
 * @return 成功返回实际写入数据的大小，如果是字符设备，返回大小以字节为单位；如果是块设备，返回的大小以块为单位；失败则返回0。
 *
 * @note 从0.4.0开始，size / pos代表的是块设备的块。
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
 * @brief 控制设备
 *
 * 该函数将在设备上执行各种控制功能。
 *
 * @param dev 设备句柄
 * @param cmd 命令控制字，这个参数通常与设备驱动程序相关。
 * @param arg 控制命令相关的参数
 *
 * @return 成功返回RT_EOK；失败则返回错误码。
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
 * @brief 设置设备接收回调函数
 *
 * 此函数将设置设备接收回调函数。该函数的回调函数由调用者提供。
 * 当硬件设备接收到数据时，会回调这个函数并把收到的数据长度放在size参数中传递给上层应用。
 * 上层应用线程应在收到指示后，立刻从设备中读取数据。
 *
 * @param dev 设备句柄
 * @param rx_ind 回调函数
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
 * @brief 设置设备发送完成回调函数
 *
 * 此函数将设置发送完成回调函数。调用这个函数时，回调函数由调用者提供，当硬件设备发送完数据时，
 * 由驱动程序回调这个函数并把发送完成的数据块地址buffer作为参数传递给上层应用。
 * 上层应用（线程）在收到指示时会根据发送buffer的情况，释放buffer内存块或将其作为下一个写数据的缓存。
 *
 * @param dev 设备句柄
 * @param tx_done 指示回调函数
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
