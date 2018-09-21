/*
 * This file is only used for doxygen document generation.
 */


/**
 * @ingroup SystemInit
 *
 * @brief 创建用户线程
 *
 * 由此创建一个用户main()线程，而 main()函数是RT-Thread的用户代码入口，
 * 用户可以在main()函数里添加自己的应用。
 */
void rt_application_init()
{
}

/**
 * @ingroup SystemInit
 *
 * @brief 初始化系统堆内存。
 *
 * @param begin_addr 系统堆内存的起始地址。
 * @param end_addr 系统堆内存的结束地址。
 *
 */
void rt_system_heap_init(void* begin_addr, void* end_addr)
{
}
