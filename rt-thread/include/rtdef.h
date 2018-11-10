/*
 * File      : rtdef.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
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
 * 2007-01-10     Bernard      the first version
 * 2008-07-12     Bernard      remove all rt_int8, rt_uint32_t etc typedef
 * 2010-10-26     yi.qiu       add module support
 * 2010-11-10     Bernard      add cleanup callback function in thread exit.
 * 2011-05-09     Bernard      use builtin va_arg in GCC 4.x
 * 2012-11-16     Bernard      change RT_NULL from ((void*)0) to 0.
 * 2012-12-29     Bernard      change the RT_USING_MEMPOOL location and add
 *                             RT_USING_MEMHEAP condition.
 * 2012-12-30     Bernard      add more control command for graphic.
 * 2013-01-09     Bernard      change version number.
 * 2015-02-01     Bernard      change version number to v2.1.0
 * 2017-08-31     Bernard      change version number to v3.0.0
 * 2017-11-30     Bernard      change version number to v3.0.1
 * 2017-12-27     Bernard      change version number to v3.0.2
 * 2018-02-24     Bernard      change version number to v3.0.3
 * 2018-04-25     Bernard      change version number to v3.0.4
 * 2018-05-31     Bernard      change version number to v3.1.0
 */

#ifndef __RT_DEF_H__
#define __RT_DEF_H__

/* include rtconfig header to import configuration */
#include <rtconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup BasicDef
 */

/*@{*/

/* RT-Thread 版本信息 */
#define RT_VERSION                      3L              /**< @brief 主版本号 */
#define RT_SUBVERSION                   1L              /**< @brief 次版本号 */
#define RT_REVISION                     0L              /**< @brief 修订版本号 */

/* RT-Thread 版本 */
#define RTTHREAD_VERSION                ((RT_VERSION * 10000) + \
                                         (RT_SUBVERSION * 100) + RT_REVISION)

/* RT-Thread basic data type definitions */
typedef signed   char                   rt_int8_t;      /**< @brief  8位整数类型 */
typedef signed   short                  rt_int16_t;     /**< @brief 16位整数类型 */
typedef signed   long                   rt_int32_t;     /**< @brief 32位整数类型 */
typedef unsigned char                   rt_uint8_t;     /**< @brief  8位无符号整数类型 */
typedef unsigned short                  rt_uint16_t;    /**< @brief 16位无符号整数类型 */
typedef unsigned long                   rt_uint32_t;    /**< @brief 32位无符号整数类型 */
typedef int                             rt_bool_t;      /**< @brief 布尔类型 */

/* 32bit CPU */
typedef long                            rt_base_t;      /**< @brief N位CPU 相关长数据类型 */
typedef unsigned long                   rt_ubase_t;     /**< @brief N位CPU 相关无符号长数据类型 */

typedef rt_base_t                       rt_err_t;       /**< @brief 错误号类型 */
typedef rt_uint32_t                     rt_time_t;      /**< @brief 时间戳类型 */
typedef rt_uint32_t                     rt_tick_t;      /**< @brief 系统时钟节拍计数类型 */
typedef rt_base_t                       rt_flag_t;      /**< @brief 参数类型 */
typedef rt_ubase_t                      rt_size_t;      /**< @brief 字节大小类型 */
typedef rt_ubase_t                      rt_dev_t;       /**< @brief 设备类型 */
typedef rt_base_t                       rt_off_t;       /**< @brief 输入偏移类型 */

/* boolean type definitions */
#define RT_TRUE                         1               /**< @brief 布尔值 真  */
#define RT_FALSE                        0               /**< @brief 布尔值 假 */


/* maximum value of base type */
#define RT_UINT8_MAX                    0xff            /**< @brief UINT8 的最大值 */
#define RT_UINT16_MAX                   0xffff          /**< @brief UINT16 的最大值 */
#define RT_UINT32_MAX                   0xffffffff      /**< @brief UINT32 的最大值 */
#define RT_TICK_MAX                     RT_UINT32_MAX   /**< @brief tick 的最大值 */

/*@}*/

/* Compiler Related Definitions */
#ifdef __CC_ARM                         /* ARM Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __weak
    #define rt_inline                   static __inline
    /* module compiling */
    #ifdef RT_USING_MODULE
        #define RTT_API                 __declspec(dllimport)
    #else
        #define RTT_API                 __declspec(dllexport)
    #endif

#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  @ x
    #define RT_UNUSED
    #define RT_USED                     __root
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
    #define RT_WEAK                     __weak
    #define rt_inline                   static inline
    #define RTT_API

#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #ifdef RT_USING_NEWLIB
        #include <stdarg.h>
    #else
        /* the version of GNU GCC must be greater than 4.x */
        typedef __builtin_va_list   __gnuc_va_list;
        typedef __gnuc_va_list      va_list;
        #define va_start(v,l)       __builtin_va_start(v,l)
        #define va_end(v)           __builtin_va_end(v)
        #define va_arg(v,l)         __builtin_va_arg(v,l)
    #endif

    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __attribute__((weak))
    #define rt_inline                   static __inline
    #define RTT_API
#elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __attribute__((weak))
    #define rt_inline                   static inline
    #define RTT_API
#elif defined (_MSC_VER)
    #include <stdarg.h>
    #define SECTION(x)
    #define RT_UNUSED
    #define RT_USED
    #define ALIGN(n)                    __declspec(align(n))
    #define RT_WEAK
    #define rt_inline                   static __inline
    #define RTT_API
