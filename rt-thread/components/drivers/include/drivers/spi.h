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

/**
 * @addtogroup spi
 */

/**@{*/

#define RT_SPI_CPHA     (1<<0)                             /**< @brief bit[0]:CPHA, 时钟极性 */
#define RT_SPI_CPOL     (1<<1)                             /**< @brief bit[1]:CPOL, 时钟相位 */
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
#define RT_SPI_LSB      (0<<2)                             /**< @brief bit[2]: LSB位在前 */
#define RT_SPI_MSB      (1<<2)                             /**< @brief bit[2]: MSB位在前 */

#define RT_SPI_MASTER   (0<<3)                             /**< @brief SPI 做主设备 */
#define RT_SPI_SLAVE    (1<<3)                             /**< @brief SPI 做从设备 */

#define RT_SPI_MODE_0       (0 | 0)                        /**< @brief CPOL = 0, CPHA = 0 */
#define RT_SPI_MODE_1       (0 | RT_SPI_CPHA)              /**< @brief CPOL = 0, CPHA = 1 */
#define RT_SPI_MODE_2       (RT_SPI_CPOL | 0)              /**< @brief CPOL = 1, CPHA = 0 */
#define RT_SPI_MODE_3       (RT_SPI_CPOL | RT_SPI_CPHA)    /**< @brief CPOL = 1, CPHA = 1 */

#define RT_SPI_MODE_MASK    (RT_SPI_CPHA | RT_SPI_CPOL | RT_SPI_MSB)

#define RT_SPI_CS_HIGH  (1<<4)                             /**< @brief 片选高电平有效 */
#define RT_SPI_NO_CS    (1<<5)                             /**< @brief 不使用片选 */
#define RT_SPI_3WIRE    (1<<6)                             /**< @brief MOSI和MISO共用一根数据线 */
#define RT_SPI_READY    (1<<7)                             /**< @brief 从设备拉低暂停 */

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
    struct rt_device parent;			/**< @brief 继承 rt_device */
    const struct rt_spi_ops *ops;		/**< @brief SPI总线的操作方法 */

    struct rt_mutex lock;				/**< @brief SPI总线的互斥锁 */
    struct rt_spi_device *owner;		/**< @brief SPI总线的当前使用者 */
};

/**
 * @brief SPI总线的操作方法
 */
struct rt_spi_ops
{
    rt_err_t (*configure)(struct rt_spi_device *device, struct rt_spi_configuration *configuration);	/**< @brief SPI设备的配置函数 */
    rt_uint32_t (*xfer)(struct rt_spi_device *device, struct rt_spi_message *message);					/**< @brief SPI设备的消息传送函数 */
};

/**
 * @brief SPI从设备
 */
struct rt_spi_device
{
    struct rt_device parent;				/**< @brief 继承 rt_device*/
    struct rt_spi_bus *bus;					/**< @brief 指向挂载的SPI总线设备 */

    struct rt_spi_configuration config;		/**< @brief SPI配置参数 */
};
#define SPI_DEVICE(dev) ((struct rt_spi_device *)(dev))


/**
 * @brief 注册SPI 总线设备
 *
 * 调用此函数可以向系统中注册 SPI 总线设备。
 *
 * @param bus SPI 总线设备句柄
 * @param name SPI 总线设备名称，一般与硬件控制器名称一致，如：“spi0”
 * @param ops SPI 总线设备的操作方法，即 SPI 驱动的实现
 *
 * @return RT_EOK 成功；-RT_ERROR 注册失败，已有其他驱动使用该name注册。
 */
rt_err_t rt_spi_bus_register(struct rt_spi_bus       *bus,
                             const char              *name,
                             const struct rt_spi_ops *ops);


/**
 * @brief 挂载 SPI 从设备到SPI总线设备上
 *
 * 此函数用于挂载一个SPI设备节点到指定的SPI总线，并内核注册SPI设备节点，并将user_data保存到SPI设备节点的user_data指针里。
 *
 * @param device SPI 从设备句柄
 * @param name SPI 从设备名称，SPI从设备命名原则为spixy，如spi10表示挂载在总线spi1上的0号设备
 * @param bus_name SPI 总线设备名称
 * @param user_data 用户数据指针，一般为SPI从设备对应的CS引脚信息，进行数据传输时SPI控制器会操作此引脚进行片选
 *
 * @return RT_EOK 成功；-RT_ERROR 已有其他驱动使用name挂载或者bus_name对应的SPI总线设备不存在。
 */
