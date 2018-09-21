/*
 * File      : object.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
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
 * 2006-03-14     Bernard      the first version
 * 2006-04-21     Bernard      change the scheduler lock to interrupt lock
 * 2006-05-18     Bernard      fix the object init bug
 * 2006-08-03     Bernard      add hook support
 * 2007-01-28     Bernard      rename RT_OBJECT_Class_Static to RT_Object_Class_Static
 * 2010-10-26     yi.qiu       add module support in rt_object_allocate and rt_object_free
 * 2017-12-10     Bernard      Add object_info enum.
 * 2018-01-25     Bernard      Fix the object find issue when enable MODULE.
 */

#include <rtthread.h>
#include <rthw.h>

/*
 * define object_info for the number of rt_object_container items.
 */
enum rt_object_info_type
{
    RT_Object_Info_Thread = 0,                         /**< The object is a thread. */
#ifdef RT_USING_SEMAPHORE
    RT_Object_Info_Semaphore,                          /**< The object is a semaphore. */
#endif
#ifdef RT_USING_MUTEX
    RT_Object_Info_Mutex,                              /**< The object is a mutex. */
#endif
#ifdef RT_USING_EVENT
    RT_Object_Info_Event,                              /**< The object is a event. */
#endif
#ifdef RT_USING_MAILBOX
    RT_Object_Info_MailBox,                            /**< The object is a mail box. */
#endif
#ifdef RT_USING_MESSAGEQUEUE
    RT_Object_Info_MessageQueue,                       /**< The object is a message queue. */
#endif
#ifdef RT_USING_MEMHEAP
    RT_Object_Info_MemHeap,                            /**< The object is a memory heap */
#endif
#ifdef RT_USING_MEMPOOL
    RT_Object_Info_MemPool,                            /**< The object is a memory pool. */
#endif
#ifdef RT_USING_DEVICE
    RT_Object_Info_Device,                             /**< The object is a device */
#endif
    RT_Object_Info_Timer,                              /**< The object is a timer. */
#ifdef RT_USING_MODULE
    RT_Object_Info_Module,                             /**< The object is a module. */
#endif
    RT_Object_Info_Unknown,                            /**< The object is unknown. */
};

#define _OBJ_CONTAINER_LIST_INIT(c)     \
    {&(rt_object_container[c].object_list), &(rt_object_container[c].object_list)}
static struct rt_object_information rt_object_container[RT_Object_Info_Unknown] =
{
    /* initialize object container - thread */
    {RT_Object_Class_Thread, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Thread), sizeof(struct rt_thread)},
#ifdef RT_USING_SEMAPHORE
    /* initialize object container - semaphore */
    {RT_Object_Class_Semaphore, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Semaphore), sizeof(struct rt_semaphore)},
#endif
#ifdef RT_USING_MUTEX
    /* initialize object container - mutex */
    {RT_Object_Class_Mutex, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Mutex), sizeof(struct rt_mutex)},
#endif
#ifdef RT_USING_EVENT
    /* initialize object container - event */
    {RT_Object_Class_Event, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Event), sizeof(struct rt_event)},
#endif
#ifdef RT_USING_MAILBOX
    /* initialize object container - mailbox */
    {RT_Object_Class_MailBox, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MailBox), sizeof(struct rt_mailbox)},
#endif
#ifdef RT_USING_MESSAGEQUEUE
    /* initialize object container - message queue */
    {RT_Object_Class_MessageQueue, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MessageQueue), sizeof(struct rt_messagequeue)},
#endif
#ifdef RT_USING_MEMHEAP
    /* initialize object container - memory heap */
    {RT_Object_Class_MemHeap, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MemHeap), sizeof(struct rt_memheap)},
#endif
#ifdef RT_USING_MEMPOOL
    /* initialize object container - memory pool */
    {RT_Object_Class_MemPool, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_MemPool), sizeof(struct rt_mempool)},
#endif
#ifdef RT_USING_DEVICE
    /* initialize object container - device */
    {RT_Object_Class_Device, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Device), sizeof(struct rt_device)},
#endif
    /* initialize object container - timer */
    {RT_Object_Class_Timer, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Timer), sizeof(struct rt_timer)},
#ifdef RT_USING_MODULE
    /* initialize object container - module */
    {RT_Object_Class_Module, _OBJ_CONTAINER_LIST_INIT(RT_Object_Info_Module), sizeof(struct rt_module)},
#endif
};

