#ifndef DATAQUEUE_H__
#define DATAQUEUE_H__

#include <rtthread.h>

/**
 * @addtogroup dataqueue
 */

/**@{*/

/*
 * ���ݶ����¼�����
 */
#define RT_DATAQUEUE_EVENT_UNKNOWN   0x00            /**< @brief δ֪���ݶ����¼� */
#define RT_DATAQUEUE_EVENT_POP       0x01            /**< @brief ���ݶ���ȡ���¼� */
#define RT_DATAQUEUE_EVENT_PUSH      0x02            /**< @brief ���ݶ���д���¼� */
#define RT_DATAQUEUE_EVENT_LWM       0x03            /**< @brief ���ݶ������ﵽ�趨��ֵ�¼� */

struct rt_data_item;
#define RT_DATAQUEUE_SIZE(dq)        ((dq)->put_index - (dq)->get_index)	            /**< @brief ���ݶ���ʹ������ */
#define RT_DATAQUEUE_EMPTY(dq)       ((dq)->size - RT_DATAQUEUE_SIZE(dq))            	/**< @brief ���ݶ��п������� */
/* data queue implementation */
/**
 * @brief data queue �ṹ��
 */
struct rt_data_queue
{
    rt_uint16_t size;					/**< @brief ���ݶ��е����� */
    rt_uint16_t lwm;					/**< @brief ��ֵ */
    rt_bool_t   waiting_lwm;			

    rt_uint16_t get_index;				/**< @brief  ���ݶ��ж�ȡλ��*/
    rt_uint16_t put_index;				/**< @brief  ���ݶ���д��λ��*/

    struct rt_data_item *queue;			/**< @brief  ���ݶ����е�������Ϣ����������ָ��ʹ�С��Ϣ*/

    rt_list_t suspended_push_list;		/**< @brief  ���ݶ���д���̹߳�������*/
    rt_list_t suspended_pop_list;		/**< @brief  ���ݶ��ж�ȡ�̹߳�������*/

    /* event notify */
    void (*evt_notify)(struct rt_data_queue *queue, rt_uint32_t event);	/**< @brief �¼�֪ͨ�ص����� */
};

/*
 * DataQueue for DeviceDriver
 */
/**
 * @brief ���ݶ��г�ʼ��
 *
 * ���øú�������ʼ�����ݶ��п��ƿ飬���趨֪ͨ�ص�������
 *
 * @param queue ���ݶ��о��
 * @param size ���ݶ��п��С
 * @param lwm ��ֵ�趨
 * @param evt_notify ֪ͨ�¼��ص�����
 *
 * @return -RT_ENOMEM �ڴ治������ʧ�ܣ� RT_EOK ��ʼ���ɹ�
 */
rt_err_t rt_data_queue_init(struct rt_data_queue *queue,
                            rt_uint16_t           size,
                            rt_uint16_t           lwm,
                            void (*evt_notify)(struct rt_data_queue *queue, rt_uint32_t event));

/**
 * @brief �����ݶ�����д������
 *
 * ���øú���������ָ�������ݶ�����д�����ݣ������ݶ�������������ָ���ĳ�ʱʱ���ڹ���ȴ���
 *
 * @param queue ���ݶ��о��
 * @param data_ptr ��д�������ָ��
 * @param data_size ��д�����ݵ��ֽ���
 * @param timeout ָ����ʱʱ��
 *
 * @return -RT_ETIMEOUT �����������ȴ���ʱ��RT_EOK д��ɹ�
 */
rt_err_t rt_data_queue_push(struct rt_data_queue *queue,
                            const void           *data_ptr,
                            rt_size_t             data_size,
                            rt_int32_t            timeout);
							
/**
 * @brief ȡ�����ݶ����е�����
 *
 * ���øú������Դ�ָ�������ݶ�����ȡ�����ݣ�������ݶ���Ϊ�գ�����ָ����ʱ���ڹ���ȴ���
 *
 * @param queue ���ݶ��о��
 * @param data_ptr ��ȡ��������ָ��
 * @param data_size ��ȡ�����ݵ��ֽ���
 * @param timeout ָ����ʱʱ��
 *
 * @return -RT_ETIMEOUT �����ѿգ��ȴ���ʱ��RT_EOK ȡ���ɹ�
 */
rt_err_t rt_data_queue_pop(struct rt_data_queue *queue,
                           const void          **data_ptr,
                           rt_size_t            *size,
                           rt_int32_t            timeout);

/**
 * @brief ȡ�����������ݶ����е�����
 *
 * ���øú������Դ�ָ�������ݶ�����ȡ�����ݣ�������ݶ���Ϊ�գ�����ָ����ʱ���ڹ���ȴ���
 *
 * @param queue ���ݶ��о��
 * @param data_ptr ��ȡ��������ָ��
 * @param data_size ��ȡ�����ݵ��ֽ���
 * @param timeout ָ����ʱʱ��
 *
 * @return --RT_EEMPTY �����ѿգ�RT_EOK ȡ���ɹ�
 */
rt_err_t rt_data_queue_peak(struct rt_data_queue *queue,
                            const void          **data_ptr,
                            rt_size_t            *size);
							

/**
 * @brief �������ݶ���
 *
 * ���øú������������ݶ����Ϲ���ȴ��������̡߳�
 *
 * @param queue ���ݶ��о��
 *
 */
void rt_data_queue_reset(struct rt_data_queue *queue);

/**@}*/
							
#endif
