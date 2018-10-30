/*
 * This file is only used for doxygen document generation.
 */


/**
 * @addtogroup bsp
 */
/*@{*/

/**
 * @brief �ر�ϵͳ�ж�
 *
 * �˺������Թر�����ϵͳ���жϡ�
 *
 * @return ���ظú�������ǰ��ϵͳ�ж�״̬
 */
rt_base_t rt_hw_interrupt_disable(void);

/**
 * @brief ��ϵͳ�ж�
 *
 * �ú�����Ҫ��rt_hw_intterrupt_disable()�������ʹ�á�
 * ���ָ��˵���rt_hw_interrupt_disable()����ǰ���ж�״̬��
 * �������rt_hw_interrupt_disable()����ǰ�ǹ��ж�״̬����ô���ô˺�������Ȼ�ǹ��ж�״̬��
 *
 * @param level rt_hw_interrupt_disable()�������ص��ж�״̬
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
 * �˺�������ָ�����жϡ�ͨ����ISR׼������ĳ���ж��ź�֮ǰ��������Ҫ�����θ��ж�Դ��
 * ��ISR������״̬�������Ժ󣬼�ʱ�Ĵ�֮ǰ�����ε��ж�Դ��
 *
 * @param vector Ҫ���ε��жϺš�
 *
 * @note ���API�����������ÿһ����ֲ��֧�У�����ͨ��Cortex-M0/M3/M4����ֲ��֧�о�û�����API��
 */
void rt_hw_interrupt_mask(int vector);

/**
 * @brief ��ָ���ж�
 *
 * �ú�����ָ�����жϡ�
 *
 * @param vector Ҫ�򿪵��жϺš�
 *
 * @note ���API�����������ÿһ����ֲ��֧�У�����ͨ��Cortex-M0/M3/M4����ֲ��֧�о�û�����API��
 */
void rt_hw_interrupt_umask(int vector);

/**
 * @brief ��װ�жϴ������
 *
 * �ú�������װָ�����жϴ������
 *
 * @param vector �жϺš�
 * @param handler �жϴ������
 * @param param �жϴ���������
 * @param name �ж�����
 *
 * @note ���API�����������ÿһ����ֲ��֧�У�����ͨ��Cortex-M0/M3/M4����ֲ��֧�о�û�����API��
 */
rt_isr_handler_t rt_hw_interrupt_install(int              vector,
                                         rt_isr_handler_t handler,
                                         void            *param,
                                         char            *name);

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
