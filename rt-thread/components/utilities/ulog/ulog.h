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
 *| `>=0` | 成功           |
 *| -5    | 失败，内存不足 |
 *
 */
int ulog_init(void);

/**
 * @brief ULOG 反初始化
 *
 * 当 ulog 不再使用时，可以执行该 deinit 释放资源。
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
 * 'E'为LOG_LVL_ERROR的缩写，表示发生严重的、**不可修复**?的错误时输出的日志属于错误级别日志 。
 * 
 * 日志级别代表了日志的重要性，在 ulog 中 **由高到低** ，有如下几个日志级别
 * 
 * | 级别            | 名称 | 描述                                                         |
 * | --------------- | ---- | ------------------------------------------------------------ |
 * | LOG_LVL_ASSERT  | 断言 | 发生无法处理、致命性的的错误，以至于系统无法继续运行的断言日志 |
 * | LOG_LVL_ERROR   | 错误 | 发生严重的、 **不可修复** 的错误时输出的日志属于错误级别日志  |
 * | LOG_LVL_WARNING | 警告 | 出现一些不太重要的、具有 **可修复性** 的错误时，会输出这些警告日志 |
 * | LOG_LVL_INFO    | 信息 | 给本模块上层使用人员查看的重要提示信息日志，例如：初始化成功，当前工作状态等。该级别日志一般在量产时依旧 **保留** |
 * | LOG_LVL_DBG     | 调试 | 给本模块开发人员查看的调试日志，该级别日志一般在量产时 **关闭** |
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_E(...)                     ulog_e(LOG_TAG, __VA_ARGS__)
 /**
 * @brief 警告级别日志
 *
 * 'W'为 LOG_LVL_WARNING 的缩写，表示出现一些不太重要的、具有?**可修复性**?的错误时，会输出这些警告日志。
 * 
 * @see LOG_E
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_W(...)                     ulog_w(LOG_TAG, __VA_ARGS__)
 /**
 * @brief 提示级别日志
 *
 * 'I'为 LOG_LVL_INFO 的缩写，表示给本模块上层使用人员查看的重要提示信息日志，例如：初始化成功，当前工作状态等。该级别日志一般在量产时依旧?**保留** 。
 * 
 * @see LOG_E
 * 
 * @param ... 日志内容，格式与 printf 一致
 */
#define LOG_I(...)                     ulog_i(LOG_TAG, __VA_ARGS__)
 /**
 * @brief 调试级别日志
 *
 * 'D'为 LOG_LVL_DBG 的缩写，表示给本模块开发人员查看的调试日志，该级别日志一般在量产时?**关闭** 。
 * 
 * @see LOG_E
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
rt_err_t ulog_backend_register(ulog_backend_t backend, const char *name, rt_bool_t support_color);
rt_err_t ulog_backend_unregister(ulog_backend_t backend);

#ifdef ULOG_USING_FILTER
/*
 * log filter setting
 */
int ulog_tag_lvl_filter_set(const char *tag, rt_uint32_t level);
rt_uint32_t ulog_tag_lvl_filter_get(const char *tag);
void ulog_global_filter_lvl_set(rt_uint32_t level);
void ulog_global_filter_tag_set(const char *tag);
void ulog_global_filter_kw_set(const char *keyword);
#endif /* ULOG_USING_FILTER */

/*
 * flush all backends's log
 */
void ulog_flush(void);

#ifdef ULOG_USING_ASYNC_OUTPUT
/*
 * asynchronous output API
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
void ulog_raw(const char *format, ...);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _ULOG_H_ */
