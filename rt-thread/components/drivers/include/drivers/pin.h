/*
 * File      : pin.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
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
 * 2015-01-20     Bernard      the first version
 * 2017-10-20      ZYH          add mode open drain and input pull down
 */

#ifndef PIN_H__
#define PIN_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* pin device and operations for RT-Thread */
struct rt_device_pin
{
    struct rt_device parent;
    const struct rt_pin_ops *ops;
};

#define PIN_LOW                 0x00
#define PIN_HIGH                0x01

#define PIN_MODE_OUTPUT         0x00
#define PIN_MODE_INPUT          0x01
#define PIN_MODE_INPUT_PULLUP   0x02
#define PIN_MODE_INPUT_PULLDOWN 0x03
#define PIN_MODE_OUTPUT_OD      0x04

#define PIN_IRQ_MODE_RISING             0x00
#define PIN_IRQ_MODE_FALLING            0x01
#define PIN_IRQ_MODE_RISING_FALLING     0x02
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03
#define PIN_IRQ_MODE_LOW_LEVEL          0x04

#define PIN_IRQ_DISABLE                 0x00
#define PIN_IRQ_ENABLE                  0x01

#define PIN_IRQ_PIN_NONE                -1

struct rt_device_pin_mode
{
    rt_uint16_t pin;
    rt_uint16_t mode;
};
struct rt_device_pin_status
{
    rt_uint16_t pin;
    rt_uint16_t status;
};
struct rt_pin_irq_hdr
{
    rt_int16_t        pin;
    rt_uint16_t       mode;
    void (*hdr)(void *args);
    void             *args;
};
struct rt_pin_ops
{
    void (*pin_mode)(struct rt_device *device, rt_base_t pin, rt_base_t mode);
    void (*pin_write)(struct rt_device *device, rt_base_t pin, rt_base_t value);
    int (*pin_read)(struct rt_device *device, rt_base_t pin);

    /* TODO: add GPIO interrupt */
    rt_err_t (*pin_attach_irq)(struct rt_device *device, rt_int32_t pin,
                      rt_uint32_t mode, void (*hdr)(void *args), void *args);
    rt_err_t (*pin_detach_irq)(struct rt_device *device, rt_int32_t pin);
    rt_err_t (*pin_irq_enable)(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled);
};


/**
 * @addtogroup ringbuffer
 */

/**@{*/

int rt_device_pin_register(const char *name, const struct rt_pin_ops *ops, void *user_data);

/**
 * @brief 管脚工作模式设置
 *
 * 此函数可以设定指定管脚的工作模式。
 *
 * @param pin 管脚编号
 * @param mode 管脚工作模式，可以去以下 5 种模式之一
 * 参数              | 说明
 * ------------------|------------------------------------
 * PIN_MODE_OUTPUT       |      输出，具体模式看 drv_gpio.c 源码实现
 * PIN_MODE_INPUT       |       输入
 * PIN_MODE_INPUT_PULLUP    |   上拉输入
 * PIN_MODE_INPUT_PULLDOWN   |  下拉输入
 * PIN_MODE_OUTPUT_OD    |      开漏输出
 *
 */
void rt_pin_mode(rt_base_t pin, rt_base_t mode);

/**
 * @brief 设置管脚电平
 *
 * 此函数可以设定指定管脚的输出电平。
 *
 * @param pin 管脚编号
 * @param value 电平逻辑值，可取 2 种值之一，PIN_LOW 低电平，PIN_HIGH 高电平
 */
void rt_pin_write(rt_base_t pin, rt_base_t value);

/**
 * @brief 读管脚电平
 *
 * 该函数可以读取指定管脚的当前电平。
 *
 * @param pin 管脚编号
 *
 * @return PIN_LOW 低电平；PIN_HIGH 高电平
 */
int  rt_pin_read(rt_base_t pin);

/**
 * @brief 绑定管脚中断
 *
 * 此函数可以设定指定管脚的中断出发模式，并可指定中断回调函数。
 *
 * @param pin 管脚编号
 * @param mode 中断触发模式，可取以下 5 种值之一
 *  参数              | 描述
 * ------------------|------------------------------------
 * PIN_IRQ_MODE_RISING          |   上升沿触发
 * PIN_IRQ_MODE_FALLING         |   下降沿触发
 * PIN_IRQ_MODE_RISING_FALLING  |   边沿触发（上升沿和下降沿都触发）
 * PIN_IRQ_MODE_HIGH_LEVEL		 |	 高电平触发
 * PIN_IRQ_MODE_LOW_LEVEL 		 |	 低电平触发
 *
 * @param hdr 中断回调函数，用户需要自行定义这个函数，其返回值为 void
 * @param args 中断回调函数的参数，不需要时设置为 RT_NULL
 *
 * @return RT_EOK 成功；RT_ENOSYS 无系统；RT_EBUSY 忙
 */
rt_err_t rt_pin_attach_irq(rt_int32_t pin, rt_uint32_t mode,
                             void (*hdr)(void *args), void  *args);

/**
 * @brief 解绑管脚中断
 *
 * 调用此函数可以解绑指定的管脚中断。
 *
 * @param pin 管脚编号
 *
 * @return RT_EOK 成功；RT_ENOSYS 出错
 */
rt_err_t rt_pin_detach_irq(rt_int32_t pin);

/**
 * @brief 失能 / 屏蔽管脚中断
 *
 * 此函数可以使能/屏蔽指定管脚的中断。
 *
 * @param pin 管脚编号
 * @param enabled 状态，可取 2 种值之一：PIN_IRQ_ENABLE  开启，PIN_IRQ_DISABLE  关闭
 *
 * @return RT_EOK 成功；RT_ENOSYS 出错
 */
rt_err_t rt_pin_irq_enable(rt_base_t pin, rt_uint32_t enabled);


/**@}*/

#ifdef __cplusplus
}
#endif

#endif
