/*
 * This file is only used for doxygen document generation.
 */

/**
 * @defgroup DFS 文件系统
 *
 * @brief DFS 是RT-Thread系统里的虚拟文件系统接口
 *
 * DFS是一种抽象的文件机制，RT-Thread中对文件系统的相关操作系统实际上都通过
 * 操作DFS实现，也就是说DFS是对各具体文件系统的抽象。DFS使得其他部分无须关心
 * 不同文件系统之间的差异，使得RT-Thread可以支持多种类型的文件系统。
 *
 * RT-Thread DFS虚拟文件系统的结构如图 **DFS虚拟文件系统结构图** 所示：
 *
 * @image html dfs.png "DFS虚拟文件系统结构图"
 *
 * DFS虚拟文件系统向用户应用程序提供POSIX接口层，POSIX接口层是给开发者使用的接口
 * 函数层。开发者使用POSIX接口层提供的POSIX API进行文件操作，无需关心文件系统是
 * 如何实现的，也无需关心数据是存放在哪个存储设备中。
 *
 * DFS虚拟文件系统有如下功能特点：
 * 1.	支持多种类型的存储设备。
 * 2.	支持多种类型的文件系统，提供普通文件、设备文件、网络文件描述符的管理。
 * 3.	提供统一的POSIX文件操作接口：read、write、poll/select等。
 */

/**
 * @addtogroup DFS
 */
/*@{*/

/**
 * @defgroup Fd 文件描述符
 *
 * @brief 文件描述符相关API的介绍说明
 */

/**
 * @defgroup FsApi 文件系统
 *
 * @brief 文件系统相关API的说明
 */

/*
 * @defgroup FileApi 
 */

/**
 * @defgroup FsPosixApi 文件和目录
 *
 * @brief 文件和目录操作相关API的说明
 */

/*@}*/
