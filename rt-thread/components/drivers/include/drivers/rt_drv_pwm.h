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
 * @brief PWM�豸�������ýṹ��
 */
struct rt_pwm_configuration
{
    rt_uint32_t channel; /**< @brief ͨ�� */
    rt_uint32_t period;  /**< @brief ���� */
    rt_uint32_t pulse;   /**< @brief ռ�ձ� */
};

struct rt_device_pwm;
/**
 * @brief PWM�豸�Ĳ�������
 */
struct rt_pwm_ops
{
    rt_err_t (*control)(struct rt_device_pwm *device, int cmd, void *arg);	/**< @brief PWM���ø��ĺ��� */
};

struct rt_device_pwm
{
    struct rt_device parent;			/**< @brief �̳��� rt_device */
    const struct rt_pwm_ops *ops;		/**< @brief PWM�豸�Ĳ������� */
};


/**
 * @brief ע��PWM�豸
 *
 * ���ô˺�������ע��PWM�豸��ϵͳ��
 *
 * @param device PWM�豸���
 * @param name �豸����
 * @param ops PWM�豸�Ĳ�������
 * @param user_data PWM�豸��˽������
 *
 * @return RT_EOK ע��ɹ���-RT_ERROR	ע��ʧ�ܣ�������������ʹ�ø�nameע�ᡣ
 */
rt_err_t rt_device_pwm_register(struct rt_device_pwm *device, const char *name, const struct rt_pwm_ops *ops, const void *user_data);

/**
 * @brief ��PWMͨ��
 *
 * ���ô˺�������ʹ��ָ����PWMͨ��
 *
 * @param device PWM�豸���
 * @param channel ָ����PWMͨ��
 *
 * @return RT_OK �ɹ���-RT_EIO δ�ҵ�pwm�豸��
 */
rt_err_t rt_pwm_enable(struct rt_device_pwm *device, int channel);

/**
 * @brief �ر�PWMͨ��
 *
 * ���ô˺������Թر�ָ����PWMͨ��
 *
 * @param device PWM�豸���
 * @param channel ָ����PWMͨ��
 *
 * @return RT_OK �򿪳ɹ���-RT_EIO δ�ҵ�pwm�豸��
 */
rt_err_t rt_pwm_disable(struct rt_device_pwm *device, int channel);

/**
 * @brief ����PWM����
 *
 * �ú��������趨ָ��PWMͨ�������ں�ռ�ղ�����
 *
 * @param device PWM�豸���
 * @param channel ָ����PWMͨ��
 * @param period ����
 * @param pulse ռ�ձ�
 *
 * @return RT_OK ���óɹ���-RT_EIO δ�ҵ�pwm�豸��
 */
rt_err_t rt_pwm_set(struct rt_device_pwm *device, int channel, rt_uint32_t period, rt_uint32_t pulse);

/**@}*/

#endif /* __DRV_PWM_H_INCLUDE__ */
