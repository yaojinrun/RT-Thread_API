#ifndef RINGBUFFER_H__
#define RINGBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>

/**
 * @addtogroup ringbuffer
 */

/**@{*/


/* ring buffer */
/**
 * @brief 环形缓冲区结构体定义
 */
struct rt_ringbuffer
{
    rt_uint8_t *buffer_ptr;			/**< @brief 缓冲区指针 */
    /* use the msb of the {read,write}_index as mirror bit. You can see this as
     * if the buffer adds a virtual mirror and the pointers point either to the
     * normal or to the mirrored buffer. If the write_index has the same value
     * with the read_index, but in a different mirror, the buffer is full.
     * While if the write_index and the read_index are the same and within the
     * same mirror, the buffer is empty. The ASCII art of the ringbuffer is:
     *
     *          mirror = 0                    mirror = 1
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Full
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     *  read_idx-^                   write_idx-^
     *
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Empty
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * read_idx-^ ^-write_idx
     *
     * The tradeoff is we could only use 32KiB of buffer for 16 bit of index.
     * But it should be enough for most of the cases.
     *
     * Ref: http://en.wikipedia.org/wiki/Circular_buffer#Mirroring */
    rt_uint16_t read_mirror : 1;	/**< @brief 读取镜像 */
    rt_uint16_t read_index : 15;	/**< @brief 读取位置 */
    rt_uint16_t write_mirror : 1;	/**< @brief 写入镜像 */
    rt_uint16_t write_index : 15;	/**< @brief 写入位置 */
    /* as we use msb of index as mirror bit, the size should be signed and
     * could only be positive. */
    rt_int16_t buffer_size;			/**< @brief 缓冲区大小 */
};

/**
 * @brief 环形缓冲区状态
 */
enum rt_ringbuffer_state
{
    RT_RINGBUFFER_EMPTY,		/**< 环形缓冲区空 */
    RT_RINGBUFFER_FULL,			/**< 环形缓冲区满 */
    /* half full is neither full nor empty */
    RT_RINGBUFFER_HALFFULL,		/**< 环形缓冲区半满 */
};


/**
 * @brief 环形缓冲区初始化
 *
 * 使用静态环形缓冲区前，需要调用该函数进行初始化。该函数将把用户指定的缓冲区空间的指针传递给环形缓冲区控制块，并初始化环形缓冲区控制块的参数。
 *
 * @param rb ringbuffer 控制块
 * @param pool 地址池指针
 * @param size 地址池的字节数大小
 *
 */
void rt_ringbuffer_init(struct rt_ringbuffer *rb, rt_uint8_t *pool, rt_int16_t size);

/**
 * @brief 复位环形缓冲区
 *
 * 调用此函数将复位环形缓冲区，环形缓冲区控制块中的读写指针被置0。
 *
 * @param rb ringbuffer 控制块
 *
 */
void rt_ringbuffer_reset(struct rt_ringbuffer *rb);

/**
 * @brief 往环形缓冲区中压入数据
 *
 * 调用此函数可以往指定环形缓冲区中写入指定长度的数据，如果剩余空间不足将丢弃剩余数据。
 *
 * @param rb ringbuffer 控制块
 * @param ptr 待压入数据的指针
 * @param length 待压入数据的大小，如果 length 大于剩余空间将丢弃剩余的数据
 *
 * @return 实际写入字节数
 */
rt_size_t rt_ringbuffer_put(struct rt_ringbuffer *rb, const rt_uint8_t *ptr, rt_uint16_t length);

/**
 * @brief 往环形缓冲区中强制压入数据
 *
 * 调用此函数可以往指定环形缓冲区中强制写入指定长度的数据，如果剩余空间不足将覆盖原有数据。
 *
 * @param rb ringbuffer 控制块
 * @param ptr 待压入数据的指针
 * @param length 待压入数据的大小，如果 length 大于剩余空间将丢弃剩余的数据
 *
 * @return 实际写入字节数
 */
