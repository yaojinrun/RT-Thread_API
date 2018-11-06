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


/**
 * @addtogroup i2c
 */

/**@{*/


#define RT_I2C_WR                0x0000		/**< @brief I2C写标志 */
#define RT_I2C_RD               (1u << 0)	/**< @brief I2C读标志 */
#define RT_I2C_ADDR_10BIT       (1u << 2)  	/**< @brief 10位的设备地址 */
#define RT_I2C_NO_START         (1u << 4)	/**< @brief 无开始条件 */
#define RT_I2C_IGNORE_NACK      (1u << 5)	/**< @brief 忽略NACK */
#define RT_I2C_NO_READ_ACK      (1u << 6)  	/**< @brief 读的时候不发送ACK */

/**
 * @brief I2C消息结构体
 */
struct rt_i2c_msg
{
    rt_uint16_t addr;		/**< @brief I2C从机地址 */
    rt_uint16_t flags;		/**< @brief 读、写标志等 */
    rt_uint16_t len;		/**< @brief 读写数据字节数 */
    rt_uint8_t  *buf;		/**< @brief 读写数据缓冲区指针 */
};

struct rt_i2c_bus_device;

/**
 * @brief I2C总线设备的操作方法
 */
struct rt_i2c_bus_device_ops
{
    rt_size_t (*master_xfer)(struct rt_i2c_bus_device *bus,
                             struct rt_i2c_msg msgs[],
                             rt_uint32_t num);				/**< @brief I2C作为主机模式的数据传输接口 */
    rt_size_t (*slave_xfer)(struct rt_i2c_bus_device *bus,
                            struct rt_i2c_msg msgs[],
                            rt_uint32_t num);				/**< @brief I2C作为从机模式的数据传输接口 */
    rt_err_t (*i2c_bus_control)(struct rt_i2c_bus_device *bus,
                                rt_uint32_t,
                                rt_uint32_t);				/**< @brief I2C总线控制接口 */
};

/*for i2c bus driver*/
/**
 * @brief I2C总线设备结构体
 */
struct rt_i2c_bus_device
{
    struct rt_device parent;					/**< @brief 继承自 rt_device */
    const struct rt_i2c_bus_device_ops *ops;	/**< @brief I2C设备的操作方法 */
    rt_uint16_t  flags;							/**< @brief I2C设备参数 */
    rt_uint16_t  addr;							/**< @brief I2C设备地址 */
    struct rt_mutex lock;						/**< @brief 互斥量*/
    rt_uint32_t  timeout;						/**< @brief I2C设备获取等待时间 */
    rt_uint32_t  retries;						/**< @brief I2C设备获取次数 */
    void *priv;									/**< @brief I2C设备的私有数据指针 */
};

#ifdef RT_I2C_DEBUG
#define i2c_dbg(fmt, ...)   rt_kprintf(fmt, ##__VA_ARGS__)
#else
#define i2c_dbg(fmt, ...)
#endif


/**
 * @brief 注册I2C 总线设备
 *
 * 本函数用于向系统中注册I2C 总线设备。
 *
 * @param bus I2C 总线设备句柄
 * @param bus_name I2C 总线设备名称，一般与硬件控制器名称一致，如：“i2c0”
 *
 * @return RT_EOK 成功；-RT_ERROR	注册失败，已有其他驱动使用该bus_name注册。
 */
rt_err_t rt_i2c_bus_device_register(struct rt_i2c_bus_device *bus,
                                    const char               *bus_name);


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
 * @return 传输成功则返回成功传输的消息数组的个数；失败则返回其他错误码。
 */
rt_size_t rt_i2c_transfer(struct rt_i2c_bus_device *bus,
                          struct rt_i2c_msg         msgs[],
                          rt_uint32_t               num);

rt_size_t rt_i2c_master_send(struct rt_i2c_bus_device *bus,
                             rt_uint16_t               addr,
                             rt_uint16_t               flags,
                             const rt_uint8_t         *buf,
                             rt_uint32_t               count);

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
