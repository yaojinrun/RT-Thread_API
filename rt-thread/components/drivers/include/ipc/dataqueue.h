#ifndef DATAQUEUE_H__
#define DATAQUEUE_H__

#include <rtthread.h>

/**
 * @addtogroup dataqueue
 */

/**@{*/

/*
 * 数据队列事件命令
 */
#define RT_DATAQUEUE_EVENT_UNKNOWN   0x00            /**< @brief 未知数据队列事件 */
#define RT_DATAQUEUE_EVENT_POP       0x01            /**< @brief 数据队列取出事件 */
#define RT_DATAQUEUE_EVENT_PUSH      0x02            /**< @brief 数据队列写入事件 */
#define RT_DATAQUEUE_EVENT_LWM       0x03            /**< @brief 数据队列数达到设定阈值事件 */

struct rt_data_item;
#define RT_DATAQUEUE_SIZE(dq)        ((dq)->put_index - (dq)->get_index)	            /**< @brief 数据队列使用数量 */
#define RT_DATAQUEUE_EMPTY(dq)       ((dq)->size - RT_DATAQUEUE_SIZE(dq))            	/**< @brief 数据队列空闲数量 */
/* data queue implementation */
/**
 * @brief data queue 结构体
 */
struct rt_data_queue
{
    rt_uint16_t size;					/**< @brief 数据队列的容量 */
    rt_uint16_t lwm;					/**< @brief 阈值 */
    rt_bool_t   waiting_lwm;			

    rt_uint16_t get_index;				/**< @brief  数据队列读取位置*/
    rt_uint16_t put_index;				/**< @brief  数据队列写入位置*/

    struct rt_data_item *queue;			/**< @brief  数据队列中的数据信息，包含数据指针和大小信息*/

    rt_list_t suspended_push_list;		/**< @brief  数据队列写入线程挂起链表*/
    rt_list_t suspended_pop_list;		/**< @brief  数据队列读取线程挂起链表*/

    /* event notify */
    void (*evt_notify)(struct rt_data_queue *queue, rt_uint32_t event);	/**< @brief 事件通知回调函数 */
};

/*
 * DataQueue for DeviceDriver
 */
/**
 * @brief 数据队列初始化
 *
 * 调用该函数将初始化数据队列控制块，并设定通知回调函数。
 *
 * @param queue 数据队列句柄
 * @param size 数据队列块大小
 * @param lwm 阈值设定
 * @param evt_notify 通知事件回调函数
 *
 * @return -RT_ENOMEM 内存不足申请失败； RT_EOK 初始化成功
 */
rt_err_t rt_data_queue_init(struct rt_data_queue *queue,
                            rt_uint16_t           size,
                            rt_uint16_t           lwm,
                            void (*evt_notify)(struct rt_data_queue *queue, rt_uint32_t event));

/**
 * @brief 向数据队列中写入数据
 *
 * 调用该函数可以向指定的数据队列中写入数据，当数据队列已满，则在指定的超时时间内挂起等待。
 *
 * @param queue 数据队列句柄
 * @param data_ptr 待写入的数据指针
 * @param data_size 待写入数据的字节数
 * @param timeout 指定超时时间
 *
 * @return -RT_ETIMEOUT 队列已满，等待超时；RT_EOK 写入成功
 */
rt_err_t rt_data_queue_push(struct rt_data_queue *queue,
                            const void           *data_ptr,
                            rt_size_t             data_size,
                            rt_int32_t            timeout);
							
/**
 * @brief 取出数据队列中的数据
 *
 * 调用该函数可以从指定的数据队列中取出数据，如果数据队列为空，则在指定的时间内挂起等待。
 *
 * @param queue 数据队列句柄
 * @param data_ptr 待取出的数据指针
 * @param data_size 待取出数据的字节数
 * @param timeout 指定超时时间
 *
 * @return -RT_ETIMEOUT 队列已空，等待超时；RT_EOK 取出成功
 */
rt_err_t rt_data_queue_pop(struct rt_data_queue *queue,
                           const void          **data_ptr,
                           rt_size_t            *size,
                           rt_int32_t            timeout);

/**
 * @brief 取出但保留数据队列中的数据
 *
 * 调用该函数可以从指定的数据队列中取出数据，如果数据队列为空，则在指定的时间内挂起等待。
 *
 * @param queue 数据队列句柄
 * @param data_ptr 待取出的数据指针
 * @param data_size 待取出数据的字节数
 * @param timeout 指定超时时间
 *
 * @return --RT_EEMPTY 队列已空；RT_EOK 取出成功
 */
rt_err_t rt_data_queue_peak(struct rt_data_queue *queue,
                            const void          **data_ptr,
                            rt_size_t            *size);
							

/**
 * @brief 重置数据队列
 *
 * 调用该函数将唤醒数据队列上挂起等待的所有线程。
 *
 * @param queue 数据队列句柄
 *
 */
void rt_data_queue_reset(struct rt_data_queue *queue);

/**@}*/
							
#endif
