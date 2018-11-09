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
 * @brief ���е�ģʽ
 */
enum
{
    /* run modes */
    PM_RUN_MODE_NORMAL = 0,		/**< @brief ����ģʽ */

    /* sleep modes */
    PM_SLEEP_MODE_SLEEP,		/**< @brief ����ģʽ */
    PM_SLEEP_MODE_TIMER,		/**< @brief ���߶�ʱ��ģʽ */
    PM_SLEEP_MODE_SHUTDOWN,		/**< @brief �ر�ģʽ */
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
 * @brief ��Դ�����豸�Ĳ�������
 */
struct rt_pm_ops
{
    void (*enter)(struct rt_pm *pm);									/**< @brief �л�����ģʽ */
    void (*exit)(struct rt_pm *pm);										/**< @brief �˳���ǰģʽ */

#if PM_RUN_MODE_COUNT > 1
    void (*frequency_change)(struct rt_pm *pm, rt_uint32_t frequency);  /**< @brief �л�����ģʽ */
#endif

    void (*timer_start)(struct rt_pm *pm, rt_uint32_t timeout);         /**< @brief ������ʱ�� */
    void (*timer_stop)(struct rt_pm *pm);								/**< @brief ֹͣ��ʱ�� */
    rt_tick_t (*timer_get_tick)(struct rt_pm *pm);						/**< @brief ��ȡ����ʱ�� */
};

/**
 * @brief  �Ե�Դ����ģʽ���е��豸�Ĳ�������
 */
struct rt_device_pm_ops
{
#if PM_RUN_MODE_COUNT > 1
    void (*frequency_change)(const struct rt_device* device);	/**< @brief �л�����ģʽ */
#endif

    void (*suspend)(const struct rt_device* device);			/**< @brief ��������ģʽ */
    void (*resume) (const struct rt_device* device);			/**< @brief ������ģʽ���� */
};

/**
 * @brief ��Դ�����豸
 */
struct rt_device_pm
{
    const struct rt_device* device;			/**< @brief �̳����豸���� rt_device */
    const struct rt_device_pm_ops* ops;		/**< @brief �Ե�Դ����ģʽ���е��豸�Ĳ������� */
};

/**
 * @brief ��Դ������ƿ�
 */
struct rt_pm
{
    struct rt_device parent;			/**< @brief �̳����豸���� rt_device */

    /* modes */
    rt_uint8_t modes[PM_MODE_COUNT];	/**< @brief ����ģʽ */
    rt_uint8_t current_mode;    		/**< @brief ��ǰ����ģʽ */
    rt_uint8_t exit_count;

    /* the list of device, which has PM feature */
    rt_uint8_t device_pm_number;		/**< @brief �е�Դ�������Ե��豸���� */
    struct rt_device_pm* device_pm;		/**< @brief ��Դ�����豸��� */
    struct rt_semaphore  device_lock;	/**< @brief �豸�� */

    /* if the mode has timer, the corresponding bit is 1*/
    rt_uint32_t timer_mask;				/**< @brief ��ʱ����־ */

    const struct rt_pm_ops *ops;		/**< @brief ��Դ�����豸�Ĳ������� */
};

/**
 * @brief ����PM ģʽ
 *
 * �ú������Խ�����͵�ģʽ�����û�������κ�����ģʽ���ͽ�������ģʽ����������Ѿ��� PM ���
 * ��ʼ��������ע�ᵽ IDLE HOOK ����Բ���Ҫ����ĵ��á�
 */
void rt_pm_enter(void);

/**
 * @brief �˳�PM ģʽ
 *
 * �ú����ڴ�����ģʽ���ѵ�ʱ����rt_pm_enter()���á��ڴ����߻���ʱ���п����Ƚ��뻽���жϵ��ж�
 * ���������ɡ��û�Ҳ�������������������û���������rt_pm_exit()�������߻���֮����ܶ�ε���
 * rt_pm_exit()��
 */
void rt_pm_exit(void);

/**
 * @brief ���� PM ģʽ
 *
 * ���� PM ģʽ����������Ӧ�û�����������õĺ���������֮�� PM ���ȷ����ǰģʽ�����������ģʽ��
 *
 * @param mode �����ģʽ
 */
void rt_pm_request(rt_ubase_t mode);

/**
 * @brief �ͷ� PM ģʽ
 *
 * �ͷ�PMģʽ����������Ӧ�û�����������õĺ���������֮�� PM �����������������ʵ�ʵ�ģʽ�л���
 * ������rt_pm_enter()����ſ�ʼ�л���
 *
 * @param mode �ͷŵ�ģʽ
 */
void rt_pm_release(rt_ubase_t mode);

/**
 * @brief ע�� PM ģʽ�仯���е��豸
 *
 * �ú���ע�� PM ģʽ�仯���е��豸��ÿ�� PM ��ģʽ�����仯��ʱ�򣬻�����豸����Ӧ API��
 *
 * - ������л����µ�����ģʽ��������豸���frequency_change()������
 *
 * - ������л����µ�����ģʽ�������ڽ�������ʱ�����豸��suspend()�������ڽ������߱�����֮��
 * �����豸��resume()��
 *
 * @param device �����ģʽ�仯���е��豸���
 * @param ops �豸�ĺ�����
 */
void rt_pm_register_device(struct rt_device* device, const struct rt_device_pm_ops* ops);

/**
 * @brief ȡ��ע��PMģʽ�仯���е��豸
 *
 * �ú���ȡ���Ѿ�ע��� PM ģʽ�仯�����豸��
 *
 * @param device �����ģʽ�仯���е��豸���
 */
void rt_pm_unregister_device(struct rt_device* device);

/**
 * @brief PM�����ʼ��
 *
 * PM �����ʼ��������������Ӧ�� PM ���������ã���� PM ����ĳ�ʼ����
 * 
 * �ú�����ɵĹ����������ײ� PM ������ע�ᡢ��Ӧ�� PM �������Դ��ʼ����Ĭ��ģʽ������
 * �����ϲ��ṩһ�������� ��pm�� ���豸����Ĭ������������ģʽ������һ��Ĭ������ģʽPM_RUN_MODE_DEFAULT��
 * һ��Ĭ������ģʽPM_SLEEP_MODE_DEFAULT����͵�ģʽPM_MODE_MAX��Ĭ������ģʽ������ģʽ��ֵ��
 * ���ǿ��Ը�����Ҫ�����壬Ĭ��ֵ�ǵ�һ��ģʽ��
 *
 * @param ops �ײ� PM �����ĺ�����
 * @param timer_mask ָ����Щģʽ�����˵͹��Ķ�ʱ��
 * @param user_data ���Ա��ײ� PM ����ʹ�õ�һ��ָ��
 */
void rt_system_pm_init(const struct rt_pm_ops *ops,
                       rt_uint8_t              timer_mask,
                       void                   *user_data);

/**@}*/

#endif /* __PM_H__ */