#ifdef RT_USING_HOOK
static void (*rt_object_attach_hook)(struct rt_object *object);
static void (*rt_object_detach_hook)(struct rt_object *object);
void (*rt_object_trytake_hook)(struct rt_object *object);
void (*rt_object_take_hook)(struct rt_object *object);
void (*rt_object_put_hook)(struct rt_object *object);

/**
 * @addtogroup Hook
 */

/**@{*/

/**
 * 该函数将设置一个钩子函数，当对象添加到内核对象系统时将调用该钩子函数。
 *
 * @param hook the hook function
 */
void rt_object_attach_sethook(void (*hook)(struct rt_object *object))
{
    rt_object_attach_hook = hook;
}

/**
 * 该函数将设置一个钩子函数，当对象从内核对象系统脱离时将调用该钩子函数。
 *
 * @param hook the hook function
 */
void rt_object_detach_sethook(void (*hook)(struct rt_object *object))
{
    rt_object_detach_hook = hook;
}

/**
 * 该函数将设置一个钩子函数，当对象从内核对象系统中被尝试获取时将调用该钩子函数。
 *
 * 对象被获取指的是：
 * semaphore - 信号量被线程获取
 * mutex - 互斥量被线程获取
 * event - 事件被线程接收
 * mailbox - 邮件被线程接收
 * message queue - 消息队列被线程接收
 *
 * @param hook 钩子函数
 */
void rt_object_trytake_sethook(void (*hook)(struct rt_object *object))
{
    rt_object_trytake_hook = hook;
}

/**
 * 该函数将设置一个钩子函数，当对象从内核对象系统中被获取后将调用该钩子函数。
 *
 * 对象被获取指的是：
 * semaphore - 信号量被线程获取
 * mutex - 互斥量被线程获取
 * event - 事件被线程接收
 * mailbox - 邮件被线程接收
 * message queue - 消息队列被线程接收
 * timer - 定时器被启动
 *
 * @param hook 钩子函数
 */
void rt_object_take_sethook(void (*hook)(struct rt_object *object))
{
    rt_object_take_hook = hook;
}

/**
 * 该函数将设置一个钩子函数，当对象被放到内核对象系统中时将运行该钩子函数。
 *
 * @param hook 钩子函数
 */
void rt_object_put_sethook(void (*hook)(struct rt_object *object))
{
    rt_object_put_hook = hook;
}

/**@}*/
#endif

/*
 * @ingroup SystemInit
 *
 * 该函数将初始化系统对象管理。
 *
 * @deprecated 从0.3.0开始，已经不需要在系统初始化中调用此函数。
 */
void rt_system_object_init(void)
{
}

/**
 * @addtogroup KernelObject
 */

/**@{*/

/*
 * 该函数将返回指定类型的对象信息。
 *
 * @param type 对象的类型
 * @return 对象的类型信息或者 RT_NULL
 */
struct rt_object_information *
rt_object_get_information(enum rt_object_class_type type)
{
    int index;

    for (index = 0; index < RT_Object_Info_Unknown; index ++)
        if (rt_object_container[index].type == type) return &rt_object_container[index];

    return RT_NULL;
}
RTM_EXPORT(rt_object_get_information);

/**
 * @brief 初始化对象
 *
 * 该函数将初始化一个对象并将其放到系统对象管理器中。
 *
 * @param object 将要被初始化的指定对象
 * @param type 对象类型
 * @param name 对象的名称。在系统中对象的名称必须唯一。
 */
void rt_object_init(struct rt_object         *object,
                    enum rt_object_class_type type,
                    const char               *name)
{
    register rt_base_t temp;
    struct rt_object_information *information;

    /* get object information */
    information = rt_object_get_information(type);
    RT_ASSERT(information != RT_NULL);

    /* initialize object's parameters */

    /* set object type to static */
    object->type = type | RT_Object_Class_Static;

    /* copy name */
    rt_strncpy(object->name, name, RT_NAME_MAX);

    RT_OBJECT_HOOK_CALL(rt_object_attach_hook, (object));

    /* lock interrupt */
    temp = rt_hw_interrupt_disable();

    /* insert object into information object list */
    rt_list_insert_after(&(information->object_list), &(object->list));

    /* unlock interrupt */
    rt_hw_interrupt_enable(temp);
}

/**
 * @brief 脱离对象
 *
 * 该函数将从对象管理器中脱离一个静态的对象，但是该静态对象并不会被释放。
 *
 * @param object 指定脱离的对象
 */
