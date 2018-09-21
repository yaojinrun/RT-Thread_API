/*
 * This file is only used for doxygen document generation.
 */


/**
 * @addtogroup bsp
 */
/*@{*/

/**
 * @brief ����ϵͳ�ж�
 *
 * �˹��ܽ����ص�ǰϵͳ�ж�״̬������ϵͳ�жϡ�
 *
 * @return ��ǰϵͳ�ж�״̬
 */
rt_base_t rt_hw_interrupt_disable(void);

/**
 * @brief ��ϵͳ�ж�
 *
 * �ù��ܽ�����ָ�����ж�״̬����״̬Ӧ��rt_hw_intterrupt_disable�������档 
 * ���������ж�״̬���жϴ򿪣��ù��ܽ���ϵͳ�ж�״̬��
 */
void rt_hw_interrupt_enable(rt_base_t level);

/**
 * @brief ��ʼ���ж�
 *
 * �ú�����ʼ���жϡ�
 */
void rt_hw_interrupt_init(void);

/**
 * @brief ����ָ���ж�
 *
 * �˺�������ָ�����жϡ�
 *
 * @param vector Ҫ���ε��жϺš�
 *
 * @note ��������ƽ̨���ṩ�˺�����
 */
void rt_hw_interrupt_mask(int vector);

/**
 * @brief ��ָ���ж�
 *
 * �ú�����ָ�����жϡ�
 *
 * @param vector Ҫ�򿪵��жϺš�
 *
 * @note ��������ƽ̨���ṩ�˺�����
 */
void rt_hw_interrupt_umask(int vector);

/**
 * @brief ��װ�жϴ������
 *
 * �ú�������װָ�����жϴ������
 *
 * @param vector Ҫ��װ���жϺš�
 * @param new_handler �µ��жϴ������
 * @param old_handler �ɵ��жϴ������ �ò���������RT_NULL��
 *
 * @note ��������ƽ̨���ṩ�˺�����
 */
void rt_hw_interrupt_install(int vector, rt_isr_handler_t new_handler,
		rt_isr_handler_t *old_handler);

/*
 * @brief ����CPU
 *
 * �˹��ܽ���������ƽ̨��
 */
void rt_hw_cpu_reset(void);

/*
 * @brief ֹͣCPU
 *
 * �˹��ܽ�ֹͣ����ƽ̨��
 */
void rt_hw_cpu_shutdown(void);

/*@}*/
