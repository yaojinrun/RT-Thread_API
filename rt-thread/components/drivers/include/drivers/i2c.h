/*
 * File      : i2c.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
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
 * Date           Author        Notes
 * 2012-04-25     weety         first version
 */

#ifndef __I2C_H__
#define __I2C_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RT_I2C_WR                0x0000
#define RT_I2C_RD               (1u << 0)
#define RT_I2C_ADDR_10BIT       (1u << 2)  /* this is a ten bit chip address */
#define RT_I2C_NO_START         (1u << 4)
#define RT_I2C_IGNORE_NACK      (1u << 5)
#define RT_I2C_NO_READ_ACK      (1u << 6)  /* when I2C reading, we do not ACK */

struct rt_i2c_msg
{
    rt_uint16_t addr;
    rt_uint16_t flags;
    rt_uint16_t len;
    rt_uint8_t  *buf;
};

struct rt_i2c_bus_device;

struct rt_i2c_bus_device_ops
{
    rt_size_t (*master_xfer)(struct rt_i2c_bus_device *bus,
                             struct rt_i2c_msg msgs[],
                             rt_uint32_t num);
    rt_size_t (*slave_xfer)(struct rt_i2c_bus_device *bus,
                            struct rt_i2c_msg msgs[],
                            rt_uint32_t num);
    rt_err_t (*i2c_bus_control)(struct rt_i2c_bus_device *bus,
                                rt_uint32_t,
                                rt_uint32_t);
};

/*for i2c bus driver*/
struct rt_i2c_bus_device
{
    struct rt_device parent;
    const struct rt_i2c_bus_device_ops *ops;
    rt_uint16_t  flags;
    rt_uint16_t  addr;
    struct rt_mutex lock;
    rt_uint32_t  timeout;
    rt_uint32_t  retries;
    void *priv;
};

#ifdef RT_I2C_DEBUG
#define i2c_dbg(fmt, ...)   rt_kprintf(fmt, ##__VA_ARGS__)
#else
#define i2c_dbg(fmt, ...)
#endif

/**
 * @addtogroup i2c
 */

/**@{*/


/**
 * @brief I2C 总线注册
 *
 * 本函数用于向系统中注册I2C 总线。
 *
 * @param bus I2C 总线句柄
 * @param bus_name I2C 总线名称，一般与硬件控制器名称一致，如：”i2c0”
 *
 * @return RT_EOK 成功
 */
rt_err_t rt_i2c_bus_device_register(struct rt_i2c_bus_device *bus,
                                    const char               *bus_name);

/**
 * @brief I2C 总线设备查找
 *
 * 调用该函数可以用名称在系统中查找指定的总线设备。
 *
 * @param bus_name I2C 总线名称
 *
 * @return I2C 设备总线存在则返回 I2C 设备句柄；未找到则返回 RT_NULL。
 */
struct rt_i2c_bus_device *rt_i2c_bus_device_find(const char *bus_name);

/**
 * @brief I2C 数据传输
 *
 * 该函数传递的消息是链式结构，可以通过消息链，实现调用一次完成多次数据的收发，主机调用此函数可以完成读或者写从设备的操作。
 *
 * @param bus I2C 总线设备句柄
 * @param msgs[] I2C 消息数组
 * @param num I2C 消息数组的数量
 *
 * @return 传输成功则返回成功传输的消息数组的个数；失败则返回 RT_ERROR。
 */
rt_size_t rt_i2c_transfer(struct rt_i2c_bus_device *bus,
                          struct rt_i2c_msg         msgs[],
                          rt_uint32_t               num);

/**
 * @brief I2C 主机模式发送
 *
 * 此函是对rt_i2c_transfer()的简单封装。主机调用此函数可以完成写从设备的操作。
 *
 * @param bus I2C 总线设备句柄
 * @param addr 从机地址，不包含读写位
 * @param flags 标志，读写标志默认为写。只支持 10 位地址选择 RT_I2C_ADDR_10BIT
 * @param buf 指向发送数据的指针
 * @param count 发送数据字节数
 *
 * @return 传输成功则返回成功发送的数据字节数；失败则返回 RT_ERROR。
 */
rt_size_t rt_i2c_master_send(struct rt_i2c_bus_device *bus,
                             rt_uint16_t               addr,
                             rt_uint16_t               flags,
                             const rt_uint8_t         *buf,
                             rt_uint32_t               count);

/**
 * @brief I2C 主机模式接收
 *
 * 此函数是对rt_i2c_transfer()的简单封装，主机调用此函数可以完成读从设备的操作。
 *
 * @param bus I2C 总线设备句柄
 * @param addr 从机地址，不包含读写位
 * @param flags 标志，读写标志默认为写。只支持 10 位地址选择 RT_I2C_ADDR_10BIT
 * @param buf 接受数据指针
 * @param count 接收数据字节数
 *
 * @return 传输成功则返回成功接收的数据字节数。
 */
rt_size_t rt_i2c_master_recv(struct rt_i2c_bus_device *bus,
                             rt_uint16_t               addr,
                             rt_uint16_t               flags,
                             rt_uint8_t               *buf,
                             rt_uint32_t               count);
int rt_i2c_core_init(void);


/**@}*/

#ifdef __cplusplus
}
#endif

#endif
