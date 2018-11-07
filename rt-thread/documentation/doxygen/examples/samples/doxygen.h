/*
 * This file is only used for doxygen document generation.
 */

/**
 * @defgroup example ʾ������
 *
 * @brief �ṩ�ںˡ��ļ�ϵͳ������������ʹ������
 */
 
/**
 * @addtogroup example
 */
/*@{*/

/**
 * @defgroup kernel_example �ں�����
 *
 * @brief �ں˽ӿڵ�ʹ�����̡�
 */
 
/**
 * @defgroup filesystem_example �ļ�ϵͳ����
 *
 * @brief �ļ�ϵͳ�ӿڵ�ʹ������
 */

/**
 * @defgroup network_example ��������
 *
 * @brief ����ӿڵ�ʹ������
 *
 */
 
/**
 * @defgroup peripheral-sample ��������
 *
 * @brief ����ӿڵ�ʹ������
 *
 */

/*@}*/


/**
 * @addtogroup kernel_example
 */
/*@{*/

/**
 * @defgroup thread �߳�
 * @brief ��������ʼ��/�����߳�
 *
 * **Դ��**
 * @include thread_sample.c
 * @example thread_sample.c
 *
 */

/**
 * @defgroup timeslice ʱ��Ƭ����
 * @brief ��ͬ���ȼ��̰߳���ʱ��Ƭ�ַ�����
 * **Դ��**
 * @include timeslice_sample.c
 * @example timeslice_sample.c
 */
 
/**
 * @defgroup timer ��ʱ��
 * @brief ���ζ�ʱ���������Զ�ʱ����ʹ��
 *
 * **Դ��**
 * @include timer_sample.c
 * @example timer_sample.c
 */

/**
 * @defgroup interrupt �ж�
 * @brief �ر��жϽ���ȫ�ֱ����ķ���
 *
 * **Դ��**
 * @include interrupt_sample.c
 * @example interrupt_sample.c
 */

/**
 * @defgroup signal �ź�
 * @brief ���̷߳����ź�
 *
 * **Դ��**
 * @include signal_sample.c
 * @example signal_sample.c
 */

/**
 * @defgroup dynmem ��̬�ڴ�
 * @brief �̶߳�̬�����ڴ沢�ͷ�
 *
 * **Դ��**
 * @include dynmem_sample.c
 * @example dynmem_sample.c
 */

/**
 * @defgroup memp �ڴ��
 * @brief �̴߳��ڴ���л���ڴ�����ͷ��ڴ��
 *
 * **Դ��**
 * @include memp_sample.c
 * @example memp_sample.c
 */

/**
 * @defgroup idlehook �����̹߳��Ӻ���
 * @brief ��ӡ�����̹߳��Ӻ��������д���
 *
 * **Դ��**
 * @include idlehook_sample.c
 * @example idlehook_sample.c
 */
 
/**
 * @defgroup scheduler ���������Ӻ���
 * @brief �ڵ����������д�ӡ�߳��л���Ϣ
 *
 * **Դ��**
 * @include scheduler_hook.c
 * @example scheduler_hook.c
 */

/**
 * @defgroup semaphore �ź���
 * @brief һ���̷߳����ź���������һ���߳̽����ź���
 *
 * **Դ��**
 * @include semaphore_sample.c
 * @example semaphore_sample.c
 *
 */

/**
 * @defgroup producer ������������ģ��
 * @brief �������߳����������߳�ʹ���ź���ͬ��
 *
 * **Դ��**
 * @include producer_consumer.c
 * @example producer_consumer.c
 */

/**
 * @defgroup mutex ������
 * @brief �߳�ʹ�û���������������Դ
 *
 * **Դ��**
 * @include mutex_sample.c
 * @example mutex_sample.c
 *
 */

/**
 * @defgroup priority ���ȼ���ת
 * @brief �����������߳����ȼ���ת����
 *
 * **Դ��**
 * @include priority_inversion.c
 * @example priority_inversion.c
 */
 
/**
 * @defgroup event �¼�
 * @brief һ���̵߳ȴ��¼�����һ���̷߳����¼�
 *
 * **Դ��**
 * @include event_sample.c
 * @example event_sample.c
 */

/**
 * @defgroup mailbox ����
 * @brief һ���߳��������з����ʼ�������һ���߳�����������ȡ�ʼ�
 *
 * **Դ��**
 * @include mailbox_sample.c
 * @example mailbox_sample.c
 */

/**
 * @defgroup msgq ��Ϣ����
 * @brief һ���̻߳����Ϣ��������ȡ��Ϣ����һ���̻߳ᶨʱ����Ϣ���з�����ͨ��Ϣ�ͽ�����Ϣ
 * 
 *
 * **Դ��**
 * @include msgq_sample.c
 * @example msgq_sample.c
 */

