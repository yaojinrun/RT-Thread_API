/*
 * This file is only used for doxygen document generation.
 */

/**
 * @defgroup DRIVER 设备管理
 *
 * @brief 提供访问通用工具或硬件设备的接口，例如 环形缓冲区、管道等通用工具，GPIO、SPI、I2C等硬件设备 
 */

/**
 * @addtogroup DRIVER
 */
/*@{*/

/**
 * @defgroup Device 设备子系统
 * @brief I/O设备子系统
 *
 * 设备系统设计为简单且最小的层，用来协助应用程序和驱动程序之间的通信。
 *
 * 设备系统为驱动程序提供五个接口：
 * - 打开，打开设备
 * - 关闭，关闭设备
 * - 读取，读取设备中的一些数据
 * - 写入，将一些数据写入设备
 * - 控制，向设备发送一些控制命令
 */
 
/**
 * @defgroup ringbuffer 环形缓冲区
 *
 * @brief 环形缓冲区接口
 */

/**
 * @defgroup completion 完成信号量
 *
 * @brief 完成信号量接口
 */

/**
 * @defgroup pipe 管道
 *
 * @brief 管道接口
 */

/**
 * @defgroup dataqueue 数据队列
 *
 * @brief 数据队列接口
 */

/**
 * @defgroup workqueue 工作队列
 *
 * @brief 工作队列接口
 */

/**
 * @defgroup waitqueue 等待队列
 *
 * @brief 等待队列接口
 */

/**
 * @defgroup 串口 UART
 *
 * @brief 串口设备接口
 */

/**
 * @defgroup pin PIN
 *
 * @brief PIN设备接口
 */

/**
 * @defgroup i2c I2C
 *
 * @brief I2C设备接口
 */

/**
 * @defgroup spi SPI
 *
 * @brief SPI设备接口
 */

/**
 * @defgroup rtc RTC
 *
 * @brief RTC设备接口
 */

/**
 * @defgroup alarm Alarm
 *
 * @brief 闹钟接口
 */

/**
 * @defgroup pwm PWM
 *
 * @brief PWM设备接口
 */

/**
 * @defgroup PM 电源管理
 *
 * @brief 电源管理接口
 *
 * PM组件从层次上划分，可以分成用户层、PM 组件层和PM 驱动层。用户层包括了应用代码和驱动代码，它们
 * 通过 API 来决定芯片运行在什么模式。PM 驱动层主要是实现了 PM 驱动的支持以及与 PM 相关的外设功耗
 * 控制。PM 组件层里完成驱动的管理和对用户层提供支持。
 */

/**
 * @defgroup WLAN WLAN 框架
 *
 * @brief WLAN接口
 *
 * WLAN框架是RT-Thread开发的一套用于管理WIFI的中间件。对下连接具体的WIFI驱动，控制WIFI的连接断开，
 * 扫描等操作。对上承载不同的应用，为应用提供WIFI控制，事件，数据导流等操作，为上层应用提供统一
 * 的WIFI控制接口。WLAN框架主要由三个部分组成。DEV驱动接口层，为WLAN框架提供统一的调用接口。Manage
 * 管理层为用户提供WIFI扫描，连接，断线重连等具体功能。Protocol协议负责处理WIFI上产生的数据流，
 * 可根据不同的使用场景挂载不同通讯协议，如LWIP等。具有使用简单，功能齐全，对接方便，兼容性强等特点。
 */

/**
 * @defgroup ulog 日志组件
 *
 * @brief 日志组件接口
 * 
 * ulog 是一个非常简洁、易用的 C/C++ 日志组件，第一个字母 u 代表 μ，即微型的意思。它能做到最低 **ROM<1K,
 * RAM<0.2K** 的资源占用。ulog 不仅有小巧体积，同样也有非常全面的功能，其设计理念参考的是另外一款 C/C++ 
 * 开源日志库：EasyLogger（简称 elog），并在功能和性能等方面做了非常多的改进。主要特性如下：
 * 
 * - 日志输出的 **后端多样化** ，可支持例如：串口、网络，文件、闪存等后端形式；
 * - 日志输出被设计为 **线程安全** 的方式，并支持 **异步输出** 模式；
 * - 日志系统 **高可靠** ，在中断 ISR 、Hardfault 等复杂环境下依旧可用；
 * - 支持 **动态/静态** 开关控制全局的日志输出级别；
 * - 各模块的日志支持 **动态/静态** 设置输出级别；
 * - 日志内容支持按 **关键词及标签** 方式进行全局过滤；
 * - API 和日志格式可兼容 **linux syslog** ；
 * - 支持以 hex 格式 dump 调试数据到日志中；
 * - 兼容 rtdbg （RTT 早期的日志头文件）及 EasyLogger 的日志输出 API。
 *
 *
 * 日志级别代表了日志的重要性，在 ulog 中 **由高到低** ，有如下几个日志级别：
 *
 * | 级别                                 | 名称 |  对应宏     |      描述                                                         |
 * | --------------- |---- | ---- |---------------------------------- |
 * | LOG_LVL_ASSERT  | 断言 |  ASSERT() |发生无法处理、致命性的的错误，以至于系统无法继续运行的断言日志 |
 * | LOG_LVL_ERROR   | 错误 | LOG_E(...) | 发生严重的、 **不可修复** 的错误时输出的日志属于错误级别日志  |
 * | LOG_LVL_WARNING | 警告 |  LOG_W(...) |出现一些不太重要的、具有 **可修复性** 的错误时，会输出这些警告日志 |
 * | LOG_LVL_INFO    | 信息 |  LOG_I(...) |给本模块上层使用人员查看的重要提示信息日志，例如：初始化成功，当前工作状态等。该级别日志一般在量产时依旧 **保留** |
 * | LOG_LVL_DBG     | 调试 |  LOG_D(...) |给本模块开发人员查看的调试日志，该级别日志一般在量产时 **关闭** |
 */

/*@}*/
