/*
 * This file is only used for doxygen document generation.
 */


/**
 * @addtogroup bsp
 */
/*@{*/

/**
 * @brief 关闭系统中断
 *
 * 此函数可以关闭整个系统的中断。
 *
 * @return 返回该函数运行前的系统中断状态
 */
rt_base_t rt_hw_interrupt_disable(void);

/**
 * @brief 打开系统中断
 *
 * 该函数需要和rt_hw_intterrupt_disable()函数配对使用。
 * 它恢复了调用rt_hw_interrupt_disable()函数前的中断状态。
 * 如果调用rt_hw_interrupt_disable()函数前是关中断状态，那么调用此函数后依然是关中断状态。
 *
 * @param level rt_hw_interrupt_disable()函数返回的中断状态
 */
void rt_hw_interrupt_enable(rt_base_t level);

/**
 * @brief 初始化中断
 *
 * 该函数初始化中断。
 */
void rt_hw_interrupt_init(void);

/**
 * @brief 禁用指定中断
 *
 * 此函数屏蔽指定的中断。通常在ISR准备处理某个中断信号之前，我们需要先屏蔽该中断源，
 * 在ISR处理完状态或数据以后，及时的打开之前被屏蔽的中断源。
 *
 * @param vector 要屏蔽的中断号。
 *
 * @note 这个API并不会出现在每一个移植分支中，例如通常Cortex-M0/M3/M4的移植分支中就没有这个API。
 */
void rt_hw_interrupt_mask(int vector);

/**
 * @brief 打开指定中断
 *
 * 该函数打开指定的中断。
 *
 * @param vector 要打开的中断号。
 *
 * @note 这个API并不会出现在每一个移植分支中，例如通常Cortex-M0/M3/M4的移植分支中就没有这个API。
 */
void rt_hw_interrupt_umask(int vector);

/**
 * @brief 安装中断处理程序
 *
 * 该函数将安装指定的中断处理程序。
 *
 * @param vector 中断号。
 * @param handler 中断处理程序。
 * @param param 中断处理程序参数
 * @param name 中断名称
 *
 * @note 这个API并不会出现在每一个移植分支中，例如通常Cortex-M0/M3/M4的移植分支中就没有这个API。
 */
rt_isr_handler_t rt_hw_interrupt_install(int              vector,
                                         rt_isr_handler_t handler,
                                         void            *param,
                                         char            *name);

/*
 * @brief 重启CPU
 *
 * 此功能将重置整个平台。
 */
void rt_hw_cpu_reset(void);

/*
 * @brief 停止CPU
 *
 * 此功能将停止整个平台。
 */
void rt_hw_cpu_shutdown(void);

/*@}*/
