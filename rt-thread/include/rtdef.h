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

/* RT-Thread �汾��Ϣ */
#define RT_VERSION                      3L              /**< @brief ���汾�� */
#define RT_SUBVERSION                   1L              /**< @brief �ΰ汾�� */
#define RT_REVISION                     0L              /**< @brief �޶��汾�� */

/* RT-Thread �汾 */
#define RTTHREAD_VERSION                ((RT_VERSION * 10000) + \
                                         (RT_SUBVERSION * 100) + RT_REVISION)

/* RT-Thread basic data type definitions */
typedef signed   char                   rt_int8_t;      /**< @brief  8λ�������� */
typedef signed   short                  rt_int16_t;     /**< @brief 16λ�������� */
typedef signed   long                   rt_int32_t;     /**< @brief 32λ�������� */
typedef unsigned char                   rt_uint8_t;     /**< @brief  8λ�޷����������� */
typedef unsigned short                  rt_uint16_t;    /**< @brief 16λ�޷����������� */
typedef unsigned long                   rt_uint32_t;    /**< @brief 32λ�޷����������� */
typedef int                             rt_bool_t;      /**< @brief �������� */

/* 32bit CPU */
typedef long                            rt_base_t;      /**< @brief NλCPU ��س��������� */
typedef unsigned long                   rt_ubase_t;     /**< @brief NλCPU ����޷��ų��������� */

typedef rt_base_t                       rt_err_t;       /**< @brief ��������� */
typedef rt_uint32_t                     rt_time_t;      /**< @brief ʱ������� */
typedef rt_uint32_t                     rt_tick_t;      /**< @brief ϵͳʱ�ӽ��ļ������� */
typedef rt_base_t                       rt_flag_t;      /**< @brief �������� */
typedef rt_ubase_t                      rt_size_t;      /**< @brief �ֽڴ�С���� */
typedef rt_ubase_t                      rt_dev_t;       /**< @brief �豸���� */
typedef rt_base_t                       rt_off_t;       /**< @brief ����ƫ������ */

/* boolean type definitions */
#define RT_TRUE                         1               /**< @brief ����ֵ ��  */
#define RT_FALSE                        0               /**< @brief ����ֵ �� */


/* maximum value of base type */
#define RT_UINT8_MAX                    0xff            /**< @brief UINT8 �����ֵ */
#define RT_UINT16_MAX                   0xffff          /**< @brief UINT16 �����ֵ */
#define RT_UINT32_MAX                   0xffffffff      /**< @brief UINT32 �����ֵ */
#define RT_TICK_MAX                     RT_UINT32_MAX   /**< @brief tick �����ֵ */

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

#define INIT_BOARD_EXPORT(fn)           INIT_EXPORT(fn, "1")	/**< @brief �弶������ʼ����fnΪ����ָ�� */

/* pre/device/component/env/app init routines will be called in init_thread */
/* components pre-initialization (pure software initilization) */

#define INIT_PREV_EXPORT(fn)            INIT_EXPORT(fn, "2")	/**< @brief ����������Զ���ʼ����fnΪ����ָ�� */
/* device initialization */

#define INIT_DEVICE_EXPORT(fn)          INIT_EXPORT(fn, "3")	/**< @brief �豸�Զ���ʼ����fnΪ����ָ�� */
/* components initialization (dfs, lwip, ...) */

#define INIT_COMPONENT_EXPORT(fn)       INIT_EXPORT(fn, "4")	/**< @brief ����Զ���ʼ����fnΪ����ָ�� */
/* environment initialization (mount disk, ...) */

#define INIT_ENV_EXPORT(fn)             INIT_EXPORT(fn, "5")	/**< @brief ϵͳ�����Զ���ʼ����fnΪ����ָ�� */
/* appliation initialization (rtgui application etc ...) */

#define INIT_APP_EXPORT(fn)             INIT_EXPORT(fn, "6")	/**< @brief Ӧ���Զ���ʼ����fnΪ����ָ�� */

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
#define RT_EOK                          0               /**< @brief �޴��� */
#define RT_ERROR                        1               /**< @brief һ����� */
#define RT_ETIMEOUT                     2               /**< @brief ��ʱ���� */
#define RT_EFULL                        3               /**< @brief ��Դ���� */
#define RT_EEMPTY                       4               /**< @brief ��Դ�ѿ� */
#define RT_ENOMEM                       5               /**< @brief �ڴ治�� */
#define RT_ENOSYS                       6               /**< @brief ��ϵͳ */
#define RT_EBUSY                        7               /**< @brief æ���� */
#define RT_EIO                          8               /**< @brief IO ���� */
#define RT_EINTR                        9               /**< @brief �ж�ϵͳ���� */
#define RT_EINVAL                       10              /**< @brief ��Ч���� */

