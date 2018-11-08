#ifndef PIPE_H__
#define PIPE_H__

/*
 * Pipe Device
 */
#include <rtthread.h>
#include <rtdevice.h>

#ifndef RT_PIPE_BUFSZ
#define PIPE_BUFSZ    512
#else
#define PIPE_BUFSZ    RT_PIPE_BUFSZ
#endif
/**
 * @brief 管道控制块
 */
struct rt_pipe_device
{
    struct rt_device parent;			/**< @brief 继承自 rt_device */

    /* ring buffer in pipe device */
    struct rt_ringbuffer *fifo;			/**< @brief 管道所使用的环形缓冲区句柄 */
    rt_uint16_t bufsz;					/**< @brief 缓冲区的大小 */

    rt_uint8_t readers;					/**< @brief 管道读取者计数 */
    rt_uint8_t writers;					/**< @brief 管道写入者计数 */

    rt_wqueue_t reader_queue;			/**< @brief 管道读等待队列 */
    rt_wqueue_t writer_queue;			/**< @brief 管道写等待队列 */

    struct rt_mutex lock;				/**< @brief 管道的互斥锁 */
};
/**
 * @brief 管道类型定义
 */
typedef struct rt_pipe_device rt_pipe_t;

rt_pipe_t *rt_pipe_create(const char *name, int bufsz);

#endif /* PIPE_H__ */
