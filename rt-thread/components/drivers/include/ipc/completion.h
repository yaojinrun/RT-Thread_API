#ifndef COMPLETION_H_
#define COMPLETION_H_

#include <rtthread.h>


/**
 * @addtogroup completion
 */

/**@{*/

/**
 * @brief Completion 结构体定义
 */

struct rt_completion
{
    rt_uint32_t flag;	/**< @brief completion的参数 */

    /* suspended list */
    rt_list_t suspended_list;	/**< @brief 挂起在该completion上的线程 */
};

/**
 * @brief 初始化completion
 *
 * 该函数将初始化一个 completion 对象。
 *
 * @param completion completion 句柄
 *
 */
void rt_completion_init(struct rt_completion *completion);

/**
 * @brief 等待 completion
 *
 * 该函数将使调用线程挂起指定的等待时间等待completion信号。
 *
 * @param completion completion 句柄
 * @param timeout 指定的超时时间
 *
 * @return RT_EOK 等到 completion 信号； -RT_ETIMEOUT 等待超时
 */
rt_err_t rt_completion_wait(struct rt_completion *completion,
                            rt_int32_t            timeout);

/**
* @brief 发送 completion信号
 *
 * 该函数将发送 completion信号。
 *
 * @param completion completion 句柄
 *
 */
void rt_completion_done(struct rt_completion *completion);


/**@}*/

#endif