/*@}*/

/**
 * @addtogroup BasicDef
 */

/*@{*/
#define RT_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1)) /**< @brief ������ָ����ȶ������������С������ RT_ALIGN(13, 4)������16 */

#define RT_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1)) /**< @brief ����ָ����ȶ���������������� RT_ALIGN_DOWN(13, 4)������12 */

#define RT_NULL                         (0) /**< @brief �൱��C���е�NULL */

/*@}*/

/**
 * @ingroup list
 *
 * @brief ˫������ڵ�
 */
struct rt_list_node
{
    struct rt_list_node *next;                          /**< @brief ָ����һ���ڵ㡣 */
    struct rt_list_node *prev;                          /**< @brief ָ����һ���ڵ㡣 */
};
/**
 * @ingroup list
 */
typedef struct rt_list_node rt_list_t;                  /**< @brief ˫���������Ͷ��� */

/**
 * @ingroup slist
 *
 * @brief ��������ڵ�
 */
struct rt_slist_node
{
    struct rt_slist_node *next;                         /**< @brief ָ����һ���ڵ㡣 */
};
/**
 * @ingroup slist
 *
 * @brief ��������ڵ����Ͷ���
 */
typedef struct rt_slist_node rt_slist_t;                /**< @brief �����������Ͷ��� */

/**
 * @addtogroup KernelObject
 */

/*@{*/

/*
 * kernel object macros
 */
#define RT_OBJECT_FLAG_MODULE           0x80            /**< @brief ��̬ģ������־ */

/**
 * @brief �ں˶��������ƿ�
 */
struct rt_object
{
    char       name[RT_NAME_MAX];                       /**< @brief �ں˶�������� */
    rt_uint8_t type;                                    /**< @brief �ں˶�������� */
    rt_uint8_t flag;                                    /**< @brief �ں˶���Ĳ��� */

#ifdef RT_USING_MODULE
    void      *module_id;                               /**< @brief ��̬ģ���id */
#endif
    rt_list_t  list;                                    /**< @brief �ں˶���������� */
};

typedef struct rt_object *rt_object_t;                  /**< @brief �ں˶�������ָ�붨�� */

/**
 * @brief �ں˶�������
 */
enum rt_object_class_type
{
    RT_Object_Class_Thread = 0,                         /**< @brief �̶߳��� */
    RT_Object_Class_Semaphore,                          /**< @brief �ź������� */
    RT_Object_Class_Mutex,                              /**< @brief ���������� */
    RT_Object_Class_Event,                              /**< @brief �¼����� */
    RT_Object_Class_MailBox,                            /**< @brief ������� */
    RT_Object_Class_MessageQueue,                       /**< @brief ��Ϣ���ж��� */
    RT_Object_Class_MemHeap,                            /**< @brief �ڴ�Ѷ��� */
    RT_Object_Class_MemPool,                            /**< @brief �ڴ�ض��� */
    RT_Object_Class_Device,                             /**< @brief �豸���� */
    RT_Object_Class_Timer,                              /**< @brief ��ʱ������ */
    RT_Object_Class_Module,                             /**< @brief ��̬ģ����� */
    RT_Object_Class_Unknown,                            /**< @brief δ֪���� */
    RT_Object_Class_Static = 0x80                       /**< @brief ��̬���� */
};

/**
 * @brief �ں˶�����Ϣ
 */
struct rt_object_information
{
    enum rt_object_class_type type;                     /**< @brief �������� */
    rt_list_t                 object_list;              /**< @brief �������� */
    rt_size_t                 object_size;              /**< @brief �����С */
};

/*@}*/

/**
 * @addtogroup Hook
 */

/**@{*/

#ifdef RT_USING_HOOK
#define RT_OBJECT_HOOK_CALL(func, argv) \
    do { if ((func) != RT_NULL) func argv; } while (0)	/**< @brief �ں˶����Ӻ��� */
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
#define RT_TIMER_FLAG_DEACTIVATED       0x0             /**< @brief ��ʱ��δ���� */
#define RT_TIMER_FLAG_ACTIVATED         0x1             /**< @brief ��ʱ���Ѽ��� */
#define RT_TIMER_FLAG_ONE_SHOT          0x0             /**< @brief һ���Լ�ʱ�� */
#define RT_TIMER_FLAG_PERIODIC          0x2             /**< @brief �����Զ�ʱ�� */

