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
 * @brief 环形缓冲区的API说明
 */

/**
 * @defgroup completion 完成信号量
 *
 * @brief 完成信号量的API说明
 */

/**
 * @defgroup pipe 管道
 *
 * @brief 管道的API说明
 */

/**
 * @defgroup dataqueue 数据队列
 *
 * @brief 数据队列的API说明
 */

/**
 * @defgroup workqueue 工作队列
 *
 * @brief 工作队列的API说明
 */

/**
 * @defgroup waitqueue 等待队列
 *
 * @brief 等待队列的API说明
 */

/**
 * @defgroup UART UART
 *
 * @brief UART设备的API说明
 */


/**
 * @defgroup pin PIN
 *
 * @brief PIN设备的API说明
 */

/**
 * @defgroup i2c I2C
 *
 * @brief I2C相关的API说明
 */

/**
 * @defgroup spi SPI
 *
 * @brief SPI相关的API说明
 */

/**
 * @defgroup rtc RTC
 *
 * @brief RTC相关的API说明
 */

/**
 * @defgroup alarm Alarm
 *
 * @brief Alarm相关的API说明
 */

/**
 * @defgroup pwm PWM
 *
 * @brief PWM设备的API说明
 */

/**
 * @defgroup PM 电源管理
 *
 * @brief 电源管理相关的API说明
 *
 * PM组件从层次上划分，可以分成用户层、PM 组件层和PM 驱动层。用户层包括了应用代码和驱动代码，它们
 * 通过 API 来决定芯片运行在什么模式。PM 驱动层主要是实现了 PM 驱动的支持以及与 PM 相关的外设功耗
 * 控制。PM 组件层里完成驱动的管理和对用户层提供支持。
 */

/**
 * @defgroup WLAN WLAN 框架
 *
 * @brief WLAN框架相关的API说明
 *
 * WLAN框架是RT-Thread开发的一套用于管理WIFI的中间件。对下连接具体的WIFI驱动，控制WIFI的连接断开，
 * 扫描等操作。对上承载不同的应用，为应用提供WIFI控制，事件，数据导流等操作，为上层应用提供统一
 * 的WIFI控制接口。WLAN框架主要由三个部分组成。DEV驱动接口层，为WLAN框架提供统一的调用接口。Manage
 * 管理层为用户提供WIFI扫描，连接，断线重连等具体功能。Protocol协议负责处理WIFI上产生的数据流，
 * 可根据不同的使用场景挂载不同通讯协议，如LWIP等。具有使用简单，功能齐全，对接方便，兼容性强等特点。
 */

/*@}*/