/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-25     armink       the first version
 */

#ifndef _ULOG_H_
#define _ULOG_H_

#include <rtthread.h>
#include "ulog_def.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup ulog
 */

/**@{*/

#define ULOG_VERSION_STR               "0.1.0"

/*
 * ulog init and deint
 */
/**
 * @brief ULOG 初始化
 *
 * 在使用 ulog 前必须调用该函数完成 ulog 初始化。如果开启了组件自动初始化， API 也将被自动调用。
 *
 * @return 
 *| 返回  | 描述           |
 *| ----- | -------------- |
 *| `0` | 成功           |
 *| -RT_ENOMEM    | 失败，内存不足 |
 *
 */
int ulog_init(void);

/**
 * @brief ULOG 去初始化
 *
 * 当 ulog 不再使用时，可以执行该函数释放资源。
 */
void ulog_deinit(void);

/*
 * output different level log by LOG_X API
 *
 * NOTE: The `LOG_TAG` and `LOG_LVL` must be defined before including the <ulog.h> when you want to use LOG_X API.
 *
 * #define LOG_TAG              "example"
 * #define LOG_LVL              LOG_LVL_DBG
 * #include <ulog.h>
 *
 * Then you can using LOG_X API to output log
 *
 * LOG_D("this is a debug log!");
 * LOG_E("this is a error log!");
 */

 /**
 * @brief 错误级别日志
 *
 * 'E'为LOG_LVL_ERROR的缩写，表示发生严重的、不可修复的错误时输出的日志属于错误级别日志 。
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_E(...)                     ulog_e(LOG_TAG, __VA_ARGS__)
 /**
 * @brief 警告级别日志
 *
 * 'W'为 LOG_LVL_WARNING 的缩写，表示出现一些不太重要的、具有可修复性的错误时，会输出这些警告日志。
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_W(...)                     ulog_w(LOG_TAG, __VA_ARGS__)
 /**
 * @brief 提示级别日志
 *
 * 'I'为 LOG_LVL_INFO 的缩写，表示给本模块上层使用人员查看的重要提示信息日志，例如：初始化成功，当前工作状态等。该级别日志一般在量产时依旧保留。
 * 
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_I(...)                     ulog_i(LOG_TAG, __VA_ARGS__)
 /**
 * @brief 调试级别日志
 *
 * 'D'为 LOG_LVL_DBG 的缩写，表示给本模块开发人员查看的调试日志，该级别日志一般在量产时关闭。
 * 
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_D(...)                     ulog_d(LOG_TAG, __VA_ARGS__)
 /**
 * @brief 输出 raw 日志
 *
 * `LOG_X` 及 `ulog_x` 这类 API 输出都是带格式日志，有些时候需要输出不带任何格式的日志时，可以使用`LOG_RAW` 或 `void ulog_raw(const char *format, ...)` 函数。
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_RAW(...)                   ulog_raw(__VA_ARGS__)

/*
 * backend register and unregister
 */
 /**
 * @brief 注册后端设备
 *
 * 将后端设备注册到 ulog 中，注册前确保后端设备控制块中的函数成员已设置。
 * 
 * @param backend 后端设备对象
 * @param name 后端设备名称
 * @param support_color 是否支持彩色日志
 *
 * @return RT_EOK 成功
 */
rt_err_t ulog_backend_register(ulog_backend_t backend, const char *name, rt_bool_t support_color);

/**
* @brief 注销后端设备
*
* 将后端设备从 ulog 中移除。
*
* @param backend 后端设备对象
*
* @return RT_EOK 成功
*/

rt_err_t ulog_backend_unregister(ulog_backend_t backend);

#ifdef ULOG_USING_FILTER
/*
 * log filter setting
 */
 /**
 * @brief 按模块/标签的级别过滤日志
 *
 * 通过这个 API 可以按照标签的级别来过滤日志。
 * 
 * @param tag 日志的标签
 * @param level 设定的日志级别，详见 ulog_def.h
 *
 * @return >=0 成功；-5 失败，没有足够的内存。
 */
