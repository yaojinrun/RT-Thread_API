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
 * @brief ���λ������ṹ�嶨��
 */
struct rt_ringbuffer
{
    rt_uint8_t *buffer_ptr;			/**< @brief ������ָ�� */
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
    rt_uint16_t read_mirror : 1;	/**< @brief ��ȡ���� */
    rt_uint16_t read_index : 15;	/**< @brief ��ȡλ�� */
    rt_uint16_t write_mirror : 1;	/**< @brief д�뾵�� */
    rt_uint16_t write_index : 15;	/**< @brief д��λ�� */
    /* as we use msb of index as mirror bit, the size should be signed and
     * could only be positive. */
    rt_int16_t buffer_size;			/**< @brief ��������С */
};

/**
 * @brief ���λ�����״̬
 */
enum rt_ringbuffer_state
{
    RT_RINGBUFFER_EMPTY,		/**< ���λ������� */
    RT_RINGBUFFER_FULL,			/**< ���λ������� */
    /* half full is neither full nor empty */
    RT_RINGBUFFER_HALFFULL,		/**< ���λ��������� */
};


/**
 * @brief ���λ�������ʼ��
 *
 * ʹ�þ�̬���λ�����ǰ����Ҫ���øú������г�ʼ�����ú��������û�ָ���Ļ������ռ��ָ�봫�ݸ����λ��������ƿ飬����ʼ�����λ��������ƿ�Ĳ�����
 *
 * @param rb ringbuffer ���ƿ�
 * @param pool ��ַ��ָ��
 * @param size ��ַ�ص��ֽ�����С
 *
 */
void rt_ringbuffer_init(struct rt_ringbuffer *rb, rt_uint8_t *pool, rt_int16_t size);

/**
 * @brief ��λ���λ�����
 *
 * ���ô˺�������λ���λ����������λ��������ƿ��еĶ�дָ�뱻��0��
 *
 * @param rb ringbuffer ���ƿ�
 *
 */
void rt_ringbuffer_reset(struct rt_ringbuffer *rb);

/**
 * @brief �����λ�������ѹ������
 *
 * ���ô˺���������ָ�����λ�������д��ָ�����ȵ����ݣ����ʣ��ռ䲻�㽫����ʣ�����ݡ�
 *
 * @param rb ringbuffer ���ƿ�
 * @param ptr ��ѹ�����ݵ�ָ��
 * @param length ��ѹ�����ݵĴ�С����� length ����ʣ��ռ佫����ʣ�������
 *
 * @return ʵ��д���ֽ���
 */
rt_size_t rt_ringbuffer_put(struct rt_ringbuffer *rb, const rt_uint8_t *ptr, rt_uint16_t length);

/**
 * @brief �����λ�������ǿ��ѹ������
 *
 * ���ô˺���������ָ�����λ�������ǿ��д��ָ�����ȵ����ݣ����ʣ��ռ䲻�㽫����ԭ�����ݡ�
 *
 * @param rb ringbuffer ���ƿ�
 * @param ptr ��ѹ�����ݵ�ָ��
 * @param length ��ѹ�����ݵĴ�С����� length ����ʣ��ռ佫����ʣ�������
 *
 * @return ʵ��д���ֽ���
 */
rt_size_t rt_ringbuffer_put_force(struct rt_ringbuffer *rb, const rt_uint8_t *ptr, rt_uint16_t length);

/**
 * @brief �����λ�������ѹ��һ���ֽ�
 *
 * ���ô˺���������ָ�����λ�������д��һ���ֽڵ����ݣ����ʣ��ռ䲻�㽫д��ʧ�ܡ�
 *
 * @param rb ringbuffer ���ƿ�
 * @param ch ��ѹ������
 *
 * @return 0, ���λ�����������ѹ��ʧ�ܣ�1 ѹ��ɹ�
 */
rt_size_t rt_ringbuffer_putchar(struct rt_ringbuffer *rb, const rt_uint8_t ch);

/**
 * @brief �����λ�������ǿ��ѹ��һ���ֽ�
 *
 * ���ô˺���������ָ�����λ�������ǿ��д��һ���ֽڵ����ݣ����ʣ��ռ䲻�㽫����ԭ�����ݡ�
 *
 * @param rb ringbuffer ���ƿ�
 * @param ch ��ѹ������
 *
 * @return 1 ѹ��ɹ������������������������������
 */
rt_size_t rt_ringbuffer_putchar_force(struct rt_ringbuffer *rb, const rt_uint8_t ch);

/**
 * @brief �ӻ��λ�������ȡ������
 *
 * ���ô˺������Դӻ��λ������ж�ȡָ�����ȵ����ݡ�
 *
 * @param rb ringbuffer ���ƿ�
 * @param ptr ȡ�����ݵ�д���ַ
 * @param length ��ȡ�����ݵĴ�С
 *
 * @return ʵ��ȡ�����ݵ��ֽ���
 */
rt_size_t rt_ringbuffer_get(struct rt_ringbuffer *rb, rt_uint8_t *ptr, rt_uint16_t length);

/**
 * @brief �ӻ��λ�������ȡ��һ���ֽڵ�����
 *
 * ���ô˺������Դӻ��λ������ж�ȡһ���ֽڵ����ݡ�
 *
 * @param rb ringbuffer ���ƿ�
 * @param ch �洢��ȡ���ֽڵı���
 *
 * @return 0 ���λ������ѿգ�ȡ��ʧ�ܣ�1 �ɹ�ȡ��
 */
rt_size_t rt_ringbuffer_getchar(struct rt_ringbuffer *rb, rt_uint8_t *ch);

/**
 * @brief ��ȡ���λ���������ʹ�õĿռ��С
 *
 * ���ô˺������Ի�ȡ���λ��������ѱ�ʹ�õ��ֽ�����
 *
 * @param rb ringbuffer ���ƿ�
 * @param ch �洢��ȡ���ֽڵı���
 *
 * @return ��ʹ�õĴ�С��0 ���ʾ���λ������ѿ�
 */
rt_size_t rt_ringbuffer_data_len(struct rt_ringbuffer *rb);

#ifdef RT_USING_HEAP

/**
 * @brief �������λ�����
 *
 * ���øú���ʱ�ȴ���һ�����λ�����������ƿ飬Ȼ�������������һ��ָ����С���ڴ�ռ䣬������������ָ�봫�����ƿ飬���ų�ʼ�����λ�����������ƿ顣
 *
 * @param length ��������С
 *
 * @return �����ɹ��򷵻�ringbuffer ���ƿ飬����ʧ���򷵻�RT_NULL
 */
struct rt_ringbuffer* rt_ringbuffer_create(rt_uint16_t length);

/**
 * @brief ���ٻ��λ�����
 *
 * ���øú������ͷŻ������ͻ��ѻ��������ƿ���ռ���ڴ�ռ䡣
 *
 * @param rb ringbuffer ���ƿ�
 *
 */
void rt_ringbuffer_destroy(struct rt_ringbuffer *rb);
#endif

/**
 * @brief ��ȡ���λ�������С
 *
 * @param rb ringbuffer ���ƿ�
 *
 * @return �����С
 */
rt_inline rt_uint16_t rt_ringbuffer_get_size(struct rt_ringbuffer *rb)
{
    RT_ASSERT(rb != RT_NULL);
    return rb->buffer_size;
}

/** ��ȡ���λ�������������С */
#define rt_ringbuffer_space_len(rb) ((rb)->buffer_size - rt_ringbuffer_data_len(rb))


/**@}*/
			

#ifdef __cplusplus
}
#endif

#endif
