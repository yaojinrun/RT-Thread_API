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
 * @brief 动态模块初始化函数指针类型定义
 */
typedef void (*rt_dlmodule_init_func_t)(struct rt_dlmodule *module);
/**
 * @brief 动态模块清除函数指针类型定义
 */
typedef void (*rt_dlmodule_cleanup_func_t)(struct rt_dlmodule *module);
/**
 * @brief 动态模块入口函数指针类型定义
 */
typedef int  (*rt_dlmodule_entry_func_t)(int argc, char** argv);

/**
 * @brief 动态模块控制块
 */
struct rt_dlmodule
{
    struct rt_object parent;	/**< @brief 继承自 rt_object */
    rt_list_t object_list;  	/**< @brief 动态模块里的对象链表 */

    rt_uint8_t stat;        	/**< @brief 动态模块的状态 */

    /* main thread of this module */
    rt_uint16_t priority;		/**< @brief 优先级 */
    rt_uint32_t stack_size;		/**< @brief 堆栈空间大小 */
    struct rt_thread *main_thread;	/**< @brief 运行的主线程 */
    /* the return code */
    int ret_code;				/**< @brief 退出返回值 */

    /* VMA base address for the first LOAD segment */
    rt_uint32_t vstart_addr;	/**< @brief 第一个LOAD段的VMA基址 */

    /* module entry, RT_NULL for dynamic library */
    rt_dlmodule_entry_func_t  entry_addr;	/**< @brief 入口地址，动态库入口为RT_NULL */
    char *cmd_line;         	/**< @brief 命令行 */

    rt_addr_t   mem_space;  	/**< @brief 内存空间地址 */
    rt_uint32_t mem_size;   	/**< @brief 内存空间大小 */

    /* init and clean function */
    rt_dlmodule_init_func_t     init_func;	/**< @brief 初始化函数 */
    rt_dlmodule_cleanup_func_t  cleanup_func;	/**< @brief 清除函数 */

    rt_uint16_t nref;       	/**< @brief 引用次数 */

    rt_uint16_t nsym;       	/**< @brief 动态模块的符号数 */
    struct rt_module_symtab *symtab;    	/**< @brief 动态模块的符号表 */
};

struct rt_dlmodule *dlmodule_create(void);
rt_err_t dlmodule_destroy(struct rt_dlmodule* module);

struct rt_dlmodule *dlmodule_self(void);

/**
 * @brief 加载动态模块
 *
 * 这个函数从文件系统中加载应用模块到内存中，若正确加载返回该模块的指针。这个函数并不会创建
 * 一个线程去执行这个动态模块，仅仅把模块加载到内存中，并解析其中的符号地址。
 *
 * @param pgname 动态模块的路径
 *
 * @return 正确加载返回模块指针，否则返回NULL。
 */
struct rt_dlmodule *dlmodule_load(const char* pgname);
/**
 * @brief 运行动态模块
 *
 * 这个函数根据`pgname`路径加载动态模块，并启动一个线程来执行这个动态模块的`main`函数，同时`cmd`
 * 会作为命令行参数传递给动态模块的`main`函数入口。 
 *
 * @param pgname 动态模块的路径
 * @param cmd 包括动态模块命令自身的命令行字符串
 * @param cmd_size 命令行字符串大小
 *
 * @return 加载并运行动态模块成功则返回动态模块指针，否则返回NULL。
 */
struct rt_dlmodule *dlmodule_exec(const char* pgname, const char* cmd, int cmd_size);
/**
 * @brief 设置动态模块退出返回值
 *
 * 这个函数由模块运行时调用，它可以设置模块退出的返回值`ret_code`，然后从模块退出。
 *
 * @param ret_code 模块的返回参数
 */
void dlmodule_exit(int ret_code);

/**
 * @brief 查找动态模块
 *
 * 这个函数以`name`查找系统中是否已经有加载的动态模块。
 *
 * @param name 模块名称
 *
 * @return 如果系统中有对应的动态模块，则返回这个动态模块的指针；否则返回NULL。
 */
struct rt_dlmodule *dlmodule_find(const char *name);

/**
 * @brief 查找符号
 *
 * 这个函数以`sym_str`查找系统中是否有该名称的符号。
 *
 * @param sym_str 符号名称
 *
 * @return 如果系统中有对应的符号，则返回这个动态模块的地址；否则返回NULL。
 */
rt_uint32_t dlmodule_symbol_find(const char *sym_str);

/*@}*/

#endif
