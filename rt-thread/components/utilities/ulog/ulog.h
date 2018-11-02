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
 * @brief ULOG ��ʼ��
 *
 * ��ʹ�� ulog ǰ������øú������ ulog ��ʼ�����������������Զ���ʼ���� API Ҳ�����Զ����á�
 *
 * @return 
 *| ����  | ����           |
 *| ----- | -------------- |
 *| `>=0` | �ɹ�           |
 *| -5    | ʧ�ܣ��ڴ治�� |
 *
 */
int ulog_init(void);

/**
 * @brief ULOG ����ʼ��
 *
 * �� ulog ����ʹ��ʱ������ִ�и� deinit �ͷ���Դ��
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
 * @brief ���󼶱���־
 *
 * 'E'ΪLOG_LVL_ERROR����д����ʾ�������صġ�**�����޸�**?�Ĵ���ʱ�������־���ڴ��󼶱���־ ��
 * 
 * ��־�����������־����Ҫ�ԣ��� ulog �� **�ɸߵ���** �������¼�����־����
 * 
 * | ����            | ���� | ����                                                         |
 * | --------------- | ---- | ------------------------------------------------------------ |
 * | LOG_LVL_ASSERT  | ���� | �����޷����������ԵĵĴ���������ϵͳ�޷��������еĶ�����־ |
 * | LOG_LVL_ERROR   | ���� | �������صġ� **�����޸�** �Ĵ���ʱ�������־���ڴ��󼶱���־  |
 * | LOG_LVL_WARNING | ���� | ����һЩ��̫��Ҫ�ġ����� **���޸���** �Ĵ���ʱ���������Щ������־ |
 * | LOG_LVL_INFO    | ��Ϣ | ����ģ���ϲ�ʹ����Ա�鿴����Ҫ��ʾ��Ϣ��־�����磺��ʼ���ɹ�����ǰ����״̬�ȡ��ü�����־һ��������ʱ���� **����** |
 * | LOG_LVL_DBG     | ���� | ����ģ�鿪����Ա�鿴�ĵ�����־���ü�����־һ��������ʱ **�ر�** |
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
 */
#define LOG_E(...)                     ulog_e(LOG_TAG, __VA_ARGS__)
 /**
 * @brief ���漶����־
 *
 * 'W'Ϊ LOG_LVL_WARNING ����д����ʾ����һЩ��̫��Ҫ�ġ�����?**���޸���**?�Ĵ���ʱ���������Щ������־��
 * 
 * @see LOG_E
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
 */
#define LOG_W(...)                     ulog_w(LOG_TAG, __VA_ARGS__)
 /**
 * @brief ��ʾ������־
 *
 * 'I'Ϊ LOG_LVL_INFO ����д����ʾ����ģ���ϲ�ʹ����Ա�鿴����Ҫ��ʾ��Ϣ��־�����磺��ʼ���ɹ�����ǰ����״̬�ȡ��ü�����־һ��������ʱ����?**����** ��
 * 
 * @see LOG_E
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
 */
#define LOG_I(...)                     ulog_i(LOG_TAG, __VA_ARGS__)
 /**
 * @brief ���Լ�����־
 *
 * 'D'Ϊ LOG_LVL_DBG ����д����ʾ����ģ�鿪����Ա�鿴�ĵ�����־���ü�����־һ��������ʱ?**�ر�** ��
 * 
 * @see LOG_E
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
 */
#define LOG_D(...)                     ulog_d(LOG_TAG, __VA_ARGS__)
 /**
 * @brief ��� raw ��־
 *
 * `LOG_X` �� `ulog_x` ���� API ������Ǵ���ʽ��־����Щʱ����Ҫ��������κθ�ʽ����־ʱ������ʹ��`LOG_RAW` �� `void ulog_raw(const char *format, ...)` ������
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
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
 * @brief ��� HEX ��ʽ��־
 *
 * �� 16 ���� hex ��ʽ dump ���ݵ���־�С�
 * 
 * @param name ��־��ǩ
 * @param width һ�� hex ���ݵĿ�ȣ�������
 * @param buf ���������������
 * @param size ���ݴ�С
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
