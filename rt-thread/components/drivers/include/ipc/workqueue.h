#ifndef WORKQUEUE_H__
#define WORKQUEUE_H__

#include <rtthread.h>

/**
 * @addtogroup workqueue
 */

/**@{*/

/* workqueue implementation */
/**
 * @brief �������еĽṹ�嶨��
 */
struct rt_workqueue
{
    rt_list_t      work_list;		/**< @brief ������������ */
    struct rt_work *work_current; 	/**< @brief ��ǰ��������ָ�� */

    struct rt_semaphore sem; 		/**< @brief ����ͬ�����ź��� */
    rt_thread_t    work_thread;		/**< @brief �������д����߳� */
};

/**
 * @brief �����Ľṹ�嶨��
 */
struct rt_work
{
    rt_list_t list;

    void (*work_func)(struct rt_work* work, void* work_data);
    void *work_data;
};

#ifdef RT_USING_HEAP


/**
 * @brief ������������
 *
 * ���øú����ӿں��ں����ȴӶ�̬�ڴ���з���һ���������п��ƿ飬Ȼ��Ըÿ��ƿ���л����ĳ�ʼ����������һ���������д����̡߳�
 *
 * @param name ������������
 * @param stack_size �������ж�ջ�ռ��С
 * @param priority �������е����ȼ�
 *
 * @return �����ɹ��򷵻�workqueue���ƿ飬ʧ���򷵻� RT_NULL
 */
struct rt_workqueue *rt_workqueue_create(const char* name, rt_uint16_t stack_size, rt_uint8_t priority);

/**
 * @brief ���ٹ�������
 *
 * ������������ӿں�ϵͳ��ɾ������������еĴ����̣߳�Ȼ���ͷ���Ӧ�Ĺ������п��ƿ�ռ�е��ڴ档
 *
 * @param queue workqueue���ƿ�
 *
 * @return RT_EOK
 */
rt_err_t rt_workqueue_destroy(struct rt_workqueue* queue);

/**
 * @brief ���ӹ�����������
 *
 * ���øú���������ָ���Ĺ����������������
 *
 * @param queue workqueue���ƿ�
 * @param work ������ƿ�
 *
 * @return -RT_EBUSY ��������ִ���У������ظ�ִ�У�RT_EOK ����workqueue����ɹ�
 */
rt_err_t rt_workqueue_dowork(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief ȡ��������������
 *
 * ���øú�������ȡ�������������δִ�е�����
 *
 * @param queue workqueue���ƿ�
 * @param work ������ƿ�
 *
 * @return -RT_EBUSY ��������ִ���У�����ȡ����RT_EOK ȡ���ɹ�
 */
rt_err_t rt_workqueue_cancel_work(struct rt_workqueue* queue, struct rt_work* work);

/**
 * @brief ͬ��ȡ��������������
 *
 * ���øú�������ȡ��ѭ�����������������ִ���У���ȴ��ô�������ɺ�ȡ����
 *
 * @param queue workqueue���ƿ�
 * @param work ������ƿ�
 *
 * @return ȡ���ɹ��������������ִ���У��ȴ�������ɺ�ȡ����
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

