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
 *| `0` | �ɹ�           |
 *| -RT_ENOMEM    | ʧ�ܣ��ڴ治�� |
 *
 */
int ulog_init(void);

/**
 * @brief ULOG ȥ��ʼ��
 *
 * �� ulog ����ʹ��ʱ������ִ�иú����ͷ���Դ��
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
 * 'E'ΪLOG_LVL_ERROR����д����ʾ�������صġ������޸��Ĵ���ʱ�������־���ڴ��󼶱���־ ��
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
 */
#define LOG_E(...)                     ulog_e(LOG_TAG, __VA_ARGS__)
 /**
 * @brief ���漶����־
 *
 * 'W'Ϊ LOG_LVL_WARNING ����д����ʾ����һЩ��̫��Ҫ�ġ����п��޸��ԵĴ���ʱ���������Щ������־��
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
 */
#define LOG_W(...)                     ulog_w(LOG_TAG, __VA_ARGS__)
 /**
 * @brief ��ʾ������־
 *
 * 'I'Ϊ LOG_LVL_INFO ����д����ʾ����ģ���ϲ�ʹ����Ա�鿴����Ҫ��ʾ��Ϣ��־�����磺��ʼ���ɹ�����ǰ����״̬�ȡ��ü�����־һ��������ʱ���ɱ�����
 * 
 * 
 * @param ... ��־���ݣ���ʽ�� printf һ��
 */
#define LOG_I(...)                     ulog_i(LOG_TAG, __VA_ARGS__)
 /**
 * @brief ���Լ�����־
 *
 * 'D'Ϊ LOG_LVL_DBG ����д����ʾ����ģ�鿪����Ա�鿴�ĵ�����־���ü�����־һ��������ʱ�رա�
 * 
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
 /**
 * @brief ע�����豸
 *
 * ������豸ע�ᵽ ulog �У�ע��ǰȷ������豸���ƿ��еĺ�����Ա�����á�
 * 
 * @param backend ����豸����
 * @param name ����豸����
 * @param support_color �Ƿ�֧�ֲ�ɫ��־
 *
 * @return RT_EOK �ɹ�
 */
rt_err_t ulog_backend_register(ulog_backend_t backend, const char *name, rt_bool_t support_color);

/**
* @brief ע������豸
*
* ������豸�� ulog ���Ƴ���
*
* @param backend ����豸����
*
* @return RT_EOK �ɹ�
*/

rt_err_t ulog_backend_unregister(ulog_backend_t backend);

#ifdef ULOG_USING_FILTER
/*
 * log filter setting
 */
 /**
 * @brief ��ģ��/��ǩ�ļ��������־
 *
 * ͨ����� API ���԰��ձ�ǩ�ļ�����������־��
 * 
 * @param tag ��־�ı�ǩ
 * @param level �趨����־������� ulog_def.h
 *
 * @return >=0 �ɹ���-5 ʧ�ܣ�û���㹻���ڴ档
 */
int ulog_tag_lvl_filter_set(const char *tag, rt_uint32_t level);
rt_uint32_t ulog_tag_lvl_filter_get(const char *tag);

 /**
 * @brief �����������־��ȫ�֣�
 *
 * �趨ȫ�ֵ���־���������𣬵�������������־����ֹͣ�����
 *
 *
 * @param level �趨�ļ��𣬿��趨�ļ��������
 * 
 * | ����                  | ����             |
 * | --------------------- | ---------------- |
 * | LOG_LVL_ASSERT        | ����             |
 * | LOG_LVL_ERROR         | ����             |
 * | LOG_LVL_WARNING       | ����             |
 * | LOG_LVL_INFO          | ��Ϣ             |
 * | LOG_LVL_DBG           | ����             |
 * | LOG_FILTER_LVL_SILENT | ��Ĭ��ֹͣ����� |
 * | LOG_FILTER_LVL_ALL    | ȫ��             |
 */
void ulog_global_filter_lvl_set(rt_uint32_t level);

 /**
 * @brief ����ǩ������־��ȫ�֣�
 *
 * �趨ȫ�ֵ���־��������ǩ��ֻ����־�ı�ǩ������?**����**?���趨�ַ���ʱ���Ż���������� 
 * ���磺�� wifi.driver �� wifi.mgnt ��audio.driver 3 �ֱ�ǩ����־�����趨���˱�ǩΪ wifi ʱ��
 * ֻ�б�ǩΪ wifi.driver �� wifi.mgnt ����־�������ͬ�������ù��˱�ǩΪ driver ʱ��
 * ֻ�б�ǩΪ wifi.driver �� audio.driver ����־�������
 * 
 * @param tag �趨�Ĺ��˱�ǩ
 */
void ulog_global_filter_tag_set(const char *tag);

 /**
 * @brief ���ؼ��ʹ�����־��ȫ�֣�
 *
 * �趨ȫ�ֵ���־�������ؼ��ʣ�ֻ�������а����ùؼ��ʵ���־�Ż����������
 * 
 * @param keyword �趨�Ĺ��˹ؼ���
 */
void ulog_global_filter_kw_set(const char *keyword);
#endif /* ULOG_USING_FILTER */

/**
* @brief �����������־�����
*
* ���� ulog ���첽ģʽ���л�����ƣ�ע������ĺ���ڲ�Ҳ���ܾ��л��档
* ���ϵͳ������ hardfault �����Եȴ���������������л�����־û�����������
* ����ܻᵼ����־��ʧ�����⣬���ڲ����쳣��ԭ����޴����֡�
* �������쳣ʱ������쳣��Ϣ��־ʱ��ͬʱ�ٵ��øú��������ɱ�֤������ʣ�����־Ҳ�ܹ�����������ȥ��
*/
void ulog_flush(void);

#ifdef ULOG_USING_ASYNC_OUTPUT
/*
 * asynchronous output API
 */
 /**
 * @brief �첽ģʽ�����ȫ����־
 *
 * ���첽ģʽ�£������Ҫʹ�������� idle �߳���Ϊ��־����߳�ʱ������Ҫ������߳���ѭ�����ø� API ���������������־�����еĺ���豸��
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

/**
* @brief ��� raw ��־
*
* ��Щʱ����Ҫ��������κθ�ʽ����־ʱ������ʹ��`LOG_RAW` �� `void ulog_raw(const char *format, ...)` ������
*
* @param ... ��־���ݣ���ʽ�� printf һ��
*
* @see LOG_RAW(...)
*/
void ulog_raw(const char *format, ...);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _ULOG_H_ */
