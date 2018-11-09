/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-06-02     Bernard      the first version
 * 2018-08-02     Tanek        split run and sleep modes, support custom mode
 */

#ifndef __PM_H__
#define __PM_H__

#include <rtthread.h>


/**
 * @addtogroup PM
 */

/**@{*/


#ifndef PM_HAS_CUSTOM_CONFIG

/* All modes used for rt_pm_request() adn rt_pm_release() */

/**
 * @brief 运行的模式
 */
enum
{
    /* run modes */
    PM_RUN_MODE_NORMAL = 0,		/**< @brief 运行模式 */

    /* sleep modes */
    PM_SLEEP_MODE_SLEEP,		/**< @brief 休眠模式 */
    PM_SLEEP_MODE_TIMER,		/**< @brief 休眠定时器模式 */
    PM_SLEEP_MODE_SHUTDOWN,		/**< @brief 关闭模式 */
};

/* The name of all modes used in the msh command "pm_dump" */
#define PM_MODE_NAMES           \
{                               \
    "Running Mode",             \
                                \
    "Sleep Mode",               \
    "Timer Mode",               \
    "Shutdown Mode",            \
}

/* run mode count : 1 */
#define PM_RUN_MODE_COUNT       1
/* sleep mode count : 3 */
#define PM_SLEEP_MODE_COUNT     3

/* support redefining default run mode */
#ifndef PM_RUN_MODE_DEFAULT
#define PM_RUN_MODE_DEFAULT     0
#endif

/* support redefining default sleep mode */
#ifndef PM_SLEEP_MODE_DEFAULT
#define PM_SLEEP_MODE_DEFAULT   (PM_SLEEP_MODE_START)
#endif

/* support redefining the minimum tick into sleep mode */
#ifndef PM_MIN_ENTER_SLEEP_TICK
#define PM_MIN_ENTER_SLEEP_TICK   (1)
#endif

#else /* PM_HAS_CUSTOM_CONFIG */

#include <pm_cfg.h>

#ifndef PM_RUN_MODE_COUNT
#error  "You must defined PM_RUN_MODE_COUNT on pm_cfg.h"
#endif

#ifndef PM_SLEEP_MODE_COUNT
#error  "You must defined PM_SLEEP_MODE_COUNT on pm_cfg.h"
#endif

#ifndef PM_MODE_DEFAULT
#error  "You must defined PM_MODE_DEFAULT on pm_cfg.h"
#endif

#ifndef PM_MODE_NAMES
#error  "You must defined PM_MODE_NAMES on pm_cfg.h"
#endif

#ifndef PM_RUN_MODE_DEFAULT
#error  "You must defined PM_RUN_MODE_DEFAULT on pm_cfg.h"
#endif

/* The default sleep mode(PM_SLEEP_MODE_DEFAULT) are not required.
 * If the default mode is defined, it is requested once in rt_system_pm_init()
 */

#endif /* PM_HAS_CUSTOM_CONFIG */

/* run mode must start at 0 */
#define PM_RUN_MODE_START       0
/* the values of the run modes and sleep mode must be consecutive */
#define PM_SLEEP_MODE_START     PM_RUN_MODE_COUNT
/* all mode count */
#define PM_MODE_COUNT           (PM_RUN_MODE_COUNT + PM_SLEEP_MODE_COUNT)
/* The last mode, will be request in rt_system_pm_init() */
#define PM_MODE_MAX             (PM_RUN_MODE_COUNT + PM_SLEEP_MODE_COUNT - 1)

#if PM_MODE_COUNT > 32
#error "The number of modes cannot exceed 32"
#endif

/*
 * device control flag to request or release power
 */
#define RT_PM_DEVICE_CTRL_REQUEST   0x01
#define RT_PM_DEVICE_CTRL_RELEASE   0x02

struct rt_pm;

/**
 * @brief 电源管理设备的操作方法
 */
struct rt_pm_ops
{
    void (*enter)(struct rt_pm *pm);									/**< @brief 切换到新模式 */
    void (*exit)(struct rt_pm *pm);										/**< @brief 退出当前模式 */

#if PM_RUN_MODE_COUNT > 1
    void (*frequency_change)(struct rt_pm *pm, rt_uint32_t frequency);  /**< @brief 切换到新模式 */
#endif

    void (*timer_start)(struct rt_pm *pm, rt_uint32_t timeout);         /**< @brief 开启定时器 */
    void (*timer_stop)(struct rt_pm *pm);								/**< @brief 停止定时器 */
    rt_tick_t (*timer_get_tick)(struct rt_pm *pm);						/**< @brief 获取休眠时间 */
};

/**
 * @brief  对电源管理模式敏感的设备的操作方法
 */
struct rt_device_pm_ops
{
#if PM_RUN_MODE_COUNT > 1
    void (*frequency_change)(const struct rt_device* device);	/**< @brief 切换到新模式 */
#endif