#define RT_TIMER_FLAG_HARD_TIMER        0x0             /**< @brief Ӳ����ʱ������ʱ���Ļص���������ϵͳʱ���ж�����á� */
#define RT_TIMER_FLAG_SOFT_TIMER        0x4             /**< @brief �����ʱ������ʱ���Ļص��������ڶ�ʱ���߳��е��á� */

#define RT_TIMER_CTRL_SET_TIME          0x0             /**< @brief ���ö�ʱ��ʱ�� */
#define RT_TIMER_CTRL_GET_TIME          0x1             /**< @brief ��ȡ��ʱ��ʱ�� */
#define RT_TIMER_CTRL_SET_ONESHOT       0x2             /**< @brief ����ʱ������Ϊһ���Զ�ʱ�� */
#define RT_TIMER_CTRL_SET_PERIODIC      0x3             /**< @brief ����ʱ������Ϊ�����Զ�ʱ�� */

#ifndef RT_TIMER_SKIP_LIST_LEVEL
#define RT_TIMER_SKIP_LIST_LEVEL          1
#endif

/* 1 or 3 */
#ifndef RT_TIMER_SKIP_LIST_MASK
#define RT_TIMER_SKIP_LIST_MASK         0x3
#endif

/**
 * @brief ��ʱ�����ƿ�
 */
struct rt_timer
{
    struct rt_object parent;                            /**< @brief �� rt_object �̳� */

    rt_list_t        row[RT_TIMER_SKIP_LIST_LEVEL];

    void (*timeout_func)(void *parameter);              /**< @brief ��ʱ�ص����� */
    void            *parameter;                         /**< @brief ��ʱ�ص������Ĵ������ */

    rt_tick_t        init_tick;                         /**< @brief ��ʱ����ʼʱ�ӽ�����*/
    rt_tick_t        timeout_tick;                      /**< @brief ��ʱ����ʱʱ�ӽ�����*/
};
/**
 * @brief ��ʱ������ָ�붨��
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
 * �߳�״̬����
 */
#define RT_THREAD_INIT                  0x00                /**< @brief ��ʼ�����״̬ */
#define RT_THREAD_READY                 0x01                /**< @brief ����״̬ */
#define RT_THREAD_SUSPEND               0x02                /**< @brief ����״̬ */
#define RT_THREAD_RUNNING               0x03                /**< @brief ����״̬ */
#define RT_THREAD_BLOCK                 RT_THREAD_SUSPEND   /**< @brief ����״̬ */
#define RT_THREAD_CLOSE                 0x04                /**< @brief �ر�״̬ */
#define RT_THREAD_STAT_MASK             0x0f

#define RT_THREAD_STAT_SIGNAL           0x10
#define RT_THREAD_STAT_SIGNAL_READY     (RT_THREAD_STAT_SIGNAL | RT_THREAD_READY)
#define RT_THREAD_STAT_SIGNAL_WAIT      0x20
#define RT_THREAD_STAT_SIGNAL_MASK      0xf0

/*
 * �߳̿��������
 */
#define RT_THREAD_CTRL_STARTUP          0x00                /**< @brief �����߳�����. */
#define RT_THREAD_CTRL_CLOSE            0x01                /**< @brief �ر��߳�����. */
#define RT_THREAD_CTRL_CHANGE_PRIORITY  0x02                /**< @brief �ı��߳����ȼ�����. */
#define RT_THREAD_CTRL_INFO             0x03                /**< @brief ��ȡ�߳���Ϣ����. */

/**
 * @brief �߳̿��ƿ�
 */
struct rt_thread
{
    /* rt object */
    char        name[RT_NAME_MAX];                      /**< @brief �߳����� */
    rt_uint8_t  type;                                   /**< @brief �������� */
    rt_uint8_t  flags;                                  /**< @brief �̵߳Ĳ��� */

#ifdef RT_USING_MODULE
    void       *module_id;                              /**< @brief ��̬ģ��ID */
#endif

    rt_list_t   list;                                   /**< @brief ������� */
    rt_list_t   tlist;                                  /**< @brief �߳����� */

    /* stack point and entry */
    void       *sp;                                     /**< @brief ջָ�� */
    void       *entry;                                  /**< @brief ��ں���ָ�� */
    void       *parameter;                              /**< @brief �߳���ڲ��� */
    void       *stack_addr;                             /**< @brief ��ջ��ַ */
    rt_uint32_t stack_size;                             /**< @brief ��ջ��С */

