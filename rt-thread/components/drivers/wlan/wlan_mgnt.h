/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-06     tyx          the first version
 */

#ifndef __WLAN_MGNT_H__
#define __WLAN_MGNT_H__

#include <wlan_dev.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WLAN
 */
/*@{*/

#ifndef RT_WLAN_SCAN_WAIT_MS
#define RT_WLAN_SCAN_WAIT_MS       (10 * 1000)
#endif

#ifndef RT_WLAN_SCAN_CACHE_NUM
#define RT_WLAN_SCAN_CACHE_NUM     (50)
#endif

#ifndef RT_WLAN_CONNECT_WAIT_MS
#define RT_WLAN_CONNECT_WAIT_MS    (10 * 1000)
#endif

#ifndef RT_WLAN_START_AP_WAIT_MS
#define RT_WLAN_START_AP_WAIT_MS    (10 * 1000)
#endif

#ifndef RT_WLAN_EBOX_NUM
#define RT_WLAN_EBOX_NUM           (10)
#endif

/*state fot station*/
#define RT_WLAN_STATE_CONNECT     (1UL << 0)
#define RT_WLAN_STATE_CONNECTING  (1UL << 1)
#define RT_WLAN_STATE_READY       (1UL << 2)
#define RT_WLAN_STATE_POWERSAVE   (1UL << 3)

/*flags fot station*/
#define RT_WLAN_STATE_AUTOEN      (1UL << 0)

/*state fot ap*/
#define RT_WLAN_STATE_ACTIVE      (1UL << 0)

typedef enum
{
    RT_WLAN_EVT_READY = 0,              /* connect and prot is ok, You can send data*/
    RT_WLAN_EVT_SCAN_DONE,              /* Scan a info */
    RT_WLAN_EVT_SCAN_REPORT,            /* Scan end */
    RT_WLAN_EVT_STA_CONNECTED,          /* connect success */
    RT_WLAN_EVT_STA_CONNECTED_FAIL,     /* connection failed */
    RT_WLAN_EVT_STA_DISCONNECTED,       /* disconnect */
    RT_WLAN_EVT_AP_START,               /* AP start */
    RT_WLAN_EVT_AP_STOP,                /* AP stop */
    RT_WLAN_EVT_AP_ASSOCIATED,          /* sta associated */
    RT_WLAN_EVT_AP_DISASSOCIATED,       /* sta disassociated */
    RT_WLAN_EVT_MAX
} rt_wlan_event_t;

typedef void (*rt_wlan_event_handler)(int event, struct rt_wlan_buff *buff, void *parameter);

struct rt_wlan_scan_result
{
    rt_int32_t num;
    struct rt_wlan_info *info;
};

/*
 * wifi init interface
 */
int rt_wlan_init(void);
/**
 * @brief 注册WLAN设备到WLAN设备框架
 *
 * @param dev_name WLAN设备名
 * @param mode WLAN设备工作模式
 *
 * @return 
 * | 返回值     | 描述       |
 * | ---------- | ---------- |
 * | -RT_EINVAL | 参数错误   |
 * | -RT_EIO    | 设备未找到 |
 * | -RT_ERROR  | 执行失败   |
 * | RT_EOK     | 执行成功   |
 */
rt_err_t rt_wlan_set_mode(const char *dev_name, rt_wlan_mode_t mode);
rt_wlan_mode_t rt_wlan_get_mode(const char *dev_name);

/*
 * wifi station mode interface
 */
/**
 * @brief 同步连接热点
 *
 * @param ssid WIFI名字
 * @param password WIFI密码
 *
 * @return 
 * | 返回值     | 描述       |
 * | ---------- | ---------- |
 * | -RT_EINVAL | 参数错误   |
 * | -RT_EIO    | 未注册设备 |
 * | -RT_ERROR  | 连接失败   |
 * | RT_EOK     | 连接成功   |
 */
rt_err_t rt_wlan_connect(const char *ssid, const char *password);
rt_err_t rt_wlan_connect_adv(struct rt_wlan_info *info, const char *password);
/**
 * @brief 同步断开热点
 *
 * @return
 * | 返回值     | 描述       |
 * | ---------- | ---------- |
 * | -RT_EIO    | 未注册设备 |
 * | -RT_ENOMEM | 内存不足   |
 * | -RT_ERROR  | 断开失败   |
 * | RT_EOK     | 断开成功   |
 */
rt_err_t rt_wlan_disconnect(void);
rt_bool_t rt_wlan_is_connected(void);
rt_bool_t rt_wlan_is_ready(void);
rt_err_t rt_wlan_set_mac(rt_uint8_t *mac);
rt_err_t rt_wlan_get_mac(rt_uint8_t *mac);
rt_err_t rt_wlan_get_info(struct rt_wlan_info *info);
int rt_wlan_get_rssi(void);

/*
 * wifi ap mode interface
 */
rt_err_t rt_wlan_start_ap(const char *ssid, const char *password);
rt_err_t rt_wlan_start_ap_adv(struct rt_wlan_info *info, const char *password);
int rt_wlan_ap_is_active(void);
rt_err_t rt_wlan_ap_stop(void);
rt_err_t rt_wlan_ap_get_info(struct rt_wlan_info *info);
int rt_wlan_ap_get_sta_num(void);
int rt_wlan_ap_get_sta_info(struct rt_wlan_info *info, int num);
rt_err_t rt_wlan_ap_deauth_sta(rt_uint8_t *mac);
rt_err_t rt_wlan_ap_set_country(rt_country_code_t country_code);
rt_country_code_t rt_wlan_ap_get_country(void);

/*
 * wifi scan interface
 */
rt_err_t rt_wlan_scan(void);
int rt_wlan_init(void);
/**
 * @brief 同步扫描热点
 *
 * @return 扫描结果
 */
struct rt_wlan_scan_result *rt_wlan_scan_sync(void);
struct rt_wlan_scan_result *rt_wlan_scan_with_info(struct rt_wlan_info *info);
int rt_wlan_scan_get_info_num(void);
int rt_wlan_scan_get_info(struct rt_wlan_info *info, int num);
struct rt_wlan_scan_result *rt_wlan_scan_get_result(void);
void rt_wlan_scan_result_clean(void);
int rt_wlan_scan_find_cache(struct rt_wlan_info *info, struct rt_wlan_info *out_info, int num);
rt_bool_t rt_wlan_find_best_by_cache(const char *ssid, struct rt_wlan_info *info);

/*
 * wifi auto connect interface
 */
/**
 * @brief 配置自动重连模式
 *
 * @param enable enanle/disable自动重连
 */
void rt_wlan_config_autoreconnect(rt_bool_t enable);
rt_bool_t rt_wlan_get_autoreconnect_mode(void);

/*
 * wifi power management interface
 */
rt_err_t rt_wlan_set_powersave(int level);
int rt_wlan_get_powersave(void);

/*
 * wifi event management interface
 */
rt_err_t rt_wlan_register_event_handler(rt_wlan_event_t event, rt_wlan_event_handler handler, void *parameter);
rt_err_t rt_wlan_unregister_event_handler(rt_wlan_event_t event);

/*
 * wifi management lock interface
 */
void rt_wlan_mgnt_lock(void);
void rt_wlan_mgnt_unlock(void);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
