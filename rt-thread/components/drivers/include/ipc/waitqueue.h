/*
 * File      : waitqueue.h
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
 * 2018/06/26     Bernard      Fix the wait queue issue when wakeup a soon 
 *                             to blocked thread.
 */

#ifndef WAITQUEUE_H__
#define WAITQUEUE_H__

#include <rtthread.h>

#define RT_WQ_FLAG_CLEAN    0x00
#define RT_WQ_FLAG_WAKEUP   0x01

struct rt_wqueue_node;
typedef int (*rt_wqueue_func_t)(struct rt_wqueue_node *wait, void *key);

/**
 * wqueue 结构体定义
 */
struct rt_wqueue_node
{
    rt_thread_t polling_thread;		/**< @brief 等待队列轮询线程 */
    rt_list_t   list;				/**< @brief 等待队列链表 */

    rt_wqueue_func_t wakeup;		/**< @brief 唤醒回调函数 */
    rt_uint32_t key;				/**< @brief 唤醒条件 */
};
typedef struct rt_wqueue_node rt_wqueue_node_t;


/**
 * @addtogroup wqueue
 */

/**@{*/

int __wqueue_default_wake(struct rt_wqueue_node *wait, void *key);

rt_inline void rt_wqueue_init(rt_wqueue_t *queue)
{
    RT_ASSERT(queue != RT_NULL);

    queue->flag = RT_WQ_FLAG_CLEAN;
    rt_list_init(&(queue->waiting_list));
}

void rt_wqueue_add(rt_wqueue_t *queue, struct rt_wqueue_node *node);
void rt_wqueue_remove(struct rt_wqueue_node *node);

/**
 * @brief 加入等待队列
 * 
 * 在指定等待队列上挂起等待被唤醒或超时返回。
 *
 * @param queue 	等待队列控制块
 * @param condition 与POSIX 标准接口兼容的参数（目前没有意义，传入0即可）。
 * @param timeout	指定的等待时间。
 *
 * @return 0 加入成功
 */
int  rt_wqueue_wait(rt_wqueue_t *queue, int condition, int timeout);

/**
 * @brief 唤醒等待队列
 * 
 * 唤醒指定等待队列上符合条件的被挂起的线程。
 *
 * @param queue 等待队列控制块
 * @param key 	唤醒条件
 */
void rt_wqueue_wakeup(rt_wqueue_t *queue, void *key);

/**
 * @brief 定义等待队列
 * 
 * @param name:    	等待队列控制块实体
 * @param function: 等待队列唤醒条件判断函数，由客户定义
 */
#define DEFINE_WAIT_FUNC(name, function)                \
    struct rt_wqueue_node name = {                      \
        rt_current_thread,                              \
        RT_LIST_OBJECT_INIT(((name).list)),             \
                                                        \
        function,                                       \
        0                                               \
    }

#define DEFINE_WAIT(name) DEFINE_WAIT_FUNC(name, __wqueue_default_wake)


/**@}*/
	
#endif
