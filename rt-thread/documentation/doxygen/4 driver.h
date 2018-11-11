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
 * @defgroup UART UART
 *
 * @brief UART设备接口
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
 * @brief 闹钟设备接口
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
 * @defgroup WLAN WLAN框架
 *
 * @brief WLAN框架接口
 *
 * WLAN框架是RT-Thread开发的一套用于管理WIFI的中间件。对下连接具体的WIFI驱动，控制WIFI的连接断开，
 * 扫描等操作。对上承载不同的应用，为应用提供WIFI控制，事件，数据导流等操作，为上层应用提供统一
 * 的WIFI控制接口。WLAN框架主要由三个部分组成。DEV驱动接口层，为WLAN框架提供统一的调用接口。Manage
 * 管理层为用户提供WIFI扫描，连接，断线重连等具体功能。Protocol协议负责处理WIFI上产生的数据流，
 * 可根据不同的使用场景挂载不同通讯协议，如LWIP等。具有使用简单，功能齐全，对接方便，兼容性强等特点。
 */

/**
 * @defgroup AT AT 组件
 *
 * @brief AT 组件接口
 *
 * AT 组件是基于 RT-Thread 系统的?`AT Server`?和?`AT Client`?的实现，组件完成 AT 命令的发送、命令格式
 * 及参数判断、命令的响应、响应数据的接收、响应数据的解析、URC 数据处理等整个 AT 命令数据交互流程。
 * 通过 AT 组件设备可以作为 AT Client 使用串口连接其他设备发送并接收解析数据，可以作为 AT Server 让
 * 其他设备甚至电脑端连接完成发送数据的响应，也可以在本地 shell 启动 CLI 模式使设备同时支持 AT Server 
 * 和 AT Client 功能，该模式多用于设备开发调试。 
 * 
 * AT Server 主要功能特点：
 * 
 * - 基础命令： 实现多种通用基础命令（ATE、ATZ等）；
 * - 命令兼容： 命令支持忽略大小写，提高命令兼容性；
 * - 命令检测： 命令支持自定义参数表达式，并实现对接收的命令参数自检测功能；
 * - 命令注册： 提供简单的用户自定义命令添加方式，类似于 `finsh/msh` 命令添加方式；
 * - 调试模式： 提供 AT Server CLI 命令行交互模式，主要用于设备调试。
 * 
 * AT Client 主要功能：
 * 
 * - URC 数据处理： 完备的 URC 数据的处理方式；
 * - 数据解析： 支持自定义响应数据的解析方式，方便获取响应数据中相关信息；
 * - 调试模式： 提供 AT Client CLI 命令行交互模式，主要用于设备调试。
 * - AT Socket：作为 AT Client 功能的延伸，使用 AT 命令收发作为基础，实现标准的 BSD Socket API，完成数据的
 * 收发功能，使用户通过 AT 命令完成设备连网和数据通讯。
 * - 多客户端支持： AT 组件目前支持多客户端同时运行
 */

/*@}*/