    void (*suspend)(const struct rt_device* device);			/**< @brief 进入休眠模式 */
    void (*resume) (const struct rt_device* device);			/**< @brief 从休眠模式唤醒 */
};

/**
 * @brief 电源管理设备
 */
struct rt_device_pm
{
    const struct rt_device* device;			/**< @brief 继承至设备基类 rt_device */
    const struct rt_device_pm_ops* ops;		/**< @brief 对电源管理模式敏感的设备的操作方法 */
};

/**
 * @brief 电源管理控制块
 */
struct rt_pm
{
    struct rt_device parent;			/**< @brief 继承至设备基类 rt_device */

    /* modes */
    rt_uint8_t modes[PM_MODE_COUNT];	/**< @brief 运行模式 */
    rt_uint8_t current_mode;    		/**< @brief 当前运行模式 */
    rt_uint8_t exit_count;

    /* the list of device, which has PM feature */
    rt_uint8_t device_pm_number;		/**< @brief 有电源管理特性的设备计数 */
    struct rt_device_pm* device_pm;		/**< @brief 电源管理设备句柄 */
    struct rt_semaphore  device_lock;	/**< @brief 设备锁 */

    /* if the mode has timer, the corresponding bit is 1*/
    rt_uint32_t timer_mask;				/**< @brief 定时器标志 */

    const struct rt_pm_ops *ops;		/**< @brief 电源管理设备的操作方法 */
};

/**
 * @brief 进入PM 模式
 *
 * 该函数尝试进入更低的模式，如果没有请求任何运行模式，就进入休眠模式。这个函数已经在 PM 组件
 * 初始化函数里注册到 IDLE HOOK 里，所以不需要另外的调用。
 */
void rt_pm_enter(void);

/**
 * @brief 退出PM 模式
 *
 * 该函数在从休眠模式唤醒的时候被在rt_pm_enter()调用。在从休眠唤醒时，有可能先进入唤醒中断的中断
 * 处理函数里由。用户也可以在这里主动调用用户主动调用rt_pm_exit()。从休眠唤醒之后可能多次调用
 * rt_pm_exit()。
 */
void rt_pm_exit(void);

/**
 * @brief 请求 PM 模式
 *
 * 请求 PM 模式函数，是在应用或者驱动里调用的函数，调用之后 PM 组件确保当前模式不低于请求的模式。
 *
 * @param mode 请求的模式
 */
void rt_pm_request(rt_ubase_t mode);

/**
 * @brief 释放 PM 模式
 *
 * 释放PM模式函数，是在应用或者驱动里调用的函数，调用之后 PM 组件并不会立即进行实际的模式切换，
 * 而是在rt_pm_enter()里面才开始切换。
 *
 * @param mode 释放的模式
 */
void rt_pm_release(rt_ubase_t mode);

/**
 * @brief 注册 PM 模式变化敏感的设备
 *
 * 该函数注册 PM 模式变化敏感的设备，每当 PM 的模式发生变化的时候，会调用设备的相应 API。
 *
 * - 如果是切换到新的运行模式，会调用设备里的frequency_change()函数。
 *
 * - 如果是切换到新的休眠模式，将会在进入休眠时调用设备的suspend()函数，在进入休眠被唤醒之后
 * 调用设备的resume()。
 *
 * @param device 具体对模式变化敏感的设备句柄
 * @param ops 设备的函数集
 */
void rt_pm_register_device(struct rt_device* device, const struct rt_device_pm_ops* ops);

/**
 * @brief 取消注册PM模式变化敏感的设备
 *
 * 该函数取消已经注册的 PM 模式变化敏感设备。
 *
 * @param device 具体对模式变化敏感的设备句柄
 */
void rt_pm_unregister_device(struct rt_device* device);

/**
 * @brief PM组件初始化
 *
 * PM 组件初始化函数，是由相应的 PM 驱动来调用，完成 PM 组件的初始化。
 * 
 * 该函数完成的工作，包括底层 PM 驱动的注册、相应的 PM 组件的资源初始化、默认模式的请求，
 * 并对上层提供一个名字是 “pm” 的设备，还默认请求了三个模式，包括一个默认运行模式PM_RUN_MODE_DEFAULT、
 * 一个默认休眠模式PM_SLEEP_MODE_DEFAULT和最低的模式PM_MODE_MAX。默认运行模式和休眠模式的值，
 * 我们可以根据需要来定义，默认值是第一个模式。
 *
 * @param ops 底层 PM 驱动的函数集
 * @param timer_mask 指定哪些模式包含了低功耗定时器
 * @param user_data 可以被底层 PM 驱动使用的一个指针
 */
void rt_system_pm_init(const struct rt_pm_ops *ops,
                       rt_uint8_t              timer_mask,
                       void                   *user_data);

/**@}*/

#endif /* __PM_H__ */
