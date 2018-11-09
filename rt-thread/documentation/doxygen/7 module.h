/*
 * This file is only used for doxygen document generation.
 */


/**
 * @defgroup Module 动态模块
 *
 * @brief 动态模块接口
 * 
 * 在传统桌面操作系统中，用户空间和内核空间是分开的，应用程序运行在用户空间，内核以及内核模块
 * 则运行于内核空间，其中内核模块可以动态加载与删除以扩展内核功能。`dlmodule` 则是RT-Thread下，
 * 在内核空间对外提供的动态模块加载机制的软件组件。在RT-Thread v3.1.0以前的版本中，这也称之为
 * 应用模块（Application Module），在RT-Thread v3.1.0及之后，则回归传统，以动态模块命名。
 * 
 * `dlmodule`组件更多的是一个ELF格式加载器，把单独编译的一个elf文件的代码段，数据段加载到内存中，
 * 并对其中的符号进行解析，绑定到内核导出的API地址上。动态模块elf文件主要放置于RT-Thread下的文件
 * 系统上。
 */
