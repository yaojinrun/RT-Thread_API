/*
 * File      : alarm.h
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
 * Date           Author            Notes
 * 2012-10-27     heyuanjie87       first version.
 */

#ifndef __ALARM_H__
#define __ALARM_H__

#include <time.h>

/**
 * @addtogroup alarm
 */

/**@{*/

#define RT_ALARM_TM_NOW        -1   /**< @brief �������ӵ�ʱ�����Ϊ���ڵ�ʱ��*/

/**< @brief �������ͺ궨�� */
#define RT_ALARM_ONESHOT       0x000 /**< @brief ��������*/
#define RT_ALARM_DAILY         0x100 /**< @brief ÿ������ */
#define RT_ALARM_WEEKLY        0x200 /**< @brief ÿ������ */
#define RT_ALARM_MONTHLY       0x400 /**< @brief ÿ������ */
#define RT_ALARM_YAERLY        0x800 /**< @brief ÿ������ */

/* alarm control cmd */
#define RT_ALARM_CTRL_MODIFY       1 /**< @brief �޸�����ʱ���������� */

/**
 * @brief ��������ָ�붨��
 */
typedef struct rt_alarm *rt_alarm_t;
/**
 * @brief ���Ӷ�ʱ�ص�����ָ�붨��
 */
typedef void (*rt_alarm_callback_t)(rt_alarm_t alarm, time_t timestamp);

/* used for low level RTC driver */
/**
 * @brief ����ʱ������ṹ��
 */
struct rt_rtc_wkalarm
{
    rt_bool_t  enable;               /**< @brief ʹ�ܱ�־λ */
    rt_int32_t tm_sec;               /**< @brief �� */
    rt_int32_t tm_min;               /**< @brief �� */
    rt_int32_t tm_hour;              /**< @brief ʱ */
};

/**
 * @brief ���ӹ���ṹ��
 */
struct rt_alarm
{
    rt_list_t list;					/**< @brief �������� */
    rt_uint32_t flag;				/**< @brief �������� */
    rt_alarm_callback_t callback;	/**< @brief ���ӵĶ�ʱ�ص����� */
    struct tm wktime;				/**< @brief ���ӵ�ʱ����� */
};

/**
 * @brief �������ýṹ��
 */
struct rt_alarm_setup
{
    rt_uint32_t flag;                /**< @brief ���ӵĲ��� */
    struct tm wktime;                /**< @brief ���ӵĶ�ʱʱ�� */
};

/**
 * @brief ���ӹ��������ṹ��
 */
struct rt_alarm_container
{
    rt_list_t head;				/**< @brief �������� */
    struct rt_mutex mutex;		/**< @brief ������ */
    struct rt_event event;		/**< @brief �����¼� */
    struct rt_alarm *current;	/**< @brief ��ǰ���ӵĹ�����ƿ� */
};



/**
 * @brief ��������
 *
 * ���ô˺������Ը����趨�Ĳ�������һ�����ӣ���ָ�����ӵĻص�������
 *
 * @param callback ���ӻص�����
 * @param setup ���ӵ����ò���
 *
 * @return �ɹ��򷵻����Ӿ����ʧ���򷵻�RT_NULL��
 */
rt_alarm_t rt_alarm_create(rt_alarm_callback_t    callback,
                           struct rt_alarm_setup *setup);

/**
 * @brief �޸���������
 *
 * @param alarm ���ӿ��ƿ�
 * @param cmd ���ӵ���������
 * @param arg ����������صĲ���
 *
 * @return �����
 */
rt_err_t rt_alarm_control(rt_alarm_t alarm, int cmd, void *arg);

/**
 * @brief ���������¼�
 *
 * @param dev ָ�������豸��ָ�루��ǰδʹ�ã����Ժ��ԣ�
 * @param event RTC�¼�����ǰδʹ�ã�
 */
void rt_alarm_update(rt_device_t dev, rt_uint32_t event);

/**
 * @brief ɾ������
 *
 * ���ô˺�������ɾ��ָ�������Ӳ��ͷ���ռ�õ�ϵͳ�ڴ�ռ䡣
 *
 * @param alarm ���Ӿ��
 *
 * @return �����
 */
rt_err_t rt_alarm_delete(rt_alarm_t alarm);

/**
 * @brief ��������
 *
 * @param alarm ���ӿ��ƿ�
 *
 * @return �����
 */
rt_err_t rt_alarm_start(rt_alarm_t alarm);

/**
 * @brief ֹͣ����
 *
 * @param alarm ���ӿ��ƿ�
 *
 * @return RT_EOK �ɹ�������� ʧ�ܡ�
 */
rt_err_t rt_alarm_stop(rt_alarm_t alarm);

/**
 * @brief ��ʼ�����ӷ���ϵͳ
 *
 */
void rt_alarm_system_init(void);


/**@}*/

#endif /* __ALARM_H__ */
