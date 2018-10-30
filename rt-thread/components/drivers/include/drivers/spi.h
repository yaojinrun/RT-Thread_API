/*
 * File      : spi.h
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
 * Date           Author       Notes
 * 2012-11-23     Bernard      Add extern "C"
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <stdlib.h>
#include <rtthread.h>

#ifdef __cplusplus
extern "C"{
#endif

#define RT_SPI_CPHA     (1<<0)                             /* bit[0]:CPHA, clock phase */
#define RT_SPI_CPOL     (1<<1)                             /* bit[1]:CPOL, clock polarity */
/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low→high transition)
 *    and data are propagated on a falling edge (high→low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */
#define RT_SPI_LSB      (0<<2)                             /* bit[2]: 0-LSB */
#define RT_SPI_MSB      (1<<2)                             /* bit[2]: 1-MSB */

#define RT_SPI_MASTER   (0<<3)                             /* SPI master device */
#define RT_SPI_SLAVE    (1<<3)                             /* SPI slave device */

#define RT_SPI_MODE_0       (0 | 0)                        /* CPOL = 0, CPHA = 0 */
#define RT_SPI_MODE_1       (0 | RT_SPI_CPHA)              /* CPOL = 0, CPHA = 1 */
#define RT_SPI_MODE_2       (RT_SPI_CPOL | 0)              /* CPOL = 1, CPHA = 0 */
#define RT_SPI_MODE_3       (RT_SPI_CPOL | RT_SPI_CPHA)    /* CPOL = 1, CPHA = 1 */

#define RT_SPI_MODE_MASK    (RT_SPI_CPHA | RT_SPI_CPOL | RT_SPI_MSB)

#define RT_SPI_CS_HIGH  (1<<4)                             /* Chipselect active high */
#define RT_SPI_NO_CS    (1<<5)                             /* No chipselect */
#define RT_SPI_3WIRE    (1<<6)                             /* SI/SO pin shared */
#define RT_SPI_READY    (1<<7)                             /* Slave pulls low to pause */

/**
 * @brief SPI消息结构体定义
 */
struct rt_spi_message
{
    const void *send_buf;				/**< @brief SPI发送缓冲区指针 */
    void *recv_buf;						/**< @brief SPI接收缓冲区指针 */
    rt_size_t length;					/**< @brief 数据长度 */
    struct rt_spi_message *next;		/**< @brief 指向下一个SPI消息的指针 */

    unsigned cs_take    : 1;			/**< @brief SPI总线获取标志位 */
    unsigned cs_release : 1;			/**< @brief SPI总线释放标志位 */
};

/**
 * @brief SPI配置参数结构体定义
 */
struct rt_spi_configuration
{
    rt_uint8_t mode;					/**< @brief SPI的工作模式 */
    rt_uint8_t data_width;				/**< @brief SPI的数据宽度 */
    rt_uint16_t reserved;				/**< @brief 保留位 */

    rt_uint32_t max_hz;					/**< @brief SPI总线的最大工作频率 */
};

struct rt_spi_ops;
/**
 * @brief SPI总线结构体定义
 */
struct rt_spi_bus
{
    struct rt_device parent;			/**< @brief 从 rt_device 继承 */
    const struct rt_spi_ops *ops;		/**< @brief SPI总线的操作集 */

    struct rt_mutex lock;				/**< @brief SPI总线的互斥量 */
    struct rt_spi_device *owner;		/**< @brief SPI总线的当前使用者 */
};

/**
 * @brief SPI总线的操作集
 */
struct rt_spi_ops
{
    rt_err_t (*configure)(struct rt_spi_device *device, struct rt_spi_configuration *configuration);	/**< @brief SPI设备的配置函数 */
    rt_uint32_t (*xfer)(struct rt_spi_device *device, struct rt_spi_message *message);					/**< @brief SPI设备的消息传送函数 */
};

/**
 * @brief SPI虚拟总线，肯定有一个设备连接到该虚拟总线
 */
struct rt_spi_device
{
    struct rt_device parent;				/**< @brief 从 rt_device 继承 */
    struct rt_spi_bus *bus;					/**< @brief SPI总线指针 */

    struct rt_spi_configuration config;		/**< @brief SPI配置参数 */
};
#define SPI_DEVICE(dev) ((struct rt_spi_device *)(dev))


/**
 * @addtogroup spi
 */

/**@{*/


/**
 * @brief SPI 总线注册
 *
 * 调用此函数可以向系统中注册 SPI 总线。
 *
 * @param bus SPI 总线句柄
 * @param name SPI 总线名称，一般与硬件控制器名称一致，如：”spi0”
 * @param ops SPI 总线操作方法，即 SPI 驱动的实现
 *
 * @return RT_EOK 成功；-RT_ERROR bus 为空或者 name 已被注册。
 */
rt_err_t rt_spi_bus_register(struct rt_spi_bus       *bus,
                             const char              *name,
                             const struct rt_spi_ops *ops);


/**
 * @brief 挂载 SPI 设备到总线上
 *
 * 此函数用于挂载一个SPI设备节点到指定的SPI总线，并内核注册SPI设备节点，并将user_data保存到SPI设备节点的user_data指针里。
 *
 * @param device SPI 设备句柄
 * @param name SPI 设备名称
 * @param bus_name SPI 总线名称
 * @param user_data 用户数据指针
 *
 * @return RT_EOK 成功；-RT_ERROR bus_name 不存在、device 为空、name 已被注册。
 */
rt_err_t rt_spi_bus_attach_device(struct rt_spi_device *device,
                                  const char           *name,
                                  const char           *bus_name,
                                  void                 *user_data);

/**
 * @brief 获取 SPI 总线
 *
 * 用户调用此函数来获取SPI总线，并设置SPI总线的工作模式和参数。
 *
 * @param device SPI 总线设备句柄
 *
 * @return 成功则返回RT_EOK；错误则返回 错误码。
 */
rt_err_t rt_spi_take_bus(struct rt_spi_device *device);


/**
 * @brief 释放 SPI 总线
 *
 * 用户可以调用此函数来释放SPI总线。
 *
 * @param device SPI 总线设备句柄
 *
 * @return 成功则返回RT_EOK；错误则返回 错误码。
 */
rt_err_t rt_spi_release_bus(struct rt_spi_device *device);


/**
 * @brief 获取 SPI 器件
 *
 * 调用此函数可以片选SPI设备。
 *
 * @param device SPI 总线设备句柄
 *
 * @return 0 获取成功，片选开始。
 */
rt_err_t rt_spi_take(struct rt_spi_device *device);

/**
 * @brief 释放 SPI 器件
 *
 * 调用此函数可以释放被片选SPI设备。
 *
 * @param device SPI 总线设备句柄
 *
 * @return 0 释放成功，片选结束。
 */
rt_err_t rt_spi_release(struct rt_spi_device *device);

/**
 * @brief SPI 总线配置
 *
 * 本函数用于配置SPI BUS以满足不同设备的时钟、数据宽度等要求，通常需要配置SPI模式、频率参数。
 *
 * @param device SPI 设备句柄
 * @param cfg SPI 传输配置参数指针
 * @param device SPI 设备句柄
 *
 * @return RT_EOK 配置成功。
 */
rt_err_t rt_spi_configure(struct rt_spi_device        *device,
                          struct rt_spi_configuration *cfg);

/**
 * @brief SPI 发送一次数据后再接收一次数据
 *
 * 本函数适合从SPI外设中读取一块数据，本函数中会先发送一些数据（如命令和地址），然后再接收指定长度的数据。此函数等同于调用rt_spi_transfer_message()传输2条消息。
 *
 * @param device SPI 总线设备句柄
 * @param send_buf 发送缓冲区数据指针
 * @param send_length 发送缓冲区数据字节数
 * @param recv_buf 接收缓冲区数据指针，spi 是全双工的，支持同时收发
 * @param recv_length 接收缓冲区数据字节数
 *
 * @return RT_EOK 成功，-RT_EIO 失败。
 */
rt_err_t rt_spi_send_then_recv(struct rt_spi_device *device,
                               const void           *send_buf,
                               rt_size_t             send_length,
                               void                 *recv_buf,
                               rt_size_t             recv_length);

/**
 * @brief SPI 连续发送两次数据
 *
 * 本函数适合向SPI外设中写入一块数据，本函数中会先发送一些数据（如命令和地址），然后再发送指定长度的数据。此函数等同于调用rt_spi_transfer_message()传输2条消息。
 *
 * @param device SPI 总线设备句柄
 * @param send_buf1 发送缓冲区1数据指针
 * @param send_length1 发送缓冲区1数据字节数
 * @param send_buf2 发送缓冲区2数据指针
 * @param send_length2 发送缓冲区2数据字节数
 *
 * @return RT_EOK 成功，-RT_EIO 失败。
 */
rt_err_t rt_spi_send_then_send(struct rt_spi_device *device,
                               const void           *send_buf1,
                               rt_size_t             send_length1,
                               const void           *send_buf2,
                               rt_size_t             send_length2);

/**
 * @brief SPI 单个消息传输
 *
 * 调用此函数将发送一次数据，同于调用rt_spi_transfer_message()传输一条消息。
 *
 * @param device SPI 设备句柄
 * @param send_buf 发送缓冲区指针
 * @param recv_buf 接收缓冲区指针
 * @param length 发送 / 接收 数据字节数
 *
 * @return 实际传输的字节数
 */
rt_size_t rt_spi_transfer(struct rt_spi_device *device,
                          const void           *send_buf,
                          void                 *recv_buf,
                          rt_size_t             length);

/**
 * @brief SPI 多个消息连续传输
 *
 * 此函数可以传输一连串消息，用户可以很灵活的设置message结构体各参数的数值，从而可以很方便的控制数据传输方式。
 *
 * @param device SPI 设备句柄
 * @param message 消息指针
 *
 * @return RT_NULL 消息列表发送成功，发送失败则返回当前的消息指针。
 */
struct rt_spi_message *rt_spi_transfer_message(struct rt_spi_device  *device,
                                               struct rt_spi_message *message);

/**
 * @brief SPI 消息接收函数
 *
 * 调用此函数接受数据并保存到recv_buf指向的缓冲区。是对rt_spi_transfer()函数的封装。
 * SPI协议里面只能由MASTER主动产生时钟，因此，在接收数据时，会发送dummy。此函数等同于
 * 调用rt_spi_transfer_message()传输一条消息。
 *
 * @param device SPI 设备句柄
 * @param recv_buf 接收缓冲区指针
 * @param length 接受数据的字节数
 *
 * @return 实际接收的字节数。
 */
rt_inline rt_size_t rt_spi_recv(struct rt_spi_device *device,
                                void                 *recv_buf,
                                rt_size_t             length)
{
    return rt_spi_transfer(device, RT_NULL, recv_buf, length);
}

/**
 * @brief SPI 消息接收函数
 *
 * 调用此函数发送send_buf指向的缓冲区的数据，忽略接收到的数据。是对rt_spi_transfer()函数的封装。
 * 此函数等同于调用rt_spi_transfer_message()传输一条消息。
 *
 * @param device SPI 设备句柄
 * @param send_buf 发送缓冲区指针
 * @param length 发送数据的字节数
 *
 * @return 实际发送的字节数。
 */
rt_inline rt_size_t rt_spi_send(struct rt_spi_device *device,
                                const void           *send_buf,
                                rt_size_t             length)
{
    return rt_spi_transfer(device, send_buf, RT_NULL, length);
}

rt_inline rt_uint8_t rt_spi_sendrecv8(struct rt_spi_device *device,
                                      rt_uint8_t            data)
{
    rt_uint8_t value;

    rt_spi_send_then_recv(device, &data, 1, &value, 1);

    return value;
}

rt_inline rt_uint16_t rt_spi_sendrecv16(struct rt_spi_device *device,
                                        rt_uint16_t           data)
{
    rt_uint16_t value;

    rt_spi_send_then_recv(device, &data, 2, &value, 2);

    return value;
}

/**
 * @brief SPI 消息追加
 *
 * 调用此函数可以向SPI消息发送链表中增加一条消息。
 *
 * @param list 消息链表指针
 * @param message 消息指针
 */
rt_inline void rt_spi_message_append(struct rt_spi_message *list,
                                     struct rt_spi_message *message)
{
    RT_ASSERT(list != RT_NULL);
    if (message == RT_NULL)
        return; /* not append */

    while (list->next != RT_NULL)
    {
        list = list->next;
    }

    list->next = message;
    message->next = RT_NULL;
}


/**@}*/

#ifdef __cplusplus
}
#endif

#endif
