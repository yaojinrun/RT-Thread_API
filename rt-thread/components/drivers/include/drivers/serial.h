/*
 * File      : serial.h
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
 * 2012-05-15     lgnq         first version.
 * 2012-05-28     bernard      change interfaces
 * 2013-02-20     bernard      use RT_SERIAL_RB_BUFSZ to define
 *                             the size of ring buffer.
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <rtthread.h>

/**
 * @addtogroup UART
 */

/**@{*/


#define BAUD_RATE_2400                  2400
#define BAUD_RATE_4800                  4800
#define BAUD_RATE_9600                  9600
#define BAUD_RATE_19200                 19200
#define BAUD_RATE_38400                 38400
#define BAUD_RATE_57600                 57600
#define BAUD_RATE_115200                115200
#define BAUD_RATE_230400                230400
#define BAUD_RATE_460800                460800
#define BAUD_RATE_921600                921600
#define BAUD_RATE_2000000               2000000
#define BAUD_RATE_3000000               3000000

#define DATA_BITS_5                     5
#define DATA_BITS_6                     6
#define DATA_BITS_7                     7
#define DATA_BITS_8                     8
#define DATA_BITS_9                     9

#define STOP_BITS_1                     0
#define STOP_BITS_2                     1
#define STOP_BITS_3                     2
#define STOP_BITS_4                     3

#ifdef _WIN32
#include <windows.h>
#else
#define PARITY_NONE                     0
#define PARITY_ODD                      1
#define PARITY_EVEN                     2
#endif

#define BIT_ORDER_LSB                   0
#define BIT_ORDER_MSB                   1

#define NRZ_NORMAL                      0       /* Non Return to Zero : normal mode */
#define NRZ_INVERTED                    1       /* Non Return to Zero : inverted mode */

#ifndef RT_SERIAL_RB_BUFSZ
#define RT_SERIAL_RB_BUFSZ              64
#endif

#define RT_SERIAL_EVENT_RX_IND          0x01    /**< @brief 接收 */
#define RT_SERIAL_EVENT_TX_DONE         0x02    /**< @brief 发送完成 */
#define RT_SERIAL_EVENT_RX_DMADONE      0x03    /**< @brief DMA接收完成 */
#define RT_SERIAL_EVENT_TX_DMADONE      0x04    /**< @brief DMA发送完成 */
#define RT_SERIAL_EVENT_RX_TIMEOUT      0x05    /**< @brief 接收超时 */

#define RT_SERIAL_DMA_RX                0x01
#define RT_SERIAL_DMA_TX                0x02

#define RT_SERIAL_RX_INT                0x01
#define RT_SERIAL_TX_INT                0x02

#define RT_SERIAL_ERR_OVERRUN           0x01
#define RT_SERIAL_ERR_FRAMING           0x02
#define RT_SERIAL_ERR_PARITY            0x03

#define RT_SERIAL_TX_DATAQUEUE_SIZE     2048
#define RT_SERIAL_TX_DATAQUEUE_LWM      30

/**
 * @brief 串口设备默认的配置参数
 */
#define RT_SERIAL_CONFIG_DEFAULT           \
{                                          \
    BAUD_RATE_115200, /* 115200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_RB_BUFSZ, /* Buffer size */  \
    0                                      \
}

/**
 * @brief 串口配置参数结构体定义
 */

struct serial_configure
{
    rt_uint32_t baud_rate;						/**< @brief 波特率 */

    rt_uint32_t data_bits               :4;		/**< @brief 数据位 */
    rt_uint32_t stop_bits               :2;		/**< @brief 停止位 */
    rt_uint32_t parity                  :2;		/**< @brief 奇偶校验 */
    rt_uint32_t bit_order               :1;		/**< @brief 位顺序 */
    rt_uint32_t invert                  :1;		/**< @brief 模式 */
    rt_uint32_t bufsz                   :16;	/**< @brief 接收数据缓冲区大小 */
    rt_uint32_t reserved                :4;		/**< @brief 保留 */
};

/*
 * Serial FIFO mode 
 */
struct rt_serial_rx_fifo
{
    /* software fifo */
    rt_uint8_t *buffer;

    rt_uint16_t put_index, get_index;

    rt_bool_t is_full;
};

struct rt_serial_tx_fifo
{
    struct rt_completion completion;
};

/* 
 * Serial DMA mode
 */
struct rt_serial_rx_dma
{
    rt_bool_t activated;
};

struct rt_serial_tx_dma
{
    rt_bool_t activated;
    struct rt_data_queue data_queue;
};

/**
 * @brief 串口设备结构体定义
 */

struct rt_serial_device
{
    struct rt_device          parent;	/**< @brief 继承 rt_device */

    const struct rt_uart_ops *ops;		/**< @brief 串口设备的操作方法 */
    struct serial_configure   config;	/**< @brief 串口设备的配置参数 */

    void *serial_rx;
    void *serial_tx;
};
typedef struct rt_serial_device rt_serial_t;

/**
 * @brief 串口设备的操作方法
 */
struct rt_uart_ops
{
    rt_err_t (*configure)(struct rt_serial_device *serial, struct serial_configure *cfg);	 /**< @brief 串口配置操作函数 */
    rt_err_t (*control)(struct rt_serial_device *serial, int cmd, void *arg);		 		/**< @brief 串口控制函数 */

    int (*putc)(struct rt_serial_device *serial, char c);									 /**< @brief 发送一个字符数据 */
    int (*getc)(struct rt_serial_device *serial);											 /**< @brief 接收一个字符数据 */

    rt_size_t (*dma_transmit)(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction);		 /**< @brief DMA模式数据传输函数 */
};

/**
 * @brief 串口中断事件
 *
 * 调用此函数可以根据指定的事件执行对应的回调函数。
 *
 * @param serial 串口设备句柄
 * @param event 事件标志
 */

void rt_hw_serial_isr(struct rt_serial_device *serial, int event);

/**
 * @brief 注册串口设备
 *
 * 调用此函数可以注册串口设备到串口驱动框架。
 *
 * @param serial 串口设备句柄
 * @param name 串口设备名称
 * @param flag 串口设备模式标志
 * @param data 串口设备私有数据
 *
 * @return RT_EOK 成功；-RT_ERROR 注册失败，已有其他驱动使用该name注册。
 */

rt_err_t rt_hw_serial_register(struct rt_serial_device *serial,
                               const char              *name,
                               rt_uint32_t              flag,
                               void                    *data);


/**@}*/

#endif
