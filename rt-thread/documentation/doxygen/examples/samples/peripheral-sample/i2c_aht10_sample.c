/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-15     misonyo      first implementation.
 */
/* 
 * �����嵥������һ�� I2C �豸ʹ������
 * ���̵����� i2c_aht10_sample ��������ն�
 * ������ø�ʽ��i2c_aht10_sample i2c1
 * ������ͣ�����ڶ���������Ҫʹ�õ�I2C�����豸���ƣ�Ϊ����ʹ��Ĭ�ϵ�I2C�����豸
 * �����ܣ�ͨ�� I2C �豸��ȡ��ʪ�ȴ����� aht10 ����ʪ�����ݲ���ӡ
*/

#include <rtthread.h>
#include <rtdevice.h>

#define AHT10_I2C_BUS_NAME          "i2c1"  /* ���������ӵ�I2C�����豸���� */
#define AHT10_ADDR                  0x38
#define AHT10_CALIBRATION_CMD       0xE1    /* У׼���� */
#define AHT10_NORMAL_CMD            0xA8    /* һ������ */
#define AHT10_GET_DATA              0xAC    /* ��ȡ�������� */

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;
static rt_bool_t initialized = RT_FALSE;        /* ��������ʼ��״̬ */

/* д�������Ĵ��� */
static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;

    buf[0] = reg; //cmd
    buf[1] = data[0];
    buf[2] = data[1];

    msgs.addr = AHT10_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 3;
    
    /* ����I2C�豸�ӿڷ������� */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
        return RT_EOK;
    else
        return -RT_ERROR;
}

/* ���������Ĵ������� */
static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;

    msgs.addr = AHT10_ADDR;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;

    /* ����I2C�豸�ӿڽ������� */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
        return RT_EOK;
    else
        return -RT_ERROR;
}

static void read_temp_humi(float *cur_temp,float *cur_humi)
{
    rt_uint8_t temp[6];

    write_reg(i2c_bus, AHT10_GET_DATA, 0);      /* �������� */
    read_regs(i2c_bus, 6, temp);                /* ��ȡ���������� */

    /* ʪ������ת�� */
    *cur_humi = (temp[1] << 12 | temp[2] << 4 | (temp[3] & 0xf0) >> 4) * 100.0 / (1 << 20);
    /* �¶�����ת�� */
    *cur_temp = ((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50;
}

static void aht10_init(const char *name)
{
    rt_uint8_t temp[2] = {0, 0};

    /* ����I2C�����豸����ȡI2C�����豸��� */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(name);

    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find %s device!\n", name);
    }
    else
    {
        write_reg(i2c_bus, AHT10_NORMAL_CMD, temp);
        rt_thread_mdelay(400);

        temp[0] = 0x08;
        temp[1] = 0x00;
        write_reg(i2c_bus, AHT10_CALIBRATION_CMD, temp); 
        rt_thread_mdelay(400);
        initialized = RT_TRUE;
    }
}

static void i2c_aht10_sample(int argc,char *argv[])
{
    float humidity, temperature;
    char name[RT_NAME_MAX];

    humidity = 0.0;
    temperature = 0.0;

    if (argc == 2)
    {
        rt_strncpy(name, argv[1], RT_NAME_MAX);
    }
    else
    {
        rt_strncpy(name, AHT10_I2C_BUS_NAME, RT_NAME_MAX);
    }

    if (!initialized)
    {
        /* ��������ʼ�� */
        aht10_init(name);
    }
    if (initialized)
    {
        /* ��ȡ��ʪ������ */
        read_temp_humi(&temperature, &humidity);

        rt_kprintf("read aht10 sensor humidity   : %d.%d %%\n", (int)humidity, (int)(humidity * 10) % 10);
        rt_kprintf("read aht10 sensor temperature: %d.%d \n", (int)temperature, (int)(temperature * 10) % 10);
    }
    else
    {
        rt_kprintf("initialize sensor failed!\n");
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(i2c_aht10_sample, i2c aht10 sample);
