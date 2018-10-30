/*
 * File      : idle.c
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
 * 2006-03-23     Bernard      the first version
 * 2010-11-10     Bernard      add cleanup callback function in thread exit.
 * 2012-12-29     Bernard      fix compiling warning.
 * 2013-12-21     Grissiom     let rt_thread_idle_excute loop until there is no
 *                             dead thread.
 * 2016-08-09     ArdaFu       add method to get the handler of the idle thread.
 * 2018-02-07     Bernard      lock scheduler to protect tid->cleanup.
 */

#include <rthw.h>
#include <rtthread.h>

#if defined (RT_USING_HOOK)
#ifndef RT_USING_IDLE_HOOK
#define RT_USING_IDLE_HOOK
#endif
#endif

#ifndef IDLE_THREAD_STACK_SIZE
#if defined (RT_USING_IDLE_HOOK) || defined(RT_USING_HEAP)
#define IDLE_THREAD_STACK_SIZE  256
#else
#define IDLE_THREAD_STACK_SIZE  128
#endif
#endif

static struct rt_thread idle;
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rt_thread_stack[IDLE_THREAD_STACK_SIZE];

extern rt_list_t rt_thread_defunct;

#ifdef RT_USING_IDLE_HOOK
static void (*rt_thread_idle_hook)();

/**
 * @ingroup Hook
 * @brief 空闲钩子函数设置
 *
 * 该函数将钩子函数设置为空闲线程循环。 当系统执行空闲循环时，会调用此钩子函数。
 *
 * @param hook 指定的钩子函数
 *
 * @note 钩子函数必须简单，并且永远不会被阻塞或挂起。
 */
void rt_thread_idle_sethook(void (*hook)(void))
{
    rt_thread_idle_hook = hook;
}
#endif

/* Return whether there is defunctional thread to be deleted. */
rt_inline int _has_defunct_thread(void)
{
    /* The rt_list_isempty has prototype of "int rt_list_isempty(const rt_list_t *l)".
     * So the compiler has a good reason that the rt_thread_defunct list does
     * not change within rt_thread_idle_excute thus optimize the "while" loop
     * into a "if".
     *
     * So add the volatile qualifier here. */
    const volatile rt_list_t *l = (const volatile rt_list_t *)&rt_thread_defunct;

    return l->next != l;
}

/**
 * @ingroup Thread
 * @brief 系统空闲线程执行函数
 *
 * 当系统空闲时将运行空闲线程并执行该函数。
 */
void rt_thread_idle_excute(void)
{
    /* Loop until there is no dead thread. So one call to rt_thread_idle_excute
     * will do all the cleanups. */
    while (_has_defunct_thread())
    {
        rt_base_t lock;
        rt_thread_t thread;
#ifdef RT_USING_MODULE
        rt_module_t module = RT_NULL;
#endif
        RT_DEBUG_NOT_IN_INTERRUPT;

        /* disable interrupt */
        lock = rt_hw_interrupt_disable();

        /* re-check whether list is empty */
        if (_has_defunct_thread())
        {
            /* get defunct thread */
            thread = rt_list_entry(rt_thread_defunct.next,
                                   struct rt_thread,
                                   tlist);
#ifdef RT_USING_MODULE
            /* get thread's parent module */
            module = (rt_module_t)thread->module_id;

            /* if the thread is module's main thread */
            if (module != RT_NULL && module->module_thread == thread)
            {
                /* detach module's main thread */
                module->module_thread = RT_NULL;
            }
#endif
            /* remove defunct thread */
            rt_list_remove(&(thread->tlist));

            /* lock scheduler to prevent scheduling in cleanup function. */
            rt_enter_critical();

            /* invoke thread cleanup */
            if (thread->cleanup != RT_NULL)
                thread->cleanup(thread);

#ifdef RT_USING_SIGNALS
            rt_thread_free_sig(thread);
#endif

            /* if it's a system object, not delete it */
            if (rt_object_is_systemobject((rt_object_t)thread) == RT_TRUE)
            {
                /* unlock scheduler */
                rt_exit_critical();

                /* enable interrupt */
                rt_hw_interrupt_enable(lock);

                return;
            }

            /* unlock scheduler */
            rt_exit_critical();
        }
        else
        {
            /* enable interrupt */
            rt_hw_interrupt_enable(lock);

            /* may the defunct thread list is removed by others, just return */
            return;
        }

        /* enable interrupt */
        rt_hw_interrupt_enable(lock);

#ifdef RT_USING_HEAP
        /* release thread's stack */
        RT_KERNEL_FREE(thread->stack_addr);
        /* delete thread object */
        rt_object_delete((rt_object_t)thread);
#endif
    }
}

static void rt_thread_idle_entry(void *parameter)
{
    while (1)
    {
#ifdef RT_USING_IDLE_HOOK
        if (rt_thread_idle_hook != RT_NULL)
        {
            rt_thread_idle_hook();
        }
#endif

        rt_thread_idle_excute();
    }
}

/**
 * @ingroup SystemInit
 * @brief 空闲线程初始化
 *
 * 该函数初始化空闲线程然后启动它。
 *
 * @note 系统初始化时必须调用该函数。
 */
void rt_thread_idle_init(void)
{
    /* initialize thread */
    rt_thread_init(&idle,
                   "tidle",
                   rt_thread_idle_entry,
                   RT_NULL,
                   &rt_thread_stack[0],
                   sizeof(rt_thread_stack),
                   RT_THREAD_PRIORITY_MAX - 1,
                   32);

    /* startup */
    rt_thread_startup(&idle);
}

/**
 * @ingroup Thread
 * @brief 获取空闲线程处理函数
 * 该函数将获取空闲线程的处理程序。
 *
 */
rt_thread_t rt_thread_idle_gethandler(void)
{
    return (rt_thread_t)(&idle);
}
