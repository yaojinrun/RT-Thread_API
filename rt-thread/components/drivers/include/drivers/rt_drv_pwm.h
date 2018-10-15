/*
 * File      : rt_drv_pwm.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2018, RT-Thread Development Team
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
 * 2018-05-07     aozima       the first version
 */

#ifndef __DRV_PWM_H_INCLUDE__
#define __DRV_PWM_H_INCLUDE__

#define PWM_CMD_ENABLE      (128 + 0)
#define PWM_CMD_DISABLE     (128 + 1)
#define PWM_CMD_SET         (128 + 2)
#define PWM_CMD_GET         (128 + 3)

struct rt_pwm_configuration
{
    rt_uint32_t channel; /* 0-n */
    rt_uint32_t period;  /* unit:ns 1ns~4.29s:1Ghz~0.23hz */
    rt_uint32_t pulse;   /* unit:ns (pulse<=period) */
};

struct rt_device_pwm;
struct rt_pwm_ops
{
    rt_err_t (*control)(struct rt_device_pwm *device, int cmd, void *arg);
};

struct rt_device_pwm
{
    struct rt_device parent;
    const struct rt_pwm_ops *ops;
};

/**
 * @addtogroup pwm
 */

/**@{*/

/*
 * @brief PWM设备注册
 *
 * 调用此函数可以解绑指定的管脚中断。
 *
 * @param pin 管脚编号
 * @param name 设备名称
 * @param ops PWM设备的私有操作集
 * @param user_data PWM设备的私有数据
 *
 * @return 注册结果
 */
extern rt_err_t rt_device_pwm_register(struct rt_device_pwm *device, const char *name, const struct rt_pwm_ops *ops, const void *user_data);

/**
 * @brief 打开指定的PWM通道
 *
 * 调用此函数可以使能指定的PWM通道
 *
 * @param channel 指定的PWM通道
 *
 * @return -RT_EIO 未找到pwm设备，RT_OK 打开成功
 */
rt_err_t rt_pwm_enable(int channel);

/**
 * @brief 设置PWM参数
 *
 * 调用此函数可以使能指定的PWM通道
 *
 * @param channel 指定的PWM通道
 * @param period 周期
 * @param pulse 占空比
 *
 * @return -RT_EIO 未找到pwm设备，RT_OK 设置成功
 */
rt_err_t rt_pwm_set(int channel, rt_uint32_t period, rt_uint32_t pulse);

/**@}*/

#endif /* __DRV_PWM_H_INCLUDE__ */