    /* error code */
    rt_err_t    error;                                  /**< @brief ������� */

    rt_uint8_t  stat;                                   /**< @brief �߳�״̬ */

    /* priority */
    rt_uint8_t  current_priority;                       /**< @brief ��ǰ���ȼ� */
    rt_uint8_t  init_priority;                          /**< @brief ��ʼ�����ȼ� */
#if RT_THREAD_PRIORITY_MAX > 32
    rt_uint8_t  number;
    rt_uint8_t  high_mask;
#endif
    rt_uint32_t number_mask;

#if defined(RT_USING_EVENT)
    /* thread event */
    rt_uint32_t event_set;								/**< @brief �̵߳ȴ����¼��� */
    rt_uint8_t  event_info;								/**< @brief �̵߳ȴ����¼���־ and/or/clear */
#endif

#if defined(RT_USING_SIGNALS)
    rt_sigset_t     sig_pending;                        /**< @brief ��������ź� */
    rt_sigset_t     sig_mask;                           /**< @brief �źŵ�����λ */

    void            *sig_ret;                           /**< @brief �źŷ��صĶ�ջָ�� */
    rt_sighandler_t *sig_vectors;                       /**< @brief �źŴ�������� */
    void            *si_list;                           /**< @brief �ź��б� */
#endif

    rt_ubase_t  init_tick;                              /**< @brief �̵߳ĳ�ʼ��ʱ�ӽ��� */
    rt_ubase_t  remaining_tick;                         /**< @brief ʣ��ʱ�ӽ����� */

    struct rt_timer thread_timer;                       /**< @brief �̵߳����ü�ʱ�� */

    void (*cleanup)(struct rt_thread *tid);             /**< @brief �߳��˳�ʱ������ص����� */

    rt_uint32_t user_data;                              /**< @brief �̵߳�˽���û����� */
};
/**
 * @brief �߳�����ָ�붨��
 */
typedef struct rt_thread *rt_thread_t;

/*@}*/

/**
 * @addtogroup IPC
 */

/*@{*/

/*
 * IPC��־�Ϳ��������
 */
#define RT_IPC_FLAG_FIFO                0x00            /**< @brief �Ƚ��ȳ�ģʽ*/
#define RT_IPC_FLAG_PRIO                0x01            /**< @brief ���ȼ�ģʽ */

#define RT_IPC_CMD_UNKNOWN              0x00            /**< @brief δ֪�� IPC ���� */
#define RT_IPC_CMD_RESET                0x01            /**< @brief ��λ IPC �������� */

#define RT_WAITING_FOREVER              -1              /**< @brief ��Զ����ֱ�������Դ */
#define RT_WAITING_NO                   0               /**< @brief ������ */

/**
 * @brief IPC������ƿ�
 */
struct rt_ipc_object
{
    struct rt_object parent;                            /**< @brief �� rt_object �̳� */

    rt_list_t        suspend_thread;                    /**< @brief �����߳����� */
};

/*@}*/
#ifdef RT_USING_SEMAPHORE

/**
 * @addtogroup semaphore
 */

/**@{*/

/**
 * @brief �ź������ƿ�
 */
struct rt_semaphore
{
    struct rt_ipc_object parent;                        /**< @brief �̳��� rt_ipc_object */

    rt_uint16_t          value;                         /**< @brief �ź�����ֵ */
};
/**
 * @brief �ź�������ָ�붨��
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
 * @brief ���������ƿ�
 */
struct rt_mutex
{
    struct rt_ipc_object parent;                        /**< @brief �̳��� rt_ipc_object */

    rt_uint16_t          value;                         /**< @brief ��������ֵ */

    rt_uint8_t           original_priority;             /**< @brief ���һ��ִ�иû��������̵߳����ȼ� */
    rt_uint8_t           hold;                          /**< @brief �̳߳��иû������Ĵ��� */

    struct rt_thread    *owner;                         /**< @brief ��ǰ�û������ĳ����ߣ��̣߳� */
};
/**
 * @brief ����������ָ�붨��
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
 * �¼�����
 */
#define RT_EVENT_FLAG_AND               0x01            /**< @brief �߼������ */
#define RT_EVENT_FLAG_OR                0x02            /**< @brief �߼������ */
#define RT_EVENT_FLAG_CLEAR             0x04            /**< @brief ������� */

/**
 * @brief �¼����ƿ�
 */
