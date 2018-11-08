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
 * @brief �ܵ����ƿ�
 */
struct rt_pipe_device
{
    struct rt_device parent;			/**< @brief �̳��� rt_device */

    /* ring buffer in pipe device */
    struct rt_ringbuffer *fifo;			/**< @brief �ܵ���ʹ�õĻ��λ�������� */
    rt_uint16_t bufsz;					/**< @brief �������Ĵ�С */

    rt_uint8_t readers;					/**< @brief �ܵ���ȡ�߼��� */
    rt_uint8_t writers;					/**< @brief �ܵ�д���߼��� */

    rt_wqueue_t reader_queue;			/**< @brief �ܵ����ȴ����� */
    rt_wqueue_t writer_queue;			/**< @brief �ܵ�д�ȴ����� */

    struct rt_mutex lock;				/**< @brief �ܵ��Ļ����� */
};
/**
 * @brief �ܵ����Ͷ���
 */
typedef struct rt_pipe_device rt_pipe_t;

rt_pipe_t *rt_pipe_create(const char *name, int bufsz);

#endif /* PIPE_H__ */
