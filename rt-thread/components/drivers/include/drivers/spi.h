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

#define RT_SPI_CPHA     (1<<0)                             /**< @brief bit[0]:CPHA, ʱ�Ӽ��� */
#define RT_SPI_CPOL     (1<<1)                             /**< @brief bit[1]:CPOL, ʱ����λ */
/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low��high transition)
 *    and data are propagated on a falling edge (high��low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */
#define RT_SPI_LSB      (0<<2)                             /**< @brief bit[2]: LSBλ��ǰ */
#define RT_SPI_MSB      (1<<2)                             /**< @brief bit[2]: MSBλ��ǰ */

#define RT_SPI_MASTER   (0<<3)                             /**< @brief SPI �����豸 */
#define RT_SPI_SLAVE    (1<<3)                             /**< @brief SPI �����豸 */

#define RT_SPI_MODE_0       (0 | 0)                        /**< @brief CPOL = 0, CPHA = 0 */
#define RT_SPI_MODE_1       (0 | RT_SPI_CPHA)              /**< @brief CPOL = 0, CPHA = 1 */
#define RT_SPI_MODE_2       (RT_SPI_CPOL | 0)              /**< @brief CPOL = 1, CPHA = 0 */
#define RT_SPI_MODE_3       (RT_SPI_CPOL | RT_SPI_CPHA)    /**< @brief CPOL = 1, CPHA = 1 */

#define RT_SPI_MODE_MASK    (RT_SPI_CPHA | RT_SPI_CPOL | RT_SPI_MSB)

#define RT_SPI_CS_HIGH  (1<<4)                             /**< @brief Ƭѡ�ߵ�ƽ��Ч */
#define RT_SPI_NO_CS    (1<<5)                             /**< @brief ��ʹ��Ƭѡ */
#define RT_SPI_3WIRE    (1<<6)                             /**< @brief MOSI��MISO����һ�������� */
#define RT_SPI_READY    (1<<7)                             /**< @brief ���豸������ͣ */

/**
 * @brief SPI��Ϣ�ṹ�嶨��
 */
struct rt_spi_message
{
    const void *send_buf;				/**< @brief SPI���ͻ�����ָ�� */
    void *recv_buf;						/**< @brief SPI���ջ�����ָ�� */
    rt_size_t length;					/**< @brief ���ݳ��� */
    struct rt_spi_message *next;		/**< @brief ָ����һ��SPI��Ϣ��ָ�� */

    unsigned cs_take    : 1;			/**< @brief SPI���߻�ȡ��־λ */
    unsigned cs_release : 1;			/**< @brief SPI�����ͷű�־λ */
};

/**
 * @brief SPI���ò����ṹ�嶨��
 */
struct rt_spi_configuration
{
    rt_uint8_t mode;					/**< @brief SPI�Ĺ���ģʽ */
    rt_uint8_t data_width;				/**< @brief SPI�����ݿ�� */
    rt_uint16_t reserved;				/**< @brief ����λ */

    rt_uint32_t max_hz;					/**< @brief SPI���ߵ������Ƶ�� */
};

struct rt_spi_ops;
/**
 * @brief SPI���߽ṹ�嶨��
 */
struct rt_spi_bus
{
    struct rt_device parent;			/**< @brief �̳� rt_device */
    const struct rt_spi_ops *ops;		/**< @brief SPI���ߵĲ������� */

    struct rt_mutex lock;				/**< @brief SPI���ߵĻ����� */
    struct rt_spi_device *owner;		/**< @brief SPI���ߵĵ�ǰʹ���� */
};

/**
 * @brief SPI���ߵĲ�������
 */
struct rt_spi_ops
{
    rt_err_t (*configure)(struct rt_spi_device *device, struct rt_spi_configuration *configuration);	/**< @brief SPI�豸�����ú��� */
    rt_uint32_t (*xfer)(struct rt_spi_device *device, struct rt_spi_message *message);					/**< @brief SPI�豸����Ϣ���ͺ��� */
};

/**
 * @brief SPI���豸
 */
struct rt_spi_device
{
    struct rt_device parent;				/**< @brief �̳� rt_device*/
    struct rt_spi_bus *bus;					/**< @brief ָ����ص�SPI�����豸 */

    struct rt_spi_configuration config;		/**< @brief SPI���ò��� */
};
#define SPI_DEVICE(dev) ((struct rt_spi_device *)(dev))


/**
 * @brief ע��SPI �����豸
 *
 * ���ô˺���������ϵͳ��ע�� SPI �����豸��
 *
 * @param bus SPI �����豸���
 * @param name SPI �����豸���ƣ�һ����Ӳ������������һ�£��磺��spi0��
 * @param ops SPI �����豸�Ĳ����������� SPI ������ʵ��
 *
 * @return RT_EOK �ɹ���-RT_ERROR ע��ʧ�ܣ�������������ʹ�ø�nameע�ᡣ
 */
rt_err_t rt_spi_bus_register(struct rt_spi_bus       *bus,
                             const char              *name,
                             const struct rt_spi_ops *ops);


/**
 * @brief ���� SPI ���豸��SPI�����豸��
 *
 * �˺������ڹ���һ��SPI�豸�ڵ㵽ָ����SPI���ߣ����ں�ע��SPI�豸�ڵ㣬����user_data���浽SPI�豸�ڵ��user_dataָ���
 *
 * @param device SPI ���豸���
 * @param name SPI ���豸���ƣ�SPI���豸����ԭ��Ϊspixy����spi10��ʾ����������spi1�ϵ�0���豸
 * @param bus_name SPI �����豸����
 * @param user_data �û�����ָ�룬һ��ΪSPI���豸��Ӧ��CS������Ϣ���������ݴ���ʱSPI����������������Ž���Ƭѡ
 *
 * @return RT_EOK �ɹ���-RT_ERROR ������������ʹ��name���ػ���bus_name��Ӧ��SPI�����豸�����ڡ�
 */
rt_err_t rt_spi_bus_attach_device(struct rt_spi_device *device,
                                  const char           *name,
                                  const char           *bus_name,
                                  void                 *user_data);

/**
 * @brief ��ȡ SPI �����豸
 *
 * �û����ô˺�������ȡSPI�����豸��������SPI�����豸�Ĺ���ģʽ�Ͳ�����
 *
 * @param device SPI ���豸���
 *
 * @return �ɹ��򷵻�RT_EOK�������򷵻ش����롣
 */
rt_err_t rt_spi_take_bus(struct rt_spi_device *device);


/**
 * @brief �ͷ� SPI �����豸
 *
 * �û����Ե��ô˺������ͷ�SPI�����豸��
 *
 * @param device SPI ���豸���
 *
 * @return �ɹ��򷵻�RT_EOK�������򷵻ش����롣
 */
rt_err_t rt_spi_release_bus(struct rt_spi_device *device);


/**
 * @brief ��ȡƬѡ
 *
 * ���ô˺�������ƬѡSPI�豸��
 *
 * @param device SPI ���豸���
 *
 * @return 0 ��ȡ�ɹ���Ƭѡ��ʼ��
 */
rt_err_t rt_spi_take(struct rt_spi_device *device);

/**
 * @brief �ͷ�Ƭѡ
 *
 * ���ô˺��������ͷű�ƬѡSPI�豸��
 *
 * @param device SPI ���豸���
 *
 * @return 0 �ͷųɹ���Ƭѡ������
 */
rt_err_t rt_spi_release(struct rt_spi_device *device);

/**
 * @brief SPI ���豸����
 *
 * ��������������SPI ���豸��ʱ�ӡ����ݿ�ȵ�Ҫ��ͨ����Ҫ����SPIģʽ��Ƶ�ʲ�����
 *
 * @param device SPI ���豸���
 * @param cfg SPI ���ò���ָ��
 *
 * @return RT_EOK ���óɹ���
 */
rt_err_t rt_spi_configure(struct rt_spi_device        *device,
                          struct rt_spi_configuration *cfg);

/**
 * @brief �ȷ��ͺ��������
 *
 * �������ʺϴ�SPI�����ж�ȡһ�����ݣ��������л��ȷ���һЩ���ݣ�������͵�ַ����Ȼ���ٽ���ָ�����ȵ����ݡ��˺�����ͬ�ڵ���rt_spi_transfer_message()����2����Ϣ��
 *
 * @param device SPI ���豸���
 * @param send_buf ���ͻ���������ָ��
 * @param send_length ���ͻ����������ֽ���
 * @param recv_buf ���ջ���������ָ�룬spi ��ȫ˫���ģ�֧��ͬʱ�շ�
 * @param recv_length ���ջ����������ֽ���
 *
 * @return RT_EOK �ɹ���-RT_EIO ʧ�ܡ�
 */
rt_err_t rt_spi_send_then_recv(struct rt_spi_device *device,
                               const void           *send_buf,
                               rt_size_t             send_length,
                               void                 *recv_buf,
                               rt_size_t             recv_length);

/**
 * @brief ����������������
 *
 * �����Ҫ�Ⱥ���������2�������������ݣ������м�Ƭѡ���ͷţ����Ե��ô˺�����
 * �������ʺ���SPI���豸��д��һ�����ݣ���һ���ȷ�������͵�ַ�����ݣ��ڶ����ٷ���ָ�����ȵ����ݡ�
 *
 * @param device SPI ���豸���
 * @param send_buf1 ���ͻ�����1����ָ��
 * @param send_length1 ���ͻ�����1�����ֽ���
 * @param send_buf2 ���ͻ�����2����ָ��
 * @param send_length2 ���ͻ�����2�����ֽ���
 *
 * @return RT_EOK �ɹ���-RT_EIO ʧ�ܡ�
 */
rt_err_t rt_spi_send_then_send(struct rt_spi_device *device,
                               const void           *send_buf1,
                               rt_size_t             send_length1,
                               const void           *send_buf2,
                               rt_size_t             send_length2);

/**
 * @brief ����һ������
 *
 * �˺������Դ��䴫��һ�����ݡ��˺�����ͬ�ڵ���rt_spi_transfer_message()����һ����Ϣ����ʼ��������ʱƬѡѡ�У���������ʱ�ͷ�Ƭѡ��
 *
 * @param device SPI ���豸���
 * @param send_buf ���ͻ�����ָ��
 * @param recv_buf ���ջ�����ָ��
 * @param length ���� / ���� �����ֽ���
 *
 * @return ʵ�ʴ�����ֽ���
 */
rt_size_t rt_spi_transfer(struct rt_spi_device *device,
                          const void           *send_buf,
                          void                 *recv_buf,
                          rt_size_t             length);

/**
 * @brief �����Ϣ��������
 *
 * �˺������Դ���һ������Ϣ���û����Ժ���������message�ṹ�����������ֵ���Ӷ����Ժܷ���Ŀ������ݴ��䷽ʽ��
 *
 * @param device SPI ���豸���
 * @param message ��Ϣָ��
 *
 * @return RT_NULL ��Ϣ�б��ͳɹ�������ʧ�ܷ���ָ��ʣ��δ���͵���Ϣ��ָ�롣
 */
struct rt_spi_message *rt_spi_transfer_message(struct rt_spi_device  *device,
                                               struct rt_spi_message *message);

/**
 * @brief ��������
 *
 * ���ô˺����������ݲ����浽recv_bufָ��Ļ��������˺����Ƕ�rt_spi_transfer()�����ķ�װ��
 * SPIЭ������ֻ����MASTER��������ʱ�ӣ���ˣ��ڽ�������ʱ���ᷢ��dummy��
 *
 * @param device SPI ���豸���
 * @param recv_buf ���ջ�����ָ��
 * @param length �������ݵ��ֽ���
 *
 * @return ʵ�ʽ��յ��ֽ�����
 */
rt_inline rt_size_t rt_spi_recv(struct rt_spi_device *device,
                                void                 *recv_buf,
                                rt_size_t             length)
{
    return rt_spi_transfer(device, RT_NULL, recv_buf, length);
}

/**
 * @brief ��������
 *
 * ���ô˺�������send_bufָ��Ļ����������ݣ����Խ��յ������ݡ�
 * �˺����Ƕ�rt_spi_transfer()�����ķ�װ����ʼ��������ʱƬѡѡ�У���������ʱ�ͷ�Ƭѡ��
 *
 * @param device SPI ���豸���
 * @param send_buf ���ͻ�����ָ��
 * @param length �������ݵ��ֽ���
 *
 * @return ʵ�ʷ��͵��ֽ�����
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
 * @brief ����һ����Ϣ
 *
 * ʹ��rt_spi_transfer_message()������Ϣʱ��
 * ���д��������Ϣ�����Ե����������ʽ���������ģ���ʹ�����º�������Ϣ����������һ���µĴ�������Ϣ��
 *
 * @param list ��Ϣ����ָ��
 * @param message ��Ϣָ��
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
