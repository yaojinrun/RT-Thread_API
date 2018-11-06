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


/**
 * @addtogroup pwm
 */

/**@{*/

/**
 * @brief PWM设备参数配置结构体
 */
struct rt_pwm_configuration
{
    rt_uint32_t channel; /**< @brief 通道 */
    rt_uint32_t period;  /**< @brief 周期 */
    rt_uint32_t pulse;   /**< @brief 占空比 */
};

struct rt_device_pwm;
/**
 * @brief PWM设备的操作方法
 */
struct rt_pwm_ops
{
    rt_err_t (*control)(struct rt_device_pwm *device, int cmd, void *arg);	/**< @brief PWM配置更改函数 */
};

struct rt_device_pwm
{
    struct rt_device parent;			/**< @brief 继承自 rt_device */
    const struct rt_pwm_ops *ops;		/**< @brief PWM设备的操作方法 */
};


/**
 * @brief 注册PWM设备
 *
 * 调用此函数可以注册PWM设备到系统。
 *
 * @param device PWM设备句柄
 * @param name 设备名称
 * @param ops PWM设备的操作方法
 * @param user_data PWM设备的私有数据
 *
 * @return RT_EOK 注册成功；-RT_ERROR	注册失败，已有其他驱动使用该name注册。
 */
rt_err_t rt_device_pwm_register(struct rt_device_pwm *device, const char *name, const struct rt_pwm_ops *ops, const void *user_data);

/**
 * @brief 打开PWM通道
 *
 * 调用此函数可以使能指定的PWM通道
 *
 * @param device PWM设备句柄
 * @param channel 指定的PWM通道
 *
 * @return RT_OK 成功；-RT_EIO 未找到pwm设备。
 */
rt_err_t rt_pwm_enable(struct rt_device_pwm *device, int channel);

/**
 * @brief 关闭PWM通道
 *
 * 调用此函数可以关闭指定的PWM通道
 *
 * @param device PWM设备句柄
 * @param channel 指定的PWM通道
 *
 * @return RT_OK 打开成功；-RT_EIO 未找到pwm设备。
 */
rt_err_t rt_pwm_disable(struct rt_device_pwm *device, int channel);

/**
 * @brief 设置PWM参数
 *
 * 该函数可以设定指定PWM通道的周期和占空参数。
 *
 * @param device PWM设备句柄
 * @param channel 指定的PWM通道
 * @param period 周期
 * @param pulse 占空比
 *
 * @return RT_OK 设置成功；-RT_EIO 未找到pwm设备。
 */
rt_err_t rt_pwm_set(struct rt_device_pwm *device, int channel, rt_uint32_t period, rt_uint32_t pulse);

/**@}*/

#endif /* __DRV_PWM_H_INCLUDE__ */