/*@}*/


/**
 * @addtogroup filesystem_example
 */
/*@{*/
 
/**
 * @defgroup mkdir ����Ŀ¼
 * @brief ����Ŀ¼
 *
 * **Դ��**
 * @include mkdir_sample.c
 * @example mkdir_sample.c
 */

/**
 * @defgroup opendir ��Ŀ¼
 * @brief �򿪺͹ر�Ŀ¼
 *
 * **Դ��**
 * @include opendir_sample.c
 * @example opendir_sample.c
 */

/**
 * @defgroup readdir ��ȡĿ¼
 * @brief ��ȡĿ¼
 *
 * **Դ��**
 * @include readdir_sample.c
 * @example readdir_sample.c
 */

/**
 * @defgroup readwrite �ļ���д
 * @brief ���ļ����ļ���д
 *
 * **Դ��**
 * @include readwrite_sample.c
 * @example readwrite_sample.c
 */

/**
 * @defgroup rename �ļ�������
 * @brief �ļ�������
 *
 * **Դ��**
 * @include rename_sample.c
 * @example rename_sample.c
 */

/**
 * @defgroup stat �ļ�״̬
 * @brief �鿴�ļ���С
 *
 * **Դ��**
 * @include stat_sample.c
 * @example stat_sample.c
 */

/**
 * @defgroup tell_seek_dir Ŀ¼λ�ò���
 * @brief ���������ö�ȡĿ¼λ��
 *
 * **Դ��**
 * @include tell_seek_dir_sample.c
 * @example tell_seek_dir_sample.c
 */

/*@}*/


/**
 * @addtogroup network_example
 */
/*@{*/
 
 /**
 * @defgroup tcpclient_sample tcp �ͻ���
 * @brief tcp �ͻ�������
 *
 * **Դ��**
 * @include tcpclient_sample.c
 * @example tcpclient_sample.c
 */

 /**
 * @defgroup udpclient_sample udp �ͻ���
 * @brief udp �ͻ���
 *
 * **Դ��**
 * @include udpclient_sample.c
 * @example udpclient_sample.c
 */


 /**
 * @defgroup httpclient_sample http �ͻ���
 * @brief http �ͻ���
 *
 * **Դ��**
 * @include httpclient_sample.c
 * @example httpclient_sample.c
 */


 /**
 * @defgroup tcpclient_select_sample ���� select ʵ�ֵ� tcp �ͻ���
 * @brief ���� select ʵ�ֵ� tcp �ͻ���
 *
 * **Դ��**
 * @include tcpclient_select_sample.c
 * @example tcpclient_select_sample.c
 */


 /**
 * @defgroup tcpserver_sample tcp �����
 * @brief tcp �����
 *
 * **Դ��**
 * @include tcpserver_sample.c
 * @example tcpserver_sample.c
 */
 

 /**
 * @defgroup udpserver_sample udp �����
 * @brief udp �����
 *
 * **Դ��**
 * @include udpserver_sample.c
 * @example udpserver_sample.c
 */


/*@}*/


/**
 * @addtogroup peripheral-sample
 */
/*@{*/

/**
 * @defgroup i2c_aht10 I2C
 * @brief ͨ�� I2C �豸��ȡ��ʪ�ȴ����� aht10 ����ʪ������
 *
 * **Դ��**
 * @include i2c_aht10_sample.c
 * @example i2c_aht10_sample.c
 */

/**
 * @defgroup iwdg Watchdog
 * @brief �������Ź��豸ʹ��
 *
 * **Դ��**
 * @include iwdg_sample.c
 * @example iwdg_sample.c
 */

/**
 * @defgroup led_blink Led
 * @brief ͨ��PIN�豸����ӿڿ���LED����
 *
 * **Դ��**
 * @include led_blink_sample.c
 * @example led_blink_sample.c
 */

/**
 * @defgroup pin_beep PIN
 * @brief ͨ���������Ʒ�������Ӧ���ŵĵ�ƽ״̬�������Ʒ�����
 *
 * **Դ��**
 * @include pin_beep_sample.c
 * @example pin_beep_sample.c
 */

/**
 * @defgroup sd_sample ���豸SD��
 * @brief SD���豸��ʹ��
 *
 * **Դ��**
 * @include sd_sample.c
 * @example sd_sample.c
 */

/**
 * @defgroup spi_w25q SPI
 * @brief ͨ��SPI�豸��ȡ w25q �� ID ����
 *
 * **Դ��**
 * @include spi_w25q_sample.c
 * @example spi_w25q_sample.c
 */
 
/**
 * @defgroup uart_sample UART
 * @brief ͨ����������ַ���"hello RT-Thread!"��Ȼ���λ���������ַ�
 *
 * **Դ��**
 * @include uart_sample.c
 * @example uart_sample.c
 */
 
/*@}*/