rt_err_t rt_spi_bus_attach_device(struct rt_spi_device *device,
                                  const char           *name,
                                  const char           *bus_name,
                                  void                 *user_data);

/**
 * @brief 获取 SPI 总线设备
 *
 * 用户调用此函数来获取SPI总线设备，并设置SPI总线设备的工作模式和参数。
 *
 * @param device SPI 从设备句柄
 *
 * @return 成功则返回RT_EOK；错误则返回错误码。
 */
rt_err_t rt_spi_take_bus(struct rt_spi_device *device);


/**
 * @brief 释放 SPI 总线设备
 *
 * 用户可以调用此函数来释放SPI总线设备。
 *
 * @param device SPI 从设备句柄
 *
 * @return 成功则返回RT_EOK；错误则返回错误码。
 */
rt_err_t rt_spi_release_bus(struct rt_spi_device *device);


/**
 * @brief 获取片选
 *
 * 调用此函数可以片选SPI设备。
 *
 * @param device SPI 从设备句柄
 *
 * @return 0 获取成功，片选开始。
 */
rt_err_t rt_spi_take(struct rt_spi_device *device);

/**
 * @brief 释放片选
 *
 * 调用此函数可以释放被片选SPI设备。
 *
 * @param device SPI 从设备句柄
 *
 * @return 0 释放成功，片选结束。
 */
rt_err_t rt_spi_release(struct rt_spi_device *device);

/**
 * @brief SPI 从设备配置
 *
 * 本函数用于配置SPI 从设备的时钟、数据宽度等要求，通常需要配置SPI模式、频率参数。
 *
 * @param device SPI 从设备句柄
 * @param cfg SPI 配置参数指针
 *
 * @return RT_EOK 配置成功。
 */
rt_err_t rt_spi_configure(struct rt_spi_device        *device,
                          struct rt_spi_configuration *cfg);

/**
 * @brief 先发送后接收数据
 *
 * 本函数适合从SPI外设中读取一块数据，本函数中会先发送一些数据（如命令和地址），然后再接收指定长度的数据。此函数等同于调用rt_spi_transfer_message()传输2条消息。
 *
 * @param device SPI 从设备句柄
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
 * @brief 连续发送两次数据
 *
 * 如果需要先后连续发送2个缓冲区的数据，并且中间片选不释放，可以调用此函数。
 * 本函数适合向SPI从设备中写入一块数据，第一次先发送命令和地址等数据，第二次再发送指定长度的数据。
 *
 * @param device SPI 从设备句柄
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
 * @brief 传输一次数据
 *
 * 此函数可以传输传输一次数据。此函数等同于调用rt_spi_transfer_message()传输一条消息，开始发送数据时片选选中，函数返回时释放片选。
 *
 * @param device SPI 从设备句柄
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
 * @brief 多个消息连续传输
 *
 * 此函数可以传输一连串消息，用户可以很灵活的设置message结构体各参数的数值，从而可以很方便的控制数据传输方式。
 *
 * @param device SPI 从设备句柄
 * @param message 消息指针
 *
 * @return RT_NULL 消息列表发送成功，发送失败返回指向剩余未发送的消息的指针。
 */
struct rt_spi_message *rt_spi_transfer_message(struct rt_spi_device  *device,
                                               struct rt_spi_message *message);

/**
 * @brief 接收数据
 *
 * 调用此函数接受数据并保存到recv_buf指向的缓冲区。此函数是对rt_spi_transfer()函数的封装。
 * SPI协议里面只能由MASTER主动产生时钟，因此，在接收数据时，会发送dummy。
 *
 * @param device SPI 从设备句柄
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
 * @brief 发送数据
 *
 * 调用此函数发送send_buf指向的缓冲区的数据，忽略接收到的数据。
 * 此函数是对rt_spi_transfer()函数的封装，开始发送数据时片选选中，函数返回时释放片选。
 *
 * @param device SPI 从设备句柄
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
 * @brief 增加一条消息
 *
 * 使用rt_spi_transfer_message()传输消息时，
 * 所有待传输的消息都是以单向链表的形式连接起来的，可使用如下函数往消息链表里增加一条新的待传输消息。
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
