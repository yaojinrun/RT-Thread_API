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

#define RT_ALARM_TM_NOW        -1   /**< @brief 设置闹钟的时间参数为现在的时间*/

/**< @brief 闹钟类型宏定义 */
#define RT_ALARM_ONESHOT       0x000 /**< @brief 单次闹钟*/
#define RT_ALARM_DAILY         0x100 /**< @brief 每天闹钟 */
#define RT_ALARM_WEEKLY        0x200 /**< @brief 每周闹钟 */
#define RT_ALARM_MONTHLY       0x400 /**< @brief 每月闹钟 */
#define RT_ALARM_YAERLY        0x800 /**< @brief 每年闹钟 */

/* alarm control cmd */
#define RT_ALARM_CTRL_MODIFY       1 /**< @brief 修改闹钟时间或参数命令 */

/**
 * @brief 闹钟类型指针定义
 */
typedef struct rt_alarm *rt_alarm_t;
/**
 * @brief 闹钟定时回调函数指针定义
 */
typedef void (*rt_alarm_callback_t)(rt_alarm_t alarm, time_t timestamp);

/* used for low level RTC driver */
/**
 * @brief 闹钟时间参数结构体
 */
struct rt_rtc_wkalarm
{
    rt_bool_t  enable;               /**< @brief 使能标志位 */
    rt_int32_t tm_sec;               /**< @brief 秒 */
    rt_int32_t tm_min;               /**< @brief 分 */
    rt_int32_t tm_hour;              /**< @brief 时 */
};

/**
 * @brief 闹钟管理结构体
 */
struct rt_alarm
{
    rt_list_t list;					/**< @brief 闹钟链表 */
    rt_uint32_t flag;				/**< @brief 闹钟类型 */
    rt_alarm_callback_t callback;	/**< @brief 闹钟的定时回调函数 */
    struct tm wktime;				/**< @brief 闹钟的时间参数 */
};

/**
 * @brief 闹钟设置结构体
 */
struct rt_alarm_setup
{
    rt_uint32_t flag;                /**< @brief 闹钟的参数 */
    struct tm wktime;                /**< @brief 闹钟的定时时间 */
};

/**
 * @brief 闹钟管理容器结构体
 */
struct rt_alarm_container
{
    rt_list_t head;				/**< @brief 闹钟链表 */
    struct rt_mutex mutex;		/**< @brief 互斥量 */
    struct rt_event event;		/**< @brief 闹钟事件 */
    struct rt_alarm *current;	/**< @brief 当前闹钟的管理控制块 */
};



/**
 * @brief 创建闹钟
 *
 * 调用此函数可以根据设定的参数创建一个闹钟，并指定闹钟的回调函数。
 *
 * @param callback 闹钟回调函数
 * @param setup 闹钟的设置参数
 *
 * @return 成功则返回闹钟句柄，失败则返回RT_NULL。
 */
rt_alarm_t rt_alarm_create(rt_alarm_callback_t    callback,
                           struct rt_alarm_setup *setup);

/**
 * @brief 修改闹钟设置
 *
 * @param alarm 闹钟控制块
 * @param cmd 闹钟的设置命令
 * @param arg 设置命令相关的参数
 *
 * @return 错误号
 */
rt_err_t rt_alarm_control(rt_alarm_t alarm, int cmd, void *arg);

/**
 * @brief 发送闹钟事件
 *
 * @param dev 指向闹钟设备的指针（当前未使用，可以忽略）
 * @param event RTC事件（当前未使用）
 */
void rt_alarm_update(rt_device_t dev, rt_uint32_t event);

/**
 * @brief 删除闹钟
 *
 * 调用此函数可以删除指定的闹钟并释放其占用的系统内存空间。
 *
 * @param alarm 闹钟句柄
 *
 * @return 错误号
 */
rt_err_t rt_alarm_delete(rt_alarm_t alarm);

/**
 * @brief 启动闹钟
 *
 * @param alarm 闹钟控制块
 *
 * @return 错误号
 */
rt_err_t rt_alarm_start(rt_alarm_t alarm);

/**
 * @brief 停止闹钟
 *
 * @param alarm 闹钟控制块
 *
 * @return RT_EOK 成功；错误号 失败。
 */
rt_err_t rt_alarm_stop(rt_alarm_t alarm);

/**
 * @brief 初始化闹钟服务系统
 *
 */
void rt_alarm_system_init(void);


/**@}*/

#endif /* __ALARM_H__ */