rt_size_t rt_ringbuffer_put_force(struct rt_ringbuffer *rb, const rt_uint8_t *ptr, rt_uint16_t length);

/**
 * @brief 往环形缓冲区中压入一个字节
 *
 * 调用此函数可以往指定环形缓冲区中写入一个字节的数据，如果剩余空间不足将写入失败。
 *
 * @param rb ringbuffer 控制块
 * @param ch 待压入数据
 *
 * @return 0, 环形缓冲区已满，压入失败；1 压入成功
 */
rt_size_t rt_ringbuffer_putchar(struct rt_ringbuffer *rb, const rt_uint8_t ch);

/**
 * @brief 往环形缓冲区中强制压入一个字节
 *
 * 调用此函数可以往指定环形缓冲区中强制写入一个字节的数据，如果剩余空间不足将覆盖原有数据。
 *
 * @param rb ringbuffer 控制块
 * @param ch 待压入数据
 *
 * @return 1 压入成功，如果缓冲区已满将覆盖已有数据
 */
rt_size_t rt_ringbuffer_putchar_force(struct rt_ringbuffer *rb, const rt_uint8_t ch);

/**
 * @brief 从环形缓冲区中取出数据
 *
 * 调用此函数可以从环形缓冲区中读取指定长度的数据。
 *
 * @param rb ringbuffer 控制块
 * @param ptr 取出数据的写入地址
 * @param length 待取出数据的大小
 *
 * @return 实际取出数据的字节数
 */
rt_size_t rt_ringbuffer_get(struct rt_ringbuffer *rb, rt_uint8_t *ptr, rt_uint16_t length);

/**
 * @brief 从环形缓冲区中取出一个字节的数据
 *
 * 调用此函数可以从环形缓冲区中读取一个字节的数据。
 *
 * @param rb ringbuffer 控制块
 * @param ch 存储待取出字节的变量
 *
 * @return 0 环形缓冲区已空，取出失败；1 成功取出
 */
rt_size_t rt_ringbuffer_getchar(struct rt_ringbuffer *rb, rt_uint8_t *ch);

/**
 * @brief 获取环形缓冲区中已使用的空间大小
 *
 * 调用此函数可以获取环形缓冲区中已被使用的字节数。
 *
 * @param rb ringbuffer 控制块
 * @param ch 存储待取出字节的变量
 *
 * @return 已使用的大小，0 则表示环形缓冲区已空
 */
rt_size_t rt_ringbuffer_data_len(struct rt_ringbuffer *rb);

#ifdef RT_USING_HEAP

/**
 * @brief 创建环形缓冲区
 *
 * 调用该函数时先创建一个环形缓冲区对象控制块，然后给缓冲区分配一块指定大小的内存空间，并将缓冲区的指针传给控制块，接着初始化环形缓冲区对象控制块。
 *
 * @param length 缓冲区大小
 *
 * @return 创建成功则返回ringbuffer 控制块，创建失败则返回RT_NULL
 */
struct rt_ringbuffer* rt_ringbuffer_create(rt_uint16_t length);

/**
 * @brief 销毁环形缓冲区
 *
 * 调用该函数将释放缓冲区和唤醒缓冲区控制块所占的内存空间。
 *
 * @param rb ringbuffer 控制块
 *
 */
void rt_ringbuffer_destroy(struct rt_ringbuffer *rb);
#endif

/**
 * @brief 获取环形缓冲区大小
 *
 * @param rb ringbuffer 控制块
 *
 * @return 缓冲大小
 */
rt_inline rt_uint16_t rt_ringbuffer_get_size(struct rt_ringbuffer *rb)
{
    RT_ASSERT(rb != RT_NULL);
    return rb->buffer_size;
}

/** 获取环形缓冲区空闲区大小 */
#define rt_ringbuffer_space_len(rb) ((rb)->buffer_size - rt_ringbuffer_data_len(rb))


/**@}*/
			

#ifdef __cplusplus
}
#endif

#endif