struct rt_event
{
    struct rt_ipc_object parent;                        /**< @brief �̳���rt_ipc_object */

    rt_uint32_t          set;                           /**< @brief ʱ�伯 */
};
/**
 * @brief �¼�����ָ�붨��
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
 * @brief ������ƿ�
 */
struct rt_mailbox
{
    struct rt_ipc_object parent;                        /**< @brief �̳��� rt_ipc_object */

    rt_uint32_t         *msg_pool;                      /**< @brief ��Ϣ����������ʼ��ַ */

    rt_uint16_t          size;                          /**< @brief ��Ϣ�صĴ�С */

    rt_uint16_t          entry;                         /**< @brief msg_pool�е���Ϣ���� */
    rt_uint16_t          in_offset;                     /**< @brief ��Ϣ������������ƫ���� */
    rt_uint16_t          out_offset;                    /**< @brief ��Ϣ�����������ƫ���� */

    rt_list_t            suspend_sender_thread;         /**< @brief �����ڸ������ϵķ����߳� */
};
/**
 * @brief ��������ָ�붨��
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
 * @brief ��Ϣ���п��ƿ�
 */
struct rt_messagequeue
{
    struct rt_ipc_object parent;                        /**< @brief �̳���rt_ipc_object */

    void                *msg_pool;                      /**< @brief ��Ϣ���е���ʼ��ַ */

    rt_uint16_t          msg_size;                      /**< @brief ÿ����Ϣ����Ϣ��С */
    rt_uint16_t          max_msgs;                      /**< @brief �����Ϣ�� */

    rt_uint16_t          entry;                         /**< @brief �����е���Ϣ���� */

    void                *msg_queue_head;                /**< @brief ����ͷ */
    void                *msg_queue_tail;                /**< @brief ����β */
    void                *msg_queue_free;                /**< @brief ָ������еĿ��нڵ��ָ�� */
};
/**
 * @brief ��Ϣ��������ָ�붨��
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
 * @brief �ڴ�ѹ�����ƿ�
 */
struct rt_memheap_item
{
    rt_uint32_t             magic;                      /**< @brief �ڴ�ѵĻ��� */
    struct rt_memheap      *pool_ptr;                   /**< @brief �ڴ�ص�ָ�� */

    struct rt_memheap_item *next;                       /**< @brief ��һ���ڴ�� */
    struct rt_memheap_item *prev;                       /**< @brief ��һ���ڴ�� */

    struct rt_memheap_item *next_free;                  /**< @brief ��һ�������ڴ�� */
    struct rt_memheap_item *prev_free;                  /**< @brief ��һ�������ڴ�� */
};

/**
 * @brief �ڴ�ѿ��ƿ�
 */
struct rt_memheap
{
    struct rt_object        parent;                     /**< @brief �̳��� rt_object */

    void                   *start_addr;                 /**< @brief �ڴ�ѵ���ʼ��ַ */

    rt_uint32_t             pool_size;                  /**< @brief �ڴ�ѵĴ�С */
    rt_uint32_t             available_size;             /**< @brief �ڴ�ѿ��ô�С */
    rt_uint32_t             max_used_size;              /**< @brief ���ɷ����С */

    struct rt_memheap_item *block_list;                 /**< @brief ��ʹ�õĿ�����*/

    struct rt_memheap_item *free_list;                  /**< @brief δʹ�õĿ����� */
    struct rt_memheap_item  free_header;                /**< @brief δʹ�õĿ������ͷ */

    struct rt_semaphore     lock;                       /**< @brief �ź����� */
};

/*@}*/
#endif


/**
 * @addtogroup Pool
 */

/*@{*/

#ifdef RT_USING_MEMPOOL
/**
 * @brief �ڴ�ؿ��ƿ�
 */
struct rt_mempool
{
    struct rt_object parent;                            /**< @brief �̳��� rt_object */

    void            *start_address;                     /**< @brief �ڴ�ص���ʼ��ַ */
    rt_size_t        size;                              /**< @brief �ڴ�صĴ�С */

    rt_size_t        block_size;                        /**< @brief �ڴ��Ĵ�С */
    rt_uint8_t      *block_list;                        /**< @brief �ڴ������ */

    rt_size_t        block_total_count;                 /**< @brief �ڴ��������� */
    rt_size_t        block_free_count;                  /**< @brief δʹ���ڴ������� */

