#ifndef COMPLETION_H_
#define COMPLETION_H_

#include <rtthread.h>

/**
 * Completion �ṹ��
 */

struct rt_completion
{
    rt_uint32_t flag;	/**< @brief completion�Ĳ��� */

    /* suspended list */
    rt_list_t suspended_list;	/**< @brief �����ڸ�completion�ϵ��߳� */
};


/**
 * @addtogroup completion
 */

/**@{*/

/**
 * @brief ��ʼ��completion
 *
 * �ú�������ʼ��һ�� completion ����
 *
 * @param completion completion ���ƿ�
 *
 */
void rt_completion_init(struct rt_completion *completion);

/**
 * @brief �ȴ� completion
 *
 * �ú�����ʹ�����̹߳���ָ���ĵȴ�ʱ��ȴ�completion�źš�
 *
 * @param completion completion ���
 * @param timeout ָ���ĳ�ʱʱ��
 *
 * @return RT_EOK �ȵ� completion �ź�, -RT_ETIMEOUT �ȴ���ʱ
 */
rt_err_t rt_completion_wait(struct rt_completion *completion,
                            rt_int32_t            timeout);

/**
* @brief ���� completion�ź�
 *
 * �ú�����ʹ�����̹߳���ָ���ĵȴ�ʱ��ȴ�completion�źš�
 *
 * @param completion completion ���
 *
 */
void rt_completion_done(struct rt_completion *completion);


/**@}*/

#endif