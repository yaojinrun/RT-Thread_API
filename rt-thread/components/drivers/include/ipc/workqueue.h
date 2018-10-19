#ifndef WORKQUEUE_H__
#define WORKQUEUE_H__

#include <rtthread.h>

/**
 * @addtogroup workqueue
 */

/**@{*/

/* workqueue implementation */
/**
 * @brief 工作队列的结构体定义
 */
struct rt_workqueue
{
    rt_list_t      work_list;		/**< @brief 工作队列链表 */
    struct rt_work *work_current; 	/**< @brief 当前工作队列指针 */

    struct rt_semaphore sem; 		/**< @brief 用于同步的信号量 */
    rt_thread_t    work_thread;		/**< @brief 工作队列处理线程 */
};

/**
 * @brief 工作的结构体定义
 */
struct rt_work
{
    rt_list_t list;

    void (*work_func)(struct rt_work* work, void* work_data);
    void *work_data;
};

#ifdef RT_USING_HEAP


/**
 * @brief 创建工作队列
 *
 * 调用该函数接口后，内核首先从动态内存堆中分配一个工作队列控制块，然后对该控制块进行基本的初始化，并创建一个工作队列处理线程。
 *
 * @param name 工作队列名称
 * @param stack_size 工作队列堆栈空间大小
 * @param priority 工作队列的优先级
 *
 * @return 创建成功则返回workqueue控制块，失败则返回 RT_NULL
 */
struct rt_workqueue *rt_workqueue_create(const char* name, rt_uint16_t stack_size, rt_uint8_t priority);

/**
 * @brief 销毁工作队列
 *
 * 调用这个函数接口后，系统会删除这个工作队列的处理线程，然后释放相应的工作队列控制块占有的内存。
 *
 * @param queue workqueue控制块
 *
 * @return RT_EOK
 */
rt_err_t rt_workqueue_destroy(struct rt_workqueue* queue);

/**
 * @brief 增加工作队列任务
 *
 * 调用该函数可以向指定的工作队列中添加任务。
 *
 * @param queue workqueue控制块
 * @param work 任务控制块
 *
 * @return -RT_EBUSY 任务已在执行中，请勿重复执行；RT_EOK 加入workqueue任务成功
 */
rt_err_t rt_workqueue_dowork(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief 取消工作队列任务
 *
 * 调用该函数可以取消任务队列中尚未执行的任务。
 *
 * @param queue workqueue控制块
 * @param work 任务控制块
 *
 * @return -RT_EBUSY 任务已在执行中，不能取消；RT_EOK 取消成功
 */
rt_err_t rt_workqueue_cancel_work(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief 同步取消工作队列任务
 *
 * 调用该函数可以取消循环任务，如果任务正在执行中，则等待该次任务完成后取消。
 *
 * @param queue workqueue控制块
 * @param work 任务控制块
 *
 * @return 取消成功（如果任务已在执行中，等待任务完成后取消）
 */
rt_err_t rt_workqueue_cancel_work_sync(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief 初始化work
 *
 * 调用该函数可以初始化指定work的链表，并指定任务回调函数和待处理的数据指针。
 *
 * @param work work控制块
 * @param work_func work回调处理函数
 * @param work_data 待处理的数据缓冲区指针
 *
 */
rt_inline void rt_work_init(struct rt_work* work, void (*work_func)(struct rt_work* work, void* work_data),
    void* work_data)
{
    rt_list_init(&(work->list));
    work->work_func = work_func;
    work->work_data = work_data;
}

/**@}*/
			
#endif

#endif

