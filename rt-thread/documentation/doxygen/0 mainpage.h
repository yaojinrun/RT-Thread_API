/*
 * This file is only used for doxygen document generation.
 */

/**
 * @mainpage RT-Thread介绍
 * @author RT-Thread Development Team
 * @version 3.1.0
 *
 * RT-Thread是一个来自中国的开源物联网操作系统，它提供了非常强的可伸缩能力：
 * 从一个可以运行在ARM Cortex-M0芯片上的极小内核，到中等的ARM Cortex-M3/4/7
 * 系统，甚至是运行于MIPS32、ARM Cortex-A系列处理器上功能丰富系统。
 *
 *
 *
 * ## 简介 ##
 *
 * RT-Thread包含了一个自有的、传统的硬实时内核：可抢占的多任务实时调度器，信号量，
 * 互斥量，邮箱，消息队列，信号等。当然，它和传统的实时操作系统还存在着三种不同：
 * - 设备驱动框架；
 * - 软件组件；
 * - 动态模块；
 *
 * 设备驱动框架更类似一套驱动框架，涉及到UART，IIC，SPI，SDIO，USB从设备/主设备，
 * EMAC，NAND闪存设备等。它会把这些设备驱动中的共性抽象/抽取出来，而驱动工程师
 * 只需要按照固定的模式实现少量的底层硬件操作及板级配置。通过这样的方式，让一个
 * 硬件外设更容易地对接到RT-Thread系统中，并获得RT-Thread平台上的完整软件栈功能。
 *
 * 软件组件是位于RT-Thread内核上的软件单元，例如命令行（finsh/msh shell），虚拟
 * 文件系统（FAT，YAFFS，UFFS，ROM/RAM文件系统等），TCP/IP网络协议栈（lwIP），
 * Libc/POSIX标准层等。一般的，一个软件组件放置于一个目录下，例如RT-Thread/components
 * 目录下的文件夹，并且每个软件组件通过一个 SConscript文件来描述并被添加到RT-Thread的
 * 构建系统中。当系统配置中开启了这一软件组件时，这个组件将被编译并链接到最终的RT-Thread
 * 固件中。
 *
 * 注：随着RT-Thread 3.0中的包管理器开启，越来越多的软件组件将以package方式出现在
 * RT-Thread平台中。而RT-Thread平台更多的是指：
 * - RT-Thread @ref Kernel 内核：
 * - shell命令行；
 * - 虚拟文件系统；
 * - TCP/IP网络协议栈；
 * - 设备驱动框架；
 * - Libc/POSIX标准层。
 *
 * 更多的IoT软件包则以package方式被添加到RT-Thread系统中。
 *
 * 动态模块是一个可动态加载的模块：它可以独立于RT-Thread固件而单独编译。
 * 一般的，每个UA都包含一个main函数入口；一个它自己的对象容器，
 * 用于管理这个应用的任务/信号量/消息队列等内核对象，创建、初始化、销毁等。更多关于UA的信息，
 * 请访问另外一个 git 仓库 了解。
 *
 *
 *
 *
 * ## 支持的芯片架构 ##
 *
 * RT-Thread支持数种芯片体系架构，已经覆盖当前应用中的主流体系架构：
 * - ARM Cortex-M0
 * - ARM Cortex-M3/M4/7
 * - ARM Cortex-R4
 * - ARM Cortex-A8/A9
 * - ARM920T/ARM926 etc
 * - MIPS32
 * - x86
 * - Andes
 * - C-Sky
 * - RISC-V
 * - PowerPC
 *
 *
 *
 *
 * ## 许可证 ##
 *
 * RT-Thread is free software; you can redistribute it and/or modify it under terms of the GNU General
 * Public License version 2 as published by the Free Software Foundation. RT-Thread RTOS is distributed 
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
 * You should have received a copy of the GNU General Public License along with RT-Thread; see file COPYING. 
 * If not, write to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * As a special exception, including RT-Thread RTOS header files in a file, instantiating RT-Thread RTOS 
 * generics or templates, or linking other files with RT-Thread RTOS objects to produce an executable 
 * application, does not by itself cause the resulting executable application to be covered by the GNU General 
 * Public License. This exception does not however invalidate any other reasons why the executable file might 
 * be covered by the GNU Public License.
 *
 * RT-Thread始终以开源的方式发展，所有发布的代码都遵循GPLv2+许可证。注意，GPLv2+的意思是，它是GPLv2的一个修改版本，
 * 添加了一定的例外。简单的来说，当使用RT-Thread时，除了RT-Thread发布的源代码以外的代码，可以不用遵循GPLv2的方式
 * 开源代码。
 */