int ulog_tag_lvl_filter_set(const char *tag, rt_uint32_t level);
rt_uint32_t ulog_tag_lvl_filter_get(const char *tag);

 /**
 * @brief 按级别过滤日志（全局）
 *
 * 设定全局的日志过滤器级别，低于这个级别的日志都将停止输出。
 *
 *
 * @param level 设定的级别，可设定的级别包括：
 * 
 * | 级别                  | 名称             |
 * | --------------------- | ---------------- |
 * | LOG_LVL_ASSERT        | 断言             |
 * | LOG_LVL_ERROR         | 错误             |
 * | LOG_LVL_WARNING       | 警告             |
 * | LOG_LVL_INFO          | 信息             |
 * | LOG_LVL_DBG           | 调试             |
 * | LOG_FILTER_LVL_SILENT | 静默（停止输出） |
 * | LOG_FILTER_LVL_ALL    | 全部             |
 */
void ulog_global_filter_lvl_set(rt_uint32_t level);

 /**
 * @brief 按标签过滤日志（全局）
 *
 * 设定全局的日志过滤器标签，只有日志的标签内容中?**包含**?该设定字符串时，才会允许输出。 
 * 例如：有 wifi.driver 、 wifi.mgnt 、audio.driver 3 种标签的日志，当设定过滤标签为 wifi 时，
 * 只有标签为 wifi.driver 及 wifi.mgnt 的日志会输出。同理，当设置过滤标签为 driver 时，
 * 只有标签为 wifi.driver 及 audio.driver 的日志会输出。
 * 
 * @param tag 设定的过滤标签
 */
void ulog_global_filter_tag_set(const char *tag);

 /**
 * @brief 按关键词过滤日志（全局）
 *
 * 设定全局的日志过滤器关键词，只有内容中包含该关键词的日志才会允许输出。
 * 
 * @param keyword 设定的过滤关键词
 */
void ulog_global_filter_kw_set(const char *keyword);
#endif /* ULOG_USING_FILTER */

/**
* @brief 输出缓冲区日志到后端
*
* 由于 ulog 的异步模式具有缓存机制，注册进来的后端内部也可能具有缓存。
* 如果系统出现了 hardfault 、断言等错误情况，但缓存中还有日志没有输出出来，
* 这可能会导致日志丢失的问题，对于查找异常的原因会无从入手。
* 当出现异常时，输出异常信息日志时，同时再调用该函数，即可保证缓存中剩余的日志也能够输出到后端中去。
*/
void ulog_flush(void);

#ifdef ULOG_USING_ASYNC_OUTPUT
/*
 * asynchronous output API
 */
 /**
 * @brief 异步模式下输出全部日志
 *
 * 在异步模式下，如果想要使用其他非 idle 线程作为日志输出线程时，则需要在输出线程中循环调用该 API ，输出缓冲区中日志至所有的后端设备。
 */
void ulog_async_output(void);
void ulog_async_waiting_log(rt_int32_t time);
#endif

/*
 * dump the hex format data to log
 */
 /**
 * @brief 输出 HEX 格式日志
 *
 * 以 16 进制 hex 格式 dump 数据到日志中。
 * 
 * @param name 日志标签
 * @param width 一行 hex 内容的宽度（数量）
 * @param buf 待输出的数据内容
 * @param size 数据大小
 */
void ulog_hexdump(const char *name, rt_size_t width, rt_uint8_t *buf, rt_size_t size);

/*
 * Another log output API. This API is difficult to use than LOG_X API.
 */
void ulog_voutput(rt_uint32_t level, const char *tag, rt_bool_t newline, const char *format, va_list args);
void ulog_output(rt_uint32_t level, const char *tag, rt_bool_t newline, const char *format, ...);

/**
* @brief 输出 raw 日志
*
* 有些时候需要输出不带任何格式的日志时，可以使用`LOG_RAW` 或 `void ulog_raw(const char *format, ...)` 函数。
*
* @param ... 日志内容，格式与 printf 一致
*
* @see LOG_RAW(...)
*/
void ulog_raw(const char *format, ...);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _ULOG_H_ */