    rt_list_t        suspend_thread;                    /**< @brief �����߳����� */
    rt_size_t        suspend_thread_count;              /**< @brief �����̵߳����� */
};
/**
 * @brief �ڴ������ָ�붨��
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
 * (I/O)�豸����
 */
enum rt_device_class_type
{
    RT_Device_Class_Char = 0,                           /**< �ַ��豸 */
    RT_Device_Class_Block,                              /**< ���豸 */
    RT_Device_Class_NetIf,                              /**< ����ӿ��豸 */
    RT_Device_Class_MTD,                                /**< �洢�豸 */
    RT_Device_Class_CAN,                                /**< CAN �豸 */
    RT_Device_Class_RTC,                                /**< RTC �豸 */
    RT_Device_Class_Sound,                              /**< ��Ƶ�豸 */
    RT_Device_Class_Graphic,                            /**< ͼ���豸 */
    RT_Device_Class_I2CBUS,                             /**< I2C �����豸 */
    RT_Device_Class_USBDevice,                          /**< USB���豸 */
    RT_Device_Class_USBHost,                            /**< USB�������� */
    RT_Device_Class_SPIBUS,                             /**< SPI�����豸 */
    RT_Device_Class_SPIDevice,                          /**< SPI ���豸 */
    RT_Device_Class_SDIO,                               /**< SDIO �豸 */
    RT_Device_Class_PM,                                 /**< ��Դ�����豸 */
    RT_Device_Class_Pipe,                               /**< �ܵ��豸 */
    RT_Device_Class_Portal,                             /**< ˫��ܵ��豸 */
    RT_Device_Class_Timer,                              /**< ��ʱ���豸 */
    RT_Device_Class_Miscellaneous,                      /**< �����豸 */
    RT_Device_Class_Unknown                             /**< δ֪�豸 */
};

/*
 * �豸��־����
 */
#define RT_DEVICE_FLAG_DEACTIVATE       0x000           /**< @brief �豸δ��ʼ�� */

#define RT_DEVICE_FLAG_RDONLY           0x001           /**< @brief ֻ�� */
#define RT_DEVICE_FLAG_WRONLY           0x002           /**< @brief ֻд */
#define RT_DEVICE_FLAG_RDWR             0x003           /**< @brief �ɶ�д */

#define RT_DEVICE_FLAG_REMOVABLE        0x004           /**< @brief ���Ƴ��豸 */
#define RT_DEVICE_FLAG_STANDALONE       0x008           /**< @brief �����豸 */
#define RT_DEVICE_FLAG_ACTIVATED        0x010           /**< @brief �豸�Ѽ��� */
#define RT_DEVICE_FLAG_SUSPENDED        0x020           /**< @brief �豸�ѹ��� */
#define RT_DEVICE_FLAG_STREAM           0x040           /**< @brief ��ģʽ */

#define RT_DEVICE_FLAG_INT_RX           0x100           /**< @brief �жϽ���ģʽ */
#define RT_DEVICE_FLAG_DMA_RX           0x200           /**< @brief DMA ����ģʽ */
#define RT_DEVICE_FLAG_INT_TX           0x400           /**< @brief �жϷ���ģʽ */
#define RT_DEVICE_FLAG_DMA_TX           0x800           /**< @brief DMA ����ģʽ */

#define RT_DEVICE_OFLAG_CLOSE           0x000           /**< @brief �豸�ѹر� */
#define RT_DEVICE_OFLAG_RDONLY          0x001           /**< @brief ֻ������Ȩ�� */
#define RT_DEVICE_OFLAG_WRONLY          0x002           /**< @brief ֻд����Ȩ�� */
#define RT_DEVICE_OFLAG_RDWR            0x003           /**< @brief �ɶ�д����Ȩ�� */
#define RT_DEVICE_OFLAG_OPEN            0x008           /**< @brief �豸�Ѵ� */
#define RT_DEVICE_OFLAG_MASK            0xf0f           /**< @brief �򿪲��������� */

/*
 * ͨ���豸����
 */
#define RT_DEVICE_CTRL_RESUME           0x01            /**< @brief �豸�ָ����� */
#define RT_DEVICE_CTRL_SUSPEND          0x02            /**< @brief �豸�������� */
#define RT_DEVICE_CTRL_CONFIG           0x03            /**< @brief �豸�������� */

#define RT_DEVICE_CTRL_SET_INT          0x10            /**< @brief �ж���λ���� */
#define RT_DEVICE_CTRL_CLR_INT          0x11            /**< @brief �ж�������� */
#define RT_DEVICE_CTRL_GET_INT          0x12            /**< @brief ��ȡ�ж�״̬���� */