#elif defined (__TI_COMPILER_VERSION__)
    #include <stdarg.h>
    /* The way that TI compiler set section is different from other(at least
     * GCC and MDK) compilers. See ARM Optimizing C/C++ Compiler 5.9.3 for more
     * details. */
    #define SECTION(x)
    #define RT_UNUSED
    #define RT_USED
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)
    #define RT_WEAK
    #define rt_inline                   static inline
    #define RTT_API
#else
    #error not supported tool chain
#endif

/* initialization export */
#ifdef RT_USING_COMPONENTS_INIT

/**
 * @addtogroup SystemInit
 */

/**@{*/

typedef int (*init_fn_t)(void);
#ifdef _MSC_VER /* we do not support MS VC++ compiler */
    #define INIT_EXPORT(fn, level)
#else
    #if RT_DEBUG_INIT
        struct rt_init_desc
        {
            const char* fn_name;
            const init_fn_t fn;
        };
        #define INIT_EXPORT(fn, level)          \
            const char __rti_##fn##_name[] = #fn; \
            const struct rt_init_desc __rt_init_desc_##fn SECTION(".rti_fn."level) = \
            { __rti_##fn##_name, fn};
    #else
        #define INIT_EXPORT(fn, level)  \
            const init_fn_t __rt_init_##fn SECTION(".rti_fn."level) = fn
    #endif
#endif
#else
#define INIT_EXPORT(fn, level)
#endif

/* board init routines will be called in board_init() function */

#define INIT_BOARD_EXPORT(fn)           INIT_EXPORT(fn, "1")	/**< @brief 板级驱动初始化，fn为函数指针 */

/* pre/device/component/env/app init routines will be called in init_thread */
/* components pre-initialization (pure software initilization) */

#define INIT_PREV_EXPORT(fn)            INIT_EXPORT(fn, "2")	/**< @brief 纯软件函数自动初始化，fn为函数指针 */
/* device initialization */

#define INIT_DEVICE_EXPORT(fn)          INIT_EXPORT(fn, "3")	/**< @brief 设备自动初始化，fn为函数指针 */
/* components initialization (dfs, lwip, ...) */

#define INIT_COMPONENT_EXPORT(fn)       INIT_EXPORT(fn, "4")	/**< @brief 组件自动初始化，fn为函数指针 */
/* environment initialization (mount disk, ...) */

#define INIT_ENV_EXPORT(fn)             INIT_EXPORT(fn, "5")	/**< @brief 系统环境自动初始化，fn为函数指针 */
/* appliation initialization (rtgui application etc ...) */

#define INIT_APP_EXPORT(fn)             INIT_EXPORT(fn, "6")	/**< @brief 应用自动初始化，fn为函数指针 */

#if !defined(RT_USING_FINSH)
/* define these to empty, even if not include finsh.h file */
#define FINSH_FUNCTION_EXPORT(name, desc)
#define FINSH_FUNCTION_EXPORT_ALIAS(name, alias, desc)
#define FINSH_VAR_EXPORT(name, type, desc)

#define MSH_CMD_EXPORT(command, desc)
#define MSH_CMD_EXPORT_ALIAS(command, alias, desc)
#elif !defined(FINSH_USING_SYMTAB)
#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc)

#endif

/**@}*/

/* event length */
#define RT_EVENT_LENGTH                 32

/* memory management option */
#define RT_MM_PAGE_SIZE                 4096
#define RT_MM_PAGE_MASK                 (RT_MM_PAGE_SIZE - 1)
#define RT_MM_PAGE_BITS                 12

/* kernel malloc definitions */
#ifndef RT_KERNEL_MALLOC
#define RT_KERNEL_MALLOC(sz)            rt_malloc(sz)
#endif

#ifndef RT_KERNEL_FREE
#define RT_KERNEL_FREE(ptr)             rt_free(ptr)
#endif

#ifndef RT_KERNEL_REALLOC
#define RT_KERNEL_REALLOC(ptr, size)    rt_realloc(ptr, size)
#endif

/**
 * @addtogroup Error
 */

/*@{*/

/* RT-Thread error code definitions */
#define RT_EOK                          0               /**< @brief 无错误 */
#define RT_ERROR                        1               /**< @brief 一般错误 */
#define RT_ETIMEOUT                     2               /**< @brief 超时错误 */
#define RT_EFULL                        3               /**< @brief 资源已满 */
#define RT_EEMPTY                       4               /**< @brief 资源已空 */
#define RT_ENOMEM                       5               /**< @brief 内存不足 */
#define RT_ENOSYS                       6               /**< @brief 无系统 */
#define RT_EBUSY                        7               /**< @brief 忙线中 */
#define RT_EIO                          8               /**< @brief IO 错误 */
#define RT_EINTR                        9               /**< @brief 中断系统调用 */
#define RT_EINVAL                       10              /**< @brief 无效参数 */

/*@}*/

/**
 * @addtogroup BasicDef
 */

/*@{*/
#define RT_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1)) /**< @brief 返回以指定宽度对齐的最连续大小。例如 RT_ALIGN(13, 4)将返回16 */

#define RT_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1)) /**< @brief 返回指定宽度对齐的向下数。例如 RT_ALIGN_DOWN(13, 4)将返回12 */

#define RT_NULL                         (0) /**< @brief 相当于C库中的NULL */

/*@}*/

/**
 * @ingroup list
 *
 * @brief 双向链表节点
 */
struct rt_list_node
{
    struct rt_list_node *next;                          /**< @brief 指向下一个节点。 */
    struct rt_list_node *prev;                          /**< @brief 指向上一个节点。 */
};
/**
 * @ingroup list
 */
typedef struct rt_list_node rt_list_t;                  /**< @brief 双向链表类型定义 */

/**
 * @ingroup slist
 *
 * @brief 单向链表节点
 */
struct rt_slist_node
{
    struct rt_slist_node *next;                         /**< @brief 指向下一个节点。 */
};
/**
 * @ingroup slist
 *
 * @brief 单向链表节点类型定义
 */
typedef struct rt_slist_node rt_slist_t;                /**< @brief 单向链表类型定义 */

/**
 * @addtogroup KernelObject
 */

/*@{*/

/*
 * kernel object macros
 */
#define RT_OBJECT_FLAG_MODULE           0x80            /**< @brief 动态模块对象标志 */

/**
 * @brief 内核对象基类控制块
 */
struct rt_object
{
    char       name[RT_NAME_MAX];                       /**< @brief 内核对象的名称 */
    rt_uint8_t type;                                    /**< @brief 内核对象的类型 */
    rt_uint8_t flag;                                    /**< @brief 内核对象的参数 */

#ifdef RT_USING_MODULE
    void      *module_id;                               /**< @brief 动态模块的id */
#endif
    rt_list_t  list;                                    /**< @brief 内核对象管理链表 */
};

typedef struct rt_object *rt_object_t;                  /**< @brief 内核对象类型指针定义 */

/**
 * @brief 内核对象类型
 */
enum rt_object_class_type
{
    RT_Object_Class_Thread = 0,                         /**< @brief 线程对象 */
    RT_Object_Class_Semaphore,                          /**< @brief 信号量对象 */
    RT_Object_Class_Mutex,                              /**< @brief 互斥量对象 */
    RT_Object_Class_Event,                              /**< @brief 事件对象 */
    RT_Object_Class_MailBox,                            /**< @brief 邮箱对象 */
    RT_Object_Class_MessageQueue,                       /**< @brief 消息队列对象 */
    RT_Object_Class_MemHeap,                            /**< @brief 内存堆对象 */
    RT_Object_Class_MemPool,                            /**< @brief 内存池对象 */
    RT_Object_Class_Device,                             /**< @brief 设备对象 */
    RT_Object_Class_Timer,                              /**< @brief 定时器对象 */
    RT_Object_Class_Module,                             /**< @brief 动态模块对象 */
    RT_Object_Class_Unknown,                            /**< @brief 未知对象 */
    RT_Object_Class_Static = 0x80                       /**< @brief 静态对象 */
};

/**
 * @brief 内核对象信息
 */
struct rt_object_information
{
    enum rt_object_class_type type;                     /**< @brief 对象类型 */
    rt_list_t                 object_list;              /**< @brief 对象链表 */
    rt_size_t                 object_size;              /**< @brief 对象大小 */
};

/*@}*/

/**
 * @addtogroup Hook
 */

/**@{*/

#ifdef RT_USING_HOOK
#define RT_OBJECT_HOOK_CALL(func, argv) \
    do { if ((func) != RT_NULL) func argv; } while (0)	/**< @brief 内核对象钩子函数 */
#else
#define RT_OBJECT_HOOK_CALL(func, argv)
#endif

/*@}*/

/**
 * @addtogroup Timer
 */

/*@{*/

/*
 * clock & timer macros
 */
#define RT_TIMER_FLAG_DEACTIVATED       0x0             /**< @brief 定时器未激活 */
#define RT_TIMER_FLAG_ACTIVATED         0x1             /**< @brief 定时器已激活 */
#define RT_TIMER_FLAG_ONE_SHOT          0x0             /**< @brief 一次性计时器 */
#define RT_TIMER_FLAG_PERIODIC          0x2             /**< @brief 周期性定时器 */

#define RT_TIMER_FLAG_HARD_TIMER        0x0             /**< @brief 硬件定时器，定时器的回调函数将在系统时钟中断里调用。 */
#define RT_TIMER_FLAG_SOFT_TIMER        0x4             /**< @brief 软件定时器，定时器的回调函数将在定时器线程中调用。 */

#define RT_TIMER_CTRL_SET_TIME          0x0             /**< @brief 设置定时器时间 */
#define RT_TIMER_CTRL_GET_TIME          0x1             /**< @brief 获取定时器时间 */
#define RT_TIMER_CTRL_SET_ONESHOT       0x2             /**< @brief 将计时器更改为一次性定时器 */
#define RT_TIMER_CTRL_SET_PERIODIC      0x3             /**< @brief 将计时器更改为周期性定时器 */

#ifndef RT_TIMER_SKIP_LIST_LEVEL
#define RT_TIMER_SKIP_LIST_LEVEL          1
#endif

/* 1 or 3 */
#ifndef RT_TIMER_SKIP_LIST_MASK
#define RT_TIMER_SKIP_LIST_MASK         0x3
#endif

/**
 * @brief 定时器控制块
 */
struct rt_timer
{
    struct rt_object parent;                            /**< @brief 从 rt_object 继承 */

    rt_list_t        row[RT_TIMER_SKIP_LIST_LEVEL];

    void (*timeout_func)(void *parameter);              /**< @brief 超时回调函数 */
    void            *parameter;                         /**< @brief 超时回调函数的传入参数 */

    rt_tick_t        init_tick;                         /**< @brief 定时器开始时钟节拍数*/
    rt_tick_t        timeout_tick;                      /**< @brief 定时器超时时钟节拍数*/
};
/**
 * @brief 定时器类型指针定义
 */
typedef struct rt_timer *rt_timer_t;

/*@}*/

/**
 * @addtogroup Signal
 */
#ifdef RT_USING_SIGNALS
#include <libc/libc_signal.h>
typedef unsigned long rt_sigset_t;
typedef void (*rt_sighandler_t)(int signo);
typedef siginfo_t rt_siginfo_t;

#define RT_SIG_MAX          32
#endif
/*@}*/

/**
 * @addtogroup Thread
 */

/*@{*/

/*
 * Thread
 */

/*
 * 线程状态定义
 */
#define RT_THREAD_INIT                  0x00                /**< @brief 初始化完成状态 */
#define RT_THREAD_READY                 0x01                /**< @brief 就绪状态 */
#define RT_THREAD_SUSPEND               0x02                /**< @brief 挂起状态 */
#define RT_THREAD_RUNNING               0x03                /**< @brief 运行状态 */
#define RT_THREAD_BLOCK                 RT_THREAD_SUSPEND   /**< @brief 阻塞状态 */
#define RT_THREAD_CLOSE                 0x04                /**< @brief 关闭状态 */
#define RT_THREAD_STAT_MASK             0x0f

#define RT_THREAD_STAT_SIGNAL           0x10
#define RT_THREAD_STAT_SIGNAL_READY     (RT_THREAD_STAT_SIGNAL | RT_THREAD_READY)
#define RT_THREAD_STAT_SIGNAL_WAIT      0x20
#define RT_THREAD_STAT_SIGNAL_MASK      0xf0

/*
 * 线程控制命令定义
 */
#define RT_THREAD_CTRL_STARTUP          0x00                /**< @brief 启动线程命令. */
#define RT_THREAD_CTRL_CLOSE            0x01                /**< @brief 关闭线程命令. */
#define RT_THREAD_CTRL_CHANGE_PRIORITY  0x02                /**< @brief 改变线程优先级命令. */
#define RT_THREAD_CTRL_INFO             0x03                /**< @brief 获取线程信息命令. */

/**
 * @brief 线程控制块
 */
struct rt_thread
{
    /* rt object */
    char        name[RT_NAME_MAX];                      /**< @brief 线程名称 */
    rt_uint8_t  type;                                   /**< @brief 对象类型 */
    rt_uint8_t  flags;                                  /**< @brief 线程的参数 */

#ifdef RT_USING_MODULE
    void       *module_id;                              /**< @brief 动态模块ID */
#endif

    rt_list_t   list;                                   /**< @brief 链表对象 */
    rt_list_t   tlist;                                  /**< @brief 线程链表 */

    /* stack point and entry */
    void       *sp;                                     /**< @brief 栈指针 */
    void       *entry;                                  /**< @brief 入口函数指针 */
    void       *parameter;                              /**< @brief 线程入口参数 */
    void       *stack_addr;                             /**< @brief 堆栈地址 */
    rt_uint32_t stack_size;                             /**< @brief 堆栈大小 */

    /* error code */
    rt_err_t    error;                                  /**< @brief 错误代码 */

    rt_uint8_t  stat;                                   /**< @brief 线程状态 */

    /* priority */
    rt_uint8_t  current_priority;                       /**< @brief 当前优先级 */
    rt_uint8_t  init_priority;                          /**< @brief 初始化优先级 */
#if RT_THREAD_PRIORITY_MAX > 32
    rt_uint8_t  number;
    rt_uint8_t  high_mask;
#endif
    rt_uint32_t number_mask;

#if defined(RT_USING_EVENT)
    /* thread event */
    rt_uint32_t event_set;								/**< @brief 线程等待的事件集 */
    rt_uint8_t  event_info;								/**< @brief 线程等待的事件标志 and/or/clear */
#endif

#if defined(RT_USING_SIGNALS)
    rt_sigset_t     sig_pending;                        /**< @brief 待处理的信号 */
    rt_sigset_t     sig_mask;                           /**< @brief 信号的掩码位 */

    void            *sig_ret;                           /**< @brief 信号返回的堆栈指针 */
    rt_sighandler_t *sig_vectors;                       /**< @brief 信号处理的向量 */
    void            *si_list;                           /**< @brief 信号列表 */
#endif

    rt_ubase_t  init_tick;                              /**< @brief 线程的初始化时钟节拍 */
    rt_ubase_t  remaining_tick;                         /**< @brief 剩余时钟节拍数 */

    struct rt_timer thread_timer;                       /**< @brief 线程的内置计时器 */

    void (*cleanup)(struct rt_thread *tid);             /**< @brief 线程退出时的清理回调函数 */

    rt_uint32_t user_data;                              /**< @brief 线程的私有用户数据 */
};
/**
 * @brief 线程类型指针定义
 */
typedef struct rt_thread *rt_thread_t;

/*@}*/

/**
 * @addtogroup IPC
 */

/*@{*/

/*
 * IPC标志和控制命令定义
 */
#define RT_IPC_FLAG_FIFO                0x00            /**< @brief 先进先出模式*/
#define RT_IPC_FLAG_PRIO                0x01            /**< @brief 优先级模式 */

#define RT_IPC_CMD_UNKNOWN              0x00            /**< @brief 未知的 IPC 命令 */
#define RT_IPC_CMD_RESET                0x01            /**< @brief 复位 IPC 对象命令 */

#define RT_WAITING_FOREVER              -1              /**< @brief 永远阻塞直到获得资源 */
#define RT_WAITING_NO                   0               /**< @brief 无阻塞 */

/**
 * @brief IPC基类控制块
 */
struct rt_ipc_object
{
    struct rt_object parent;                            /**< @brief 从 rt_object 继承 */

    rt_list_t        suspend_thread;                    /**< @brief 挂起线程链表 */
};

/*@}*/
#ifdef RT_USING_SEMAPHORE

/**
 * @addtogroup semaphore
 */

/**@{*/

/**
 * @brief 信号量控制块
 */
struct rt_semaphore
{
    struct rt_ipc_object parent;                        /**< @brief 继承自 rt_ipc_object */

    rt_uint16_t          value;                         /**< @brief 信号量的值 */
};
/**
 * @brief 信号量类型指针定义
 */
typedef struct rt_semaphore *rt_sem_t;

/*@}*/
#endif

#ifdef RT_USING_MUTEX

/**
 * @addtogroup mutex
 */

/**@{*/

/**
 * @brief 互斥量控制块
 */
struct rt_mutex
{
    struct rt_ipc_object parent;                        /**< @brief 继承自 rt_ipc_object */

    rt_uint16_t          value;                         /**< @brief 互斥量的值 */

    rt_uint8_t           original_priority;             /**< @brief 最后一个执有该互斥量的线程的优先级 */
    rt_uint8_t           hold;                          /**< @brief 线程持有该互斥量的次数 */

    struct rt_thread    *owner;                         /**< @brief 当前该互斥量的持有者（线程） */
};
/**
 * @brief 互斥量类型指针定义
 */
typedef struct rt_mutex *rt_mutex_t;

/**@}*/
#endif

#ifdef RT_USING_EVENT
/**
 * @addtogroup event
 */

/**@{*/

/*
 * 事件参数
 */
#define RT_EVENT_FLAG_AND               0x01            /**< @brief 逻辑与参数 */
#define RT_EVENT_FLAG_OR                0x02            /**< @brief 逻辑或参数 */
#define RT_EVENT_FLAG_CLEAR             0x04            /**< @brief 清除参数 */

/**
 * @brief 事件控制块
 */
struct rt_event
{
    struct rt_ipc_object parent;                        /**< @brief 继承自rt_ipc_object */

    rt_uint32_t          set;                           /**< @brief 时间集 */
};
/**
 * @brief 事件类型指针定义
 */
typedef struct rt_event *rt_event_t;

/**@}*/
#endif

#ifdef RT_USING_MAILBOX
/**
 * @addtogroup mailbox
 */

/**@{*/

/**
 * @brief 邮箱控制块
 */
struct rt_mailbox
{
    struct rt_ipc_object parent;                        /**< @brief 继承自 rt_ipc_object */

    rt_uint32_t         *msg_pool;                      /**< @brief 消息缓冲区的起始地址 */

    rt_uint16_t          size;                          /**< @brief 消息池的大小 */

    rt_uint16_t          entry;                         /**< @brief msg_pool中的消息索引 */
    rt_uint16_t          in_offset;                     /**< @brief 消息缓冲区的输入偏移量 */
    rt_uint16_t          out_offset;                    /**< @brief 消息缓冲区的输出偏移量 */

    rt_list_t            suspend_sender_thread;         /**< @brief 挂起在该邮箱上的发送线程 */
};
/**
 * @brief 邮箱类型指针定义
 */
typedef struct rt_mailbox *rt_mailbox_t;

/**@}*/
#endif

#ifdef RT_USING_MESSAGEQUEUE
/**
 * @addtogroup messagequeue
 */

/**@{*/

/**
 * @brief 消息队列控制块
 */
struct rt_messagequeue
{
    struct rt_ipc_object parent;                        /**< @brief 继承自rt_ipc_object */

    void                *msg_pool;                      /**< @brief 消息队列的起始地址 */

    rt_uint16_t          msg_size;                      /**< @brief 每条消息的消息大小 */
    rt_uint16_t          max_msgs;                      /**< @brief 最大消息数 */

    rt_uint16_t          entry;                         /**< @brief 队列中的消息索引 */

    void                *msg_queue_head;                /**< @brief 链表头 */
    void                *msg_queue_tail;                /**< @brief 链表尾 */
    void                *msg_queue_free;                /**< @brief 指向队列中的空闲节点的指针 */
};
/**
 * @brief 消息队列类型指针定义
 */
typedef struct rt_messagequeue *rt_mq_t;

/**@}*/
#endif

/*
 * memory management
 * heap & partition
 */


#ifdef RT_USING_MEMHEAP

/**
 * @addtogroup MM
 */

/*@{*/

/**
 * @brief 内存堆管理控制块
 */
struct rt_memheap_item
{
    rt_uint32_t             magic;                      /**< @brief 内存堆的幻数 */
    struct rt_memheap      *pool_ptr;                   /**< @brief 内存池的指针 */

    struct rt_memheap_item *next;                       /**< @brief 下一个内存堆 */
    struct rt_memheap_item *prev;                       /**< @brief 上一个内存堆 */

    struct rt_memheap_item *next_free;                  /**< @brief 下一个空闲内存堆 */
    struct rt_memheap_item *prev_free;                  /**< @brief 上一个空闲内存堆 */
};

/**
 * @brief 内存堆控制块
 */
struct rt_memheap
{
    struct rt_object        parent;                     /**< @brief 继承自 rt_object */

    void                   *start_addr;                 /**< @brief 内存堆的起始地址 */

    rt_uint32_t             pool_size;                  /**< @brief 内存堆的大小 */
    rt_uint32_t             available_size;             /**< @brief 内存堆可用大小 */
    rt_uint32_t             max_used_size;              /**< @brief 最大可分配大小 */

    struct rt_memheap_item *block_list;                 /**< @brief 已使用的块链表*/

    struct rt_memheap_item *free_list;                  /**< @brief 未使用的块链表 */
    struct rt_memheap_item  free_header;                /**< @brief 未使用的块链表表头 */

    struct rt_semaphore     lock;                       /**< @brief 信号量锁 */
};

/*@}*/
#endif


/**
 * @addtogroup Pool
 */

/*@{*/

#ifdef RT_USING_MEMPOOL
/**
 * @brief 内存池控制块
 */
struct rt_mempool
{
    struct rt_object parent;                            /**< @brief 继承自 rt_object */

    void            *start_address;                     /**< @brief 内存池的起始地址 */
    rt_size_t        size;                              /**< @brief 内存池的大小 */

    rt_size_t        block_size;                        /**< @brief 内存块的大小 */
    rt_uint8_t      *block_list;                        /**< @brief 内存块链表 */

    rt_size_t        block_total_count;                 /**< @brief 内存块的总数量 */
    rt_size_t        block_free_count;                  /**< @brief 未使用内存块的数量 */

    rt_list_t        suspend_thread;                    /**< @brief 挂起线程链表 */
    rt_size_t        suspend_thread_count;              /**< @brief 挂起线程的数量 */
};
/**
 * @brief 内存池类型指针定义
 */
typedef struct rt_mempool *rt_mp_t;
#endif

/*@}*/

#ifdef RT_USING_DEVICE
/**
 * @addtogroup Device
 */

/*@{*/

/**
 * (I/O)设备类型
 */
enum rt_device_class_type
{
    RT_Device_Class_Char = 0,                           /**< 字符设备 */
    RT_Device_Class_Block,                              /**< 块设备 */
    RT_Device_Class_NetIf,                              /**< 网络接口设备 */
    RT_Device_Class_MTD,                                /**< 存储设备 */
    RT_Device_Class_CAN,                                /**< CAN 设备 */
    RT_Device_Class_RTC,                                /**< RTC 设备 */
    RT_Device_Class_Sound,                              /**< 音频设备 */
    RT_Device_Class_Graphic,                            /**< 图形设备 */
    RT_Device_Class_I2CBUS,                             /**< I2C 总线设备 */
    RT_Device_Class_USBDevice,                          /**< USB从设备 */
    RT_Device_Class_USBHost,                            /**< USB主机总线 */
    RT_Device_Class_SPIBUS,                             /**< SPI总线设备 */
    RT_Device_Class_SPIDevice,                          /**< SPI 从设备 */
    RT_Device_Class_SDIO,                               /**< SDIO 设备 */
    RT_Device_Class_PM,                                 /**< 电源管理设备 */
    RT_Device_Class_Pipe,                               /**< 管道设备 */
    RT_Device_Class_Portal,                             /**< 双向管道设备 */
    RT_Device_Class_Timer,                              /**< 定时器设备 */
    RT_Device_Class_Miscellaneous,                      /**< 杂项设备 */
    RT_Device_Class_Unknown                             /**< 未知设备 */
};

/*
 * 设备标志定义
 */
#define RT_DEVICE_FLAG_DEACTIVATE       0x000           /**< @brief 设备未初始化 */

#define RT_DEVICE_FLAG_RDONLY           0x001           /**< @brief 只读 */
#define RT_DEVICE_FLAG_WRONLY           0x002           /**< @brief 只写 */
#define RT_DEVICE_FLAG_RDWR             0x003           /**< @brief 可读写 */

#define RT_DEVICE_FLAG_REMOVABLE        0x004           /**< @brief 可移除设备 */
#define RT_DEVICE_FLAG_STANDALONE       0x008           /**< @brief 独立设备 */
#define RT_DEVICE_FLAG_ACTIVATED        0x010           /**< @brief 设备已激活 */
#define RT_DEVICE_FLAG_SUSPENDED        0x020           /**< @brief 设备已挂起 */
#define RT_DEVICE_FLAG_STREAM           0x040           /**< @brief 流模式 */

#define RT_DEVICE_FLAG_INT_RX           0x100           /**< @brief 中断接收模式 */
#define RT_DEVICE_FLAG_DMA_RX           0x200           /**< @brief DMA 接收模式 */
#define RT_DEVICE_FLAG_INT_TX           0x400           /**< @brief 中断发送模式 */
#define RT_DEVICE_FLAG_DMA_TX           0x800           /**< @brief DMA 发送模式 */

#define RT_DEVICE_OFLAG_CLOSE           0x000           /**< @brief 设备已关闭 */
#define RT_DEVICE_OFLAG_RDONLY          0x001           /**< @brief 只读访问权限 */
#define RT_DEVICE_OFLAG_WRONLY          0x002           /**< @brief 只写访问权限 */
#define RT_DEVICE_OFLAG_RDWR            0x003           /**< @brief 可读写访问权限 */
#define RT_DEVICE_OFLAG_OPEN            0x008           /**< @brief 设备已打开 */
#define RT_DEVICE_OFLAG_MASK            0xf0f           /**< @brief 打开参数的掩码 */

/*
 * 通用设备命令
 */
#define RT_DEVICE_CTRL_RESUME           0x01            /**< @brief 设备恢复命令 */
#define RT_DEVICE_CTRL_SUSPEND          0x02            /**< @brief 设备挂起命令 */
#define RT_DEVICE_CTRL_CONFIG           0x03            /**< @brief 设备配置命令 */

#define RT_DEVICE_CTRL_SET_INT          0x10            /**< @brief 中断置位命令 */
#define RT_DEVICE_CTRL_CLR_INT          0x11            /**< @brief 中断清除命令 */
#define RT_DEVICE_CTRL_GET_INT          0x12            /**< @brief 获取中断状态命令 */

/*
 * 特殊设备命令
 */
#define RT_DEVICE_CTRL_CHAR_STREAM      0x10            /**< @brief 字符设备上的流模式 */
#define RT_DEVICE_CTRL_BLK_GETGEOME     0x10            /**< @brief 获取块设备信息   */
#define RT_DEVICE_CTRL_BLK_SYNC         0x11            /**< @brief 将数据刷新到块设备 */
#define RT_DEVICE_CTRL_BLK_ERASE        0x12            /**< @brief 擦除块设备上的存储块 */
#define RT_DEVICE_CTRL_BLK_AUTOREFRESH  0x13            /**< @brief 块设备：进入/退出自动刷新模式 */
#define RT_DEVICE_CTRL_NETIF_GETMAC     0x10            /**< @brief 获取mac地址 */
#define RT_DEVICE_CTRL_MTD_FORMAT       0x10            /**< @brief 格式化MTD设备 */
#define RT_DEVICE_CTRL_RTC_GET_TIME     0x10            /**< @brief 获取时间 */
#define RT_DEVICE_CTRL_RTC_SET_TIME     0x11            /**< @brief 设置时间 */
#define RT_DEVICE_CTRL_RTC_GET_ALARM    0x12            /**< @brief 获取闹钟信息 */
#define RT_DEVICE_CTRL_RTC_SET_ALARM    0x13            /**< @brief 设置闹钟 */

/**
 * @brief 设备类型指针定义
 */
typedef struct rt_device *rt_device_t;
/**
 * @brief 设备对象的操作方法
 */
struct rt_device_ops
{
    /* common device interface */
    rt_err_t  (*init)   (rt_device_t dev);			/**< @brief 初始化设备 */
    rt_err_t  (*open)   (rt_device_t dev, rt_uint16_t oflag);		/**< @brief 打开设备 */
    rt_err_t  (*close)  (rt_device_t dev);			/**< @brief 关闭设备 */
    rt_size_t (*read)   (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);		/**< @brief 读取设备数据 */
    rt_size_t (*write)  (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size);	/**< @brief 向设备写入数据 */
    rt_err_t  (*control)(rt_device_t dev, int cmd, void *args);		/**< @brief 控制设备 */
};



/**
 * @brief设备基类控制块
 */
struct rt_device
{
    struct rt_object          parent;                   /**< @brief 继承自 rt_object */

    enum rt_device_class_type type;                     /**< @brief 设备类型 */
    rt_uint16_t               flag;                     /**< @brief 设备标志 */
    rt_uint16_t               open_flag;                /**< @brief 设备打开标志 */

    rt_uint8_t                ref_count;                /**< @brief 引用次数 */
    rt_uint8_t                device_id;                /**< @brief 设备id,0 - 255 */

    /* device call back */
    rt_err_t (*rx_indicate)(rt_device_t dev, rt_size_t size); /**< @brief 设备接收数据回调 */
    rt_err_t (*tx_complete)(rt_device_t dev, void *buffer);  /**< @brief 设备发送数据回调 */

#ifdef RT_USING_DEVICE_OPS
    const struct rt_device_ops *ops;	/**< @brief 设备对象的操作方法 */
#else
    /* common device interface */
    rt_err_t  (*init)   (rt_device_t dev);
    rt_err_t  (*open)   (rt_device_t dev, rt_uint16_t oflag);
    rt_err_t  (*close)  (rt_device_t dev);
    rt_size_t (*read)   (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);
    rt_size_t (*write)  (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size);
    rt_err_t  (*control)(rt_device_t dev, int cmd, void *args);
#endif

#if defined(RT_USING_POSIX)
    const struct dfs_file_ops *fops;	/**< @brief 文件系统文件的操作方法 */
    struct rt_wqueue wait_queue;		/**< @brief 等待队列 */
#endif

    void                     *user_data;                /**< @brief 设备私有数据 */
};

/**
 * @brief 块设备扇区信息
 */
struct rt_device_blk_geometry
{
    rt_uint32_t sector_count;                           /**< @brief 扇区数量 */
    rt_uint32_t bytes_per_sector;                       /**< @brief 扇区字节数 */
    rt_uint32_t block_size;                             /**< @brief 擦除一个块的字节数 */
};

/**
 * @brief 块设备的扇区分布信息
 */
struct rt_device_blk_sectors
{
    rt_uint32_t sector_begin;                           /**< @brief 起始扇区 */
    rt_uint32_t sector_end;                             /**< @brief 结束扇区   */
};

/*
 * cursor control command
 */
#define RT_DEVICE_CTRL_CURSOR_SET_POSITION  0x10		/**< @brief 设置光标位置 */
#define RT_DEVICE_CTRL_CURSOR_SET_TYPE      0x11		/**< @brief 设置关闭类型 */

/*
 * graphic device control command
 */
#define RTGRAPHIC_CTRL_RECT_UPDATE      0				/**< @brief 更新设备图形 */
#define RTGRAPHIC_CTRL_POWERON          1				/**< @brief 打开设备电源 */
#define RTGRAPHIC_CTRL_POWEROFF         2				/**< @brief 关闭设备电源 */
#define RTGRAPHIC_CTRL_GET_INFO         3				/**< @brief 获取设备信息 */
#define RTGRAPHIC_CTRL_SET_MODE         4				/**< @brief 设置工作模式 */
#define RTGRAPHIC_CTRL_GET_EXT          5				/**< @brief 获取 */

/* 图形设备像素格式 */
enum
{
    RTGRAPHIC_PIXEL_FORMAT_MONO = 0,
    RTGRAPHIC_PIXEL_FORMAT_GRAY4,
    RTGRAPHIC_PIXEL_FORMAT_GRAY16,
    RTGRAPHIC_PIXEL_FORMAT_RGB332,
    RTGRAPHIC_PIXEL_FORMAT_RGB444,
    RTGRAPHIC_PIXEL_FORMAT_RGB565,
    RTGRAPHIC_PIXEL_FORMAT_RGB565P,
    RTGRAPHIC_PIXEL_FORMAT_BGR565 = RTGRAPHIC_PIXEL_FORMAT_RGB565P,
    RTGRAPHIC_PIXEL_FORMAT_RGB666,
    RTGRAPHIC_PIXEL_FORMAT_RGB888,
    RTGRAPHIC_PIXEL_FORMAT_ARGB888,
    RTGRAPHIC_PIXEL_FORMAT_ABGR888,
    RTGRAPHIC_PIXEL_FORMAT_ARGB565,
    RTGRAPHIC_PIXEL_FORMAT_ALPHA,
};

#define RTGRAPHIC_PIXEL_POSITION(x, y)  ((x << 16) | y)		/**< @brief 根据（x，y）坐标构建像素位置 */

/**
 * @brief 图形设备信息结构
 */
struct rt_device_graphic_info
{
    rt_uint8_t  pixel_format;                           /**< @brief 图形格式 */
    rt_uint8_t  bits_per_pixel;                         /**< @brief 像素位数 */
    rt_uint16_t reserved;                               /**< @brief 保留字段 */

    rt_uint16_t width;                                  /**< @brief 图形设备的宽度 */
    rt_uint16_t height;                                 /**< @brief 图形设备的高度 */

    rt_uint8_t *framebuffer;                            /**< @brief 帧缓冲区 */
};

/**
 * @brief 矩形信息结构
 */
struct rt_device_rect_info
{
    rt_uint16_t x;                                      /**< @brief x坐标 */
    rt_uint16_t y;                                      /**< @brief y坐标 */
    rt_uint16_t width;                                  /**< @brief 宽度 */
    rt_uint16_t height;                                 /**< @brief 高度 */
};

/*
 * @brief 图形设备的操作方法
 */
struct rt_device_graphic_ops
{
    void (*set_pixel) (const char *pixel, int x, int y);			/**< @brief 设置像素点颜色 */
    void (*get_pixel) (char *pixel, int x, int y);					/**< @brief 获取像素点颜色 */

    void (*draw_hline)(const char *pixel, int x1, int x2, int y);	/**< @brief 画水平线 */
    void (*draw_vline)(const char *pixel, int x, int y1, int y2);	/**< @brief 画垂直线 */

    void (*blit_line) (const char *pixel, int x, int y, rt_size_t size);
};
#define rt_graphix_ops(device)          ((struct rt_device_graphic_ops *)(device->user_data))

/*@}*/
#endif

#ifdef RT_USING_MODULE
/*
 * @addtogroup Module
 */

/*@{*/

/*
 * module system
 */

#define RT_MODULE_FLAG_WITHENTRY        0x00            /**< @brief 有入口点 */
#define RT_MODULE_FLAG_WITHOUTENTRY     0x01            /**< @brief 无入口点 */

/*
 * @brief 动态模块控制块
 */
struct rt_module
{
    struct rt_object             parent;                /**< @brief 继承自 rt_object */

    rt_uint32_t                  vstart_addr;           /**< @brief 第一个LOAD段的VMA基址 */
    rt_uint8_t                  *module_space;          /**< @brief 动态模块的内存空间 */

    void                        *module_entry;          /**< @brief 动态模块的入口地址 */
    rt_thread_t                  module_thread;         /**< @brief 动态模块的主线程 */

    rt_uint8_t                  *module_cmd_line;       /**< @brief 动态模块的命令行 */
    rt_uint32_t                  module_cmd_size;       /**< @brief 动态模块命令行的大小 */

#ifdef RT_USING_SLAB
    /* module memory allocator */
    void                        *mem_list;              /**< @brief 动态模块的空闲内存列表 */
    void                        *page_array;            /**< @brief 动态模块的使用页 */
    rt_uint32_t                  page_cnt;              /**< @brief 动态模块的使用页数 */
#endif

    rt_uint16_t                  nref;                  /**< @brief 引用次数 */

    rt_uint16_t                  nsym;                  /**< @brief 动态模块的变量数 */
    struct rt_module_symtab     *symtab;                /**< @brief 动态模块的变量表 */

    rt_uint32_t                  user_data;             /**< @brief 动态模块的私有数据 */

    void (*module_init)(void);							/**< @brief 模块的初始化函数指针 */
    void (*module_cleanup)(void);						/**< @brief 模块的清除函数指针 */
};
/*
 * @brief 动态模块类型指针定义
 */
typedef struct rt_module *rt_module_t;

/*@}*/
#endif

/* definitions for libc */
#include "rtlibc.h"

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
/* RT-Thread definitions for C++ */
namespace rtthread {

enum TICK_WAIT {
    WAIT_NONE = 0,
    WAIT_FOREVER = -1,
};

}

#endif /* end of __cplusplus */

#endif
