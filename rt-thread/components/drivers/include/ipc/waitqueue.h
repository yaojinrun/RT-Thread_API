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
 * waitqueue �ṹ�嶨��
 */
struct rt_wqueue_node
{
    rt_thread_t polling_thread;		/**< @brief �ȴ�������ѯ�߳� */
    rt_list_t   list;				/**< @brief �ȴ��������� */

    rt_wqueue_func_t wakeup;		/**< @brief ���ѻص����� */
    rt_uint32_t key;				/**< @brief �������� */
};
typedef struct rt_wqueue_node rt_wqueue_node_t;


/**
 * @addtogroup waitqueue
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
 * @brief ����ȴ�����
 * 
 * ��ָ���ȴ������Ϲ���ȴ������ѻ�ʱ���ء�
 *
 * @param queue 	�ȴ����п��ƿ�
 * @param condition ��POSIX ��׼�ӿڼ��ݵĲ�����Ŀǰû�����壬����0���ɣ���
 * @param timeout	ָ���ĵȴ�ʱ�䡣
 *
 * @return 0 ����ɹ�
 */
int  rt_wqueue_wait(rt_wqueue_t *queue, int condition, int timeout);

/**
 * @brief ���ѵȴ�����
 * 
 * ����ָ���ȴ������Ϸ��������ı�������̡߳�
 *
 * @param queue �ȴ����п��ƿ�
 * @param key 	��������
 */
void rt_wqueue_wakeup(rt_wqueue_t *queue, void *key);

/**
 * @brief ����ȴ�����
 * 
 * @param name:    	�ȴ����п��ƿ�ʵ��
 * @param function: �ȴ����л��������жϺ������ɿͻ�����
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