void rt_object_detach(rt_object_t object)
{
    register rt_base_t temp;

    /* object check */
    RT_ASSERT(object != RT_NULL);

    RT_OBJECT_HOOK_CALL(rt_object_detach_hook, (object));

    /* lock interrupt */
    temp = rt_hw_interrupt_disable();

    /* remove from old list */
    rt_list_remove(&(object->list));

    /* unlock interrupt */
    rt_hw_interrupt_enable(temp);
}

#ifdef RT_USING_HEAP
/**
 * @brief 分配对象
 *
 * 该函数将从系统资源中分配一个对象
 *
 * @param type 对象类型
 * @param name 对象的名称，在系统中，对象的名称必须唯一
 *
 * @return 分配的对象
 */
rt_object_t rt_object_allocate(enum rt_object_class_type type, const char *name)
{
    struct rt_object *object;
    register rt_base_t temp;
    struct rt_object_information *information;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* get object information */
    information = rt_object_get_information(type);
    RT_ASSERT(information != RT_NULL);

    object = (struct rt_object *)RT_KERNEL_MALLOC(information->object_size);
    if (object == RT_NULL)
    {
        /* no memory can be allocated */
        return RT_NULL;
    }

    /* initialize object's parameters */

    /* set object type */
    object->type = type;

    /* set object flag */
    object->flag = 0;

#ifdef RT_USING_MODULE
    if (rt_module_self() != RT_NULL)
    {
        object->flag |= RT_OBJECT_FLAG_MODULE;
    }
    object->module_id = (void *)rt_module_self();
#endif

    /* copy name */
    rt_strncpy(object->name, name, RT_NAME_MAX);

    RT_OBJECT_HOOK_CALL(rt_object_attach_hook, (object));

    /* lock interrupt */
    temp = rt_hw_interrupt_disable();

    /* insert object into information object list */
    rt_list_insert_after(&(information->object_list), &(object->list));

    /* unlock interrupt */
    rt_hw_interrupt_enable(temp);

    /* return object */
    return object;
}

/**
 * @brief 删除对象
 *
 * 该函数将删除一个对象并释放对象所占用的内存空间。
 *
 * @param object 指定将被删除的对象
 */
void rt_object_delete(rt_object_t object)
{
    register rt_base_t temp;

    /* object check */
    RT_ASSERT(object != RT_NULL);
    RT_ASSERT(!(object->type & RT_Object_Class_Static));

    RT_OBJECT_HOOK_CALL(rt_object_detach_hook, (object));

    /* lock interrupt */
    temp = rt_hw_interrupt_disable();

    /* remove from old list */
    rt_list_remove(&(object->list));

    /* unlock interrupt */
    rt_hw_interrupt_enable(temp);

    /* free the memory of object */
    RT_KERNEL_FREE(object);
}
#endif

/**
 * @brief 辨别对象
 *
 * 此函数将判断对象是否为系统对象。 通常，系统对象是静态对象，
 * 对象类型设置为RT_Object_Class_Static。
 *
 * @param object 将被判定的指定对象
 *
 * @return 如果为系统对象返回 RT_TRUE, 否则返回 RT_FALSE。
 */
rt_bool_t rt_object_is_systemobject(rt_object_t object)
{
    /* object check */
    RT_ASSERT(object != RT_NULL);

    if (object->type & RT_Object_Class_Static)
        return RT_TRUE;

    return RT_FALSE;
}

/*
 * 该函数将从对象容器中查找具有指定名称的对象。
 *
 * @param name 具有指定名称额对象
 * @param type the type of object
 *
 * @return 返回查找到的对象，如果没有此对象则返回 RT_NULL
 *
 * @note 该函数应该在中断上下文中调用。
 */
rt_object_t rt_object_find(const char *name, rt_uint8_t type)
{
    struct rt_object *object = RT_NULL;
    struct rt_list_node *node = RT_NULL;
    struct rt_object_information *information = RT_NULL;

    /* parameter check */
    if ((name == RT_NULL) || (type > RT_Object_Class_Unknown))
        return RT_NULL;

    /* which is invoke in interrupt status */
    RT_DEBUG_NOT_IN_INTERRUPT;

    /* enter critical */
    rt_enter_critical();

    /* try to find object */
    if (information == RT_NULL)
    {
        information = rt_object_get_information((enum rt_object_class_type)type);
        RT_ASSERT(information != RT_NULL);
    }
    for (node  = information->object_list.next;
            node != &(information->object_list);
            node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (rt_strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            rt_exit_critical();

            return object;
        }
    }

    /* leave critical */
    rt_exit_critical();

    return RT_NULL;
}

/**@}*/