/*
 * �����豸����
 */
#define RT_DEVICE_CTRL_CHAR_STREAM      0x10            /**< @brief �ַ��豸�ϵ���ģʽ */
#define RT_DEVICE_CTRL_BLK_GETGEOME     0x10            /**< @brief ��ȡ���豸��Ϣ   */
#define RT_DEVICE_CTRL_BLK_SYNC         0x11            /**< @brief ������ˢ�µ����豸 */
#define RT_DEVICE_CTRL_BLK_ERASE        0x12            /**< @brief �������豸�ϵĴ洢�� */
#define RT_DEVICE_CTRL_BLK_AUTOREFRESH  0x13            /**< @brief ���豸������/�˳��Զ�ˢ��ģʽ */
#define RT_DEVICE_CTRL_NETIF_GETMAC     0x10            /**< @brief ��ȡmac��ַ */
#define RT_DEVICE_CTRL_MTD_FORMAT       0x10            /**< @brief ��ʽ��MTD�豸 */
#define RT_DEVICE_CTRL_RTC_GET_TIME     0x10            /**< @brief ��ȡʱ�� */
#define RT_DEVICE_CTRL_RTC_SET_TIME     0x11            /**< @brief ����ʱ�� */
#define RT_DEVICE_CTRL_RTC_GET_ALARM    0x12            /**< @brief ��ȡ������Ϣ */
#define RT_DEVICE_CTRL_RTC_SET_ALARM    0x13            /**< @brief �������� */

/**
 * @brief �豸����ָ�붨��
 */
typedef struct rt_device *rt_device_t;
/**
 * @brief �豸����Ĳ�������
 */
struct rt_device_ops
{
    /* common device interface */
    rt_err_t  (*init)   (rt_device_t dev);			/**< @brief ��ʼ���豸 */
    rt_err_t  (*open)   (rt_device_t dev, rt_uint16_t oflag);		/**< @brief ���豸 */
    rt_err_t  (*close)  (rt_device_t dev);			/**< @brief �ر��豸 */
    rt_size_t (*read)   (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);		/**< @brief ��ȡ�豸���� */
    rt_size_t (*write)  (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size);	/**< @brief ���豸д������ */
    rt_err_t  (*control)(rt_device_t dev, int cmd, void *args);		/**< @brief �����豸 */
};



/**
 * @brief�豸������ƿ�
 */
struct rt_device
{
    struct rt_object          parent;                   /**< @brief �̳��� rt_object */

    enum rt_device_class_type type;                     /**< @brief �豸���� */
    rt_uint16_t               flag;                     /**< @brief �豸��־ */
    rt_uint16_t               open_flag;                /**< @brief �豸�򿪱�־ */

    rt_uint8_t                ref_count;                /**< @brief ���ô��� */
    rt_uint8_t                device_id;                /**< @brief �豸id,0 - 255 */

    /* device call back */
    rt_err_t (*rx_indicate)(rt_device_t dev, rt_size_t size); /**< @brief �豸�������ݻص� */
    rt_err_t (*tx_complete)(rt_device_t dev, void *buffer);  /**< @brief �豸�������ݻص� */

#ifdef RT_USING_DEVICE_OPS
    const struct rt_device_ops *ops;	/**< @brief �豸����Ĳ������� */
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
    const struct dfs_file_ops *fops;	/**< @brief �ļ�ϵͳ�ļ��Ĳ������� */
    struct rt_wqueue wait_queue;		/**< @brief �ȴ����� */
#endif

    void                     *user_data;                /**< @brief �豸˽������ */
};

/**
 * @brief ���豸������Ϣ
 */
struct rt_device_blk_geometry
{
    rt_uint32_t sector_count;                           /**< @brief �������� */
    rt_uint32_t bytes_per_sector;                       /**< @brief �����ֽ��� */
    rt_uint32_t block_size;                             /**< @brief ����һ������ֽ��� */
};

/**
 * @brief ���豸�������ֲ���Ϣ
 */
struct rt_device_blk_sectors
{
    rt_uint32_t sector_begin;                           /**< @brief ��ʼ���� */
    rt_uint32_t sector_end;                             /**< @brief ��������   */
};

/*
 * cursor control command
 */
#define RT_DEVICE_CTRL_CURSOR_SET_POSITION  0x10		/**< @brief ���ù��λ�� */
#define RT_DEVICE_CTRL_CURSOR_SET_TYPE      0x11		/**< @brief ���ùر����� */

