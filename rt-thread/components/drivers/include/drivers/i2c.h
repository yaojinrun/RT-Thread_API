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


#define RT_I2C_WR                0x0000		/**< @brief I2Cд��־ */
#define RT_I2C_RD               (1u << 0)	/**< @brief I2C����־ */
#define RT_I2C_ADDR_10BIT       (1u << 2)  	/**< @brief 10λ���豸��ַ */
#define RT_I2C_NO_START         (1u << 4)	/**< @brief �޿�ʼ���� */
#define RT_I2C_IGNORE_NACK      (1u << 5)	/**< @brief ����NACK */
#define RT_I2C_NO_READ_ACK      (1u << 6)  	/**< @brief ����ʱ�򲻷���ACK */

/**
 * @brief I2C��Ϣ�ṹ��
 */
struct rt_i2c_msg
{
    rt_uint16_t addr;		/**< @brief I2C�ӻ���ַ */
    rt_uint16_t flags;		/**< @brief ����д��־�� */
    rt_uint16_t len;		/**< @brief ��д�����ֽ��� */
    rt_uint8_t  *buf;		/**< @brief ��д���ݻ�����ָ�� */
};

struct rt_i2c_bus_device;

/**
 * @brief I2C�����豸�Ĳ�������
 */
struct rt_i2c_bus_device_ops
{
    rt_size_t (*master_xfer)(struct rt_i2c_bus_device *bus,
                             struct rt_i2c_msg msgs[],
                             rt_uint32_t num);				/**< @brief I2C��Ϊ����ģʽ�����ݴ���ӿ� */
    rt_size_t (*slave_xfer)(struct rt_i2c_bus_device *bus,
                            struct rt_i2c_msg msgs[],
                            rt_uint32_t num);				/**< @brief I2C��Ϊ�ӻ�ģʽ�����ݴ���ӿ� */
    rt_err_t (*i2c_bus_control)(struct rt_i2c_bus_device *bus,
                                rt_uint32_t,
                                rt_uint32_t);				/**< @brief I2C���߿��ƽӿ� */
};

/*for i2c bus driver*/
/**
 * @brief I2C�����豸�ṹ��
 */
struct rt_i2c_bus_device
{
    struct rt_device parent;					/**< @brief �̳��� rt_device */
    const struct rt_i2c_bus_device_ops *ops;	/**< @brief I2C�豸�Ĳ������� */
    rt_uint16_t  flags;							/**< @brief I2C�豸���� */
    rt_uint16_t  addr;							/**< @brief I2C�豸��ַ */
    struct rt_mutex lock;						/**< @brief ������*/
    rt_uint32_t  timeout;						/**< @brief I2C�豸��ȡ�ȴ�ʱ�� */
    rt_uint32_t  retries;						/**< @brief I2C�豸��ȡ���� */
    void *priv;									/**< @brief I2C�豸��˽������ָ�� */
};

#ifdef RT_I2C_DEBUG
#define i2c_dbg(fmt, ...)   rt_kprintf(fmt, ##__VA_ARGS__)
#else
#define i2c_dbg(fmt, ...)
#endif


/**
 * @brief ע��I2C �����豸
 *
 * ������������ϵͳ��ע��I2C �����豸��
 *
 * @param bus I2C �����豸���
 * @param bus_name I2C �����豸���ƣ�һ����Ӳ������������һ�£��磺��i2c0��
 *
 * @return RT_EOK �ɹ���-RT_ERROR	ע��ʧ�ܣ�������������ʹ�ø�bus_nameע�ᡣ
 */
rt_err_t rt_i2c_bus_device_register(struct rt_i2c_bus_device *bus,
                                    const char               *bus_name);


struct rt_i2c_bus_device *rt_i2c_bus_device_find(const char *bus_name);

/**
 * @brief I2C ���ݴ���
 *
 * �ú������ݵ���Ϣ����ʽ�ṹ������ͨ����Ϣ����ʵ�ֵ���һ����ɶ�����ݵ��շ����������ô˺���������ɶ�����д���豸�Ĳ�����
 *
 * @param bus I2C �����豸���
 * @param msgs[] I2C ��Ϣ����
 * @param num I2C ��Ϣ���������
 *
 * @return ����ɹ��򷵻سɹ��������Ϣ����ĸ�����ʧ���򷵻����������롣
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
