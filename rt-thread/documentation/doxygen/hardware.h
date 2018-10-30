/*
 * This file is only used for doxygen document generation.
 */


/**
 * @addtogroup bsp
 */
/*@{*/

/**
 * @brief 禁用系统中断
 *
 * 此功能将返回当前系统中断状态并禁用系统中断。
 *
 * @return 当前系统中断状态
 */
rt_base_t rt_hw_interrupt_disable(void);

/**
 * @brief 打开系统中断
 *
 * 该功能将设置指定的中断状态，该状态应由rt_hw_intterrupt_disable函数保存。 
 * 如果保存的中断状态是中断打开，该功能将打开系统中断状态。
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
 * 此函数屏蔽指定的中断。
 *
 * @param vector 要屏蔽的中断号。
 *
 * @note 并非所有平台都提供此函数。
 */
void rt_hw_interrupt_mask(int vector);

/**
 * @brief 打开指定中断
 *
 * 该函数打开指定的中断。
 *
 * @param vector 要打开的中断号。
 *
 * @note 并非所有平台都提供此函数。
 */
void rt_hw_interrupt_umask(int vector);

/**
 * @brief 安装中断处理程序
 *
 * 该函数将安装指定的中断处理程序。
 *
 * @param vector 要安装的中断号。
 * @param new_handler 新的中断处理程序。
 * @param old_handler 旧的中断处理程序。 该参数可以是RT_NULL。
 *
 * @note 并非所有平台都提供此函数。
 */
void rt_hw_interrupt_install(int vector, rt_isr_handler_t new_handler,
		rt_isr_handler_t *old_handler);

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
