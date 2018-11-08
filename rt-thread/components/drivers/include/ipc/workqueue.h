#ifndef WORKQUEUE_H__
#define WORKQUEUE_H__

#include <rtthread.h>

/**
 * @addtogroup workqueue
 */

/**@{*/

/* workqueue implementation */
/**
 * @brief �������п��ƿ�
 */
struct rt_workqueue
{
    rt_list_t      work_list;		/**< @brief ������������ */
    struct rt_work *work_current; 	/**< @brief ��ǰ��������ָ�� */

    struct rt_semaphore sem; 		/**< @brief ����ͬ�����ź��� */
    rt_thread_t    work_thread;		/**< @brief �������д����߳� */
};

/**
 * @brief �����ص�������Ϣ
 */
struct rt_work
{
    rt_list_t list;					/**< @brief �������� */

    void (*work_func)(struct rt_work* work, void* work_data);	/**< @brief �������Ļص����� */
    void *work_data;				/**< @brief �ص�������ص�����ָ�� */
};

#ifdef RT_USING_HEAP


/**
 * @brief ������������
 *
 * ���øú����ӿں��ں����ȴӶ�̬�ڴ���з���һ���������п��ƿ飬Ȼ��Ըÿ��ƿ���л����ĳ�ʼ����������һ���������д����̡߳�
 *
 * @param name ������������
 * @param stack_size �������д����̶߳�ջ�ռ��С
 * @param priority �������д����̵߳����ȼ�
 *
 * @return �����ɹ��򷵻ع������о����ʧ���򷵻� RT_NULL
 */
struct rt_workqueue *rt_workqueue_create(const char* name, rt_uint16_t stack_size, rt_uint8_t priority);

/**
 * @brief ���ٹ�������
 *
 * ������������ӿں�ϵͳ��ɾ������������еĴ����̣߳�Ȼ���ͷ���Ӧ�Ĺ������п��ƿ�ռ�е��ڴ档
 *
 * @param queue �������о��
 *
 * @return RT_EOK
 */
rt_err_t rt_workqueue_destroy(struct rt_workqueue* queue);

/**
 * @brief ���ӹ�����������
 *
 * ���øú���������ָ���Ĺ����������������
 *
 * @param queue �������о��
 * @param work ������ƿ�
 *
 * @return RT_EOK ��������ɹ���-RT_EBUSY ��������ִ���У������ظ�ִ�С�
 */
rt_err_t rt_workqueue_dowork(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief ȡ��������������
 *
 * ���øú�������ȡ�������������δִ�е�����
 *
 * @param queue �������о��
 * @param work ������ƿ�
 *
 * @return RT_EOK ȡ���ɹ���-RT_EBUSY ��������ִ���У�����ȡ����
 */
rt_err_t rt_workqueue_cancel_work(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief ͬ��ȡ��������������
 *
 * ���øú�������ȡ��ѭ�����������������ִ���У���ȴ��ô�������ɺ�ȡ����
 *
 * @param queue �������о��
 * @param work ������ƿ�
 *
 * @return RT_EOK�������������ִ���У��ȴ�������ɺ�ȡ����
 */
rt_err_t rt_workqueue_cancel_work_sync(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief ��ʼ��work
 *
 * ���øú������Գ�ʼ��ָ��work��������ָ������ص������ʹ����������ָ�롣
 *
 * @param work work���ƿ�
 * @param work_func work�ص�������
 * @param work_data ����������ݻ�����ָ��
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

