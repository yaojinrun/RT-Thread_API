/*
 * File      : rtc.h
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
 * 2012-10-10     aozima       first version.
 */

#ifndef __RTC_H__
#define __RTC_H__


/**
 * @addtogroup rtc
 */

/**@{*/

/**
 * @brief 管设置系统日期但不修改时间
 *
 * @param year 年
 * @param month 月(1~12).
 * @param day 日
 *
 * @return RT_EOK 成功，RT_ERROR 失败。
 *
 */
rt_err_t set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day);

/**
 * @brief 管设置系统时间但不修改日期
 *
 * @param hour  小时（0~23）
 * @param minute 分钟（0~59）
 * @param second 秒（0~59）
 *
 * @return RT_EOK 成功，RT_ERROR 失败。
 *
 */
rt_err_t set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second);

int rt_soft_rtc_init(void);
int rt_rtc_ntp_sync_init(void);

/**@}*/

#endif /* __RTC_H__ */
