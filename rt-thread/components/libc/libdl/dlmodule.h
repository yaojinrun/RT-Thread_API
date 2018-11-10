/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/08/11     Bernard      the first version
 */

#ifndef RT_DL_MODULE_H__
#define RT_DL_MODULE_H__

#include <rtthread.h>

/**
 * @addtogroup Module
 */

/*@{*/

#define RT_DLMODULE_STAT_INIT       0x00
#define RT_DLMODULE_STAT_RUNNING    0x01
#define RT_DLMODULE_STAT_CLOSING    0x02
#define RT_DLMODULE_STAT_CLOSED     0x03

struct rt_dlmodule;
typedef void* rt_addr_t;

/**
 * @brief ��̬ģ���ʼ������ָ�����Ͷ���
 */
typedef void (*rt_dlmodule_init_func_t)(struct rt_dlmodule *module);
/**
 * @brief ��̬ģ���������ָ�����Ͷ���
 */
typedef void (*rt_dlmodule_cleanup_func_t)(struct rt_dlmodule *module);
/**
 * @brief ��̬ģ����ں���ָ�����Ͷ���
 */
typedef int  (*rt_dlmodule_entry_func_t)(int argc, char** argv);

/**
 * @brief ��̬ģ����ƿ�
 */
struct rt_dlmodule
{
    struct rt_object parent;	/**< @brief �̳��� rt_object */
    rt_list_t object_list;  	/**< @brief ��̬ģ����Ķ������� */

    rt_uint8_t stat;        	/**< @brief ��̬ģ���״̬ */

    /* main thread of this module */
    rt_uint16_t priority;		/**< @brief ���ȼ� */
    rt_uint32_t stack_size;		/**< @brief ��ջ�ռ��С */
    struct rt_thread *main_thread;	/**< @brief ���е����߳� */
    /* the return code */
    int ret_code;				/**< @brief �˳�����ֵ */

    /* VMA base address for the first LOAD segment */
    rt_uint32_t vstart_addr;	/**< @brief ��һ��LOAD�ε�VMA��ַ */

    /* module entry, RT_NULL for dynamic library */
    rt_dlmodule_entry_func_t  entry_addr;	/**< @brief ��ڵ�ַ����̬�����ΪRT_NULL */
    char *cmd_line;         	/**< @brief ������ */

    rt_addr_t   mem_space;  	/**< @brief �ڴ�ռ��ַ */
    rt_uint32_t mem_size;   	/**< @brief �ڴ�ռ��С */

    /* init and clean function */
    rt_dlmodule_init_func_t     init_func;	/**< @brief ��ʼ������ */
    rt_dlmodule_cleanup_func_t  cleanup_func;	/**< @brief ������� */

    rt_uint16_t nref;       	/**< @brief ���ô��� */

    rt_uint16_t nsym;       	/**< @brief ��̬ģ��ķ����� */
    struct rt_module_symtab *symtab;    	/**< @brief ��̬ģ��ķ��ű� */
};

struct rt_dlmodule *dlmodule_create(void);
rt_err_t dlmodule_destroy(struct rt_dlmodule* module);

struct rt_dlmodule *dlmodule_self(void);

/**
 * @brief ���ض�̬ģ��
 *
 * ����������ļ�ϵͳ�м���Ӧ��ģ�鵽�ڴ��У�����ȷ���ط��ظ�ģ���ָ�롣������������ᴴ��
 * һ���߳�ȥִ�������̬ģ�飬������ģ����ص��ڴ��У����������еķ��ŵ�ַ��
 *
 * @param pgname ��̬ģ���·��
 *
 * @return ��ȷ���ط���ģ��ָ�룬���򷵻�NULL��
 */
struct rt_dlmodule *dlmodule_load(const char* pgname);
/**
 * @brief ���ж�̬ģ��
 *
 * �����������`pgname`·�����ض�̬ģ�飬������һ���߳���ִ�������̬ģ���`main`������ͬʱ`cmd`
 * ����Ϊ�����в������ݸ���̬ģ���`main`������ڡ� 
 *
 * @param pgname ��̬ģ���·��
 * @param cmd ������̬ģ������������������ַ���
 * @param cmd_size �������ַ�����С
 *
 * @return ���ز����ж�̬ģ��ɹ��򷵻ض�̬ģ��ָ�룬���򷵻�NULL��
 */
struct rt_dlmodule *dlmodule_exec(const char* pgname, const char* cmd, int cmd_size);
/**
 * @brief ���ö�̬ģ���˳�����ֵ
 *
 * ���������ģ������ʱ���ã�����������ģ���˳��ķ���ֵ`ret_code`��Ȼ���ģ���˳���
 *
 * @param ret_code ģ��ķ��ز���
 */
void dlmodule_exit(int ret_code);

/**
 * @brief ���Ҷ�̬ģ��
 *
 * ���������`name`����ϵͳ���Ƿ��Ѿ��м��صĶ�̬ģ�顣
 *
 * @param name ģ������
 *
 * @return ���ϵͳ���ж�Ӧ�Ķ�̬ģ�飬�򷵻������̬ģ���ָ�룻���򷵻�NULL��
 */
struct rt_dlmodule *dlmodule_find(const char *name);

/**
 * @brief ���ҷ���
 *
 * ���������`sym_str`����ϵͳ���Ƿ��и����Ƶķ��š�
 *
 * @param sym_str ��������
 *
 * @return ���ϵͳ���ж�Ӧ�ķ��ţ��򷵻������̬ģ��ĵ�ַ�����򷵻�NULL��
 */
rt_uint32_t dlmodule_symbol_find(const char *sym_str);

/*@}*/

#endif