/*
 * graphic device control command
 */
#define RTGRAPHIC_CTRL_RECT_UPDATE      0				/**< @brief �����豸ͼ�� */
#define RTGRAPHIC_CTRL_POWERON          1				/**< @brief ���豸��Դ */
#define RTGRAPHIC_CTRL_POWEROFF         2				/**< @brief �ر��豸��Դ */
#define RTGRAPHIC_CTRL_GET_INFO         3				/**< @brief ��ȡ�豸��Ϣ */
#define RTGRAPHIC_CTRL_SET_MODE         4				/**< @brief ���ù���ģʽ */
#define RTGRAPHIC_CTRL_GET_EXT          5				/**< @brief ��ȡ */

/* ͼ���豸���ظ�ʽ */
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

#define RTGRAPHIC_PIXEL_POSITION(x, y)  ((x << 16) | y)		/**< @brief ���ݣ�x��y�����깹������λ�� */

/**
 * @brief ͼ���豸��Ϣ�ṹ
 */
struct rt_device_graphic_info
{
    rt_uint8_t  pixel_format;                           /**< @brief ͼ�θ�ʽ */
    rt_uint8_t  bits_per_pixel;                         /**< @brief ����λ�� */
    rt_uint16_t reserved;                               /**< @brief �����ֶ� */

    rt_uint16_t width;                                  /**< @brief ͼ���豸�Ŀ�� */
    rt_uint16_t height;                                 /**< @brief ͼ���豸�ĸ߶� */

    rt_uint8_t *framebuffer;                            /**< @brief ֡������ */
};

/**
 * @brief ������Ϣ�ṹ
 */
struct rt_device_rect_info
{
    rt_uint16_t x;                                      /**< @brief x���� */
    rt_uint16_t y;                                      /**< @brief y���� */
    rt_uint16_t width;                                  /**< @brief ��� */
    rt_uint16_t height;                                 /**< @brief �߶� */
};

/*
 * @brief ͼ���豸�Ĳ�������
 */
struct rt_device_graphic_ops
{
    void (*set_pixel) (const char *pixel, int x, int y);			/**< @brief �������ص���ɫ */
    void (*get_pixel) (char *pixel, int x, int y);					/**< @brief ��ȡ���ص���ɫ */

    void (*draw_hline)(const char *pixel, int x1, int x2, int y);	/**< @brief ��ˮƽ�� */
    void (*draw_vline)(const char *pixel, int x, int y1, int y2);	/**< @brief ����ֱ�� */

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

#define RT_MODULE_FLAG_WITHENTRY        0x00            /**< @brief ����ڵ� */
#define RT_MODULE_FLAG_WITHOUTENTRY     0x01            /**< @brief ����ڵ� */

/*
 * @brief ��̬ģ����ƿ�
 */
struct rt_module
{
    struct rt_object             parent;                /**< @brief �̳��� rt_object */

    rt_uint32_t                  vstart_addr;           /**< @brief ��һ��LOAD�ε�VMA��ַ */
    rt_uint8_t                  *module_space;          /**< @brief ��̬ģ����ڴ�ռ� */

    void                        *module_entry;          /**< @brief ��̬ģ�����ڵ�ַ */
    rt_thread_t                  module_thread;         /**< @brief ��̬ģ������߳� */

    rt_uint8_t                  *module_cmd_line;       /**< @brief ��̬ģ��������� */
    rt_uint32_t                  module_cmd_size;       /**< @brief ��̬ģ�������еĴ�С */

#ifdef RT_USING_SLAB
    /* module memory allocator */
    void                        *mem_list;              /**< @brief ��̬ģ��Ŀ����ڴ��б� */
    void                        *page_array;            /**< @brief ��̬ģ���ʹ��ҳ */
    rt_uint32_t                  page_cnt;              /**< @brief ��̬ģ���ʹ��ҳ�� */
#endif

    rt_uint16_t                  nref;                  /**< @brief ���ô��� */

    rt_uint16_t                  nsym;                  /**< @brief ��̬ģ��ı����� */
    struct rt_module_symtab     *symtab;                /**< @brief ��̬ģ��ı����� */

    rt_uint32_t                  user_data;             /**< @brief ��̬ģ���˽������ */

    void (*module_init)(void);							/**< @brief ģ��ĳ�ʼ������ָ�� */
    void (*module_cleanup)(void);						/**< @brief ģ����������ָ�� */
};
/*
 * @brief ��̬ģ������ָ�붨��
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
