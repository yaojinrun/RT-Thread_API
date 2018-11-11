/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-30     chenyong     first version
 * 2018-08-17     chenyong     multiple client support
 */

#ifndef __AT_H__
#define __AT_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AT_SW_VERSION                  "1.2.0"

#define AT_CMD_NAME_LEN                16
#define AT_END_MARK_LEN                4

#ifndef AT_CMD_MAX_LEN
#define AT_CMD_MAX_LEN                 128
#endif

/* the server AT commands new line sign */
#if defined(AT_CMD_END_MARK_CRLF)
#define AT_CMD_END_MARK                "\r\n"
#elif defined(AT_CMD_END_MARK_CR)
#define AT_CMD_END_MARK                "\r"
#elif defined(AT_CMD_END_MARK_LF)
#define AT_CMD_END_MARK                "\n"
#endif

#ifndef AT_SERVER_RECV_BUFF_LEN
#define AT_SERVER_RECV_BUFF_LEN        256
#endif

#ifndef AT_SERVER_DEVICE
#define AT_SERVER_DEVICE               "uart2"
#endif

/* the maximum number of supported AT clients */
#ifndef AT_CLIENT_NUM_MAX
#define AT_CLIENT_NUM_MAX              1
#endif

#define AT_CMD_EXPORT(_name_, _args_expr_, _test_, _query_, _setup_, _exec_)   \
    RT_USED static const struct at_cmd __at_cmd_##_test_##_query_##_setup_##_exec_ SECTION("RtAtCmdTab") = \
    {                                                                          \
        _name_,                                                                \
        _args_expr_,                                                           \
        _test_,                                                                \
        _query_,                                                               \
        _setup_,                                                               \
        _exec_,                                                                \
    };

/**
 * @addtogroup AT
 */

/*@{*/

enum at_status
{
    AT_STATUS_UNINITIALIZED = 0,
    AT_STATUS_INITIALIZED,
    AT_STATUS_BUSY,
};
typedef enum at_status at_status_t;

#ifdef AT_USING_SERVER
/**
 * @brief AT 命令错误类型
 */
enum at_result
{
    AT_RESULT_OK = 0,                  /**< @brief 无错误 */
    AT_RESULT_FAILE = -1,              /**< @brief 一般错误 */
    AT_RESULT_NULL = -2,               /**< @brief 结果不需要返回 */
    AT_RESULT_CMD_ERR = -3,            /**< @brief AT命令格式错误或无法执行 */
    AT_RESULT_CHECK_FAILE = -4,        /**< @brief AT命令表达式格式错误 */
    AT_RESULT_PARSE_FAILE = -5,        /**< @brief AT命令参数解析错误 */
};
typedef enum at_result at_result_t;		/**< @brief AT 命令错误类型定义 */

/**
 * @brief AT 命令控制块
 */
struct at_cmd
{
    char name[AT_CMD_NAME_LEN];					/**< @brief 名称 */
    char *args_expr;							/**< @brief 参数及解析表达式 */
    at_result_t (*test)(void);					/**< @brief 测试函数指针 */
    at_result_t (*query)(void);					/**< @brief 查询函数指针 */
    at_result_t (*setup)(const char *args);		/**< @brief 设置函数指针 */
    at_result_t (*exec)(void);					/**< @brief 执行函数指针 */
};
typedef struct at_cmd *at_cmd_t;				/**< @brief AT 命令类型定义 */

/**
 * @brief AT Serve 控制块
 */
struct at_server
{
    rt_device_t device;							/**< @brief  */

    at_status_t status;
    char (*get_char)(void);
    rt_bool_t echo_mode;

    char recv_buffer[AT_SERVER_RECV_BUFF_LEN];
    rt_size_t cur_recv_len;
    rt_sem_t rx_notice;
    char end_mark[AT_END_MARK_LEN];

    rt_thread_t parser;
    void (*parser_entry)(struct at_server *server);
};
typedef struct at_server *at_server_t;					/**< @brief AT Serve 类型定义 */
#endif /* AT_USING_SERVER */

#ifdef AT_USING_CLIENT
enum at_resp_status
{
     AT_RESP_OK = 0,                   /* AT response end is OK */
     AT_RESP_ERROR = -1,               /* AT response end is ERROR */
     AT_RESP_TIMEOUT = -2,             /* AT response is timeout */
     AT_RESP_BUFF_FULL= -3,            /* AT response buffer is full */
};
typedef enum at_resp_status at_resp_status_t;

/**
 * @brief  AT Server 响应数据控制块
 */
struct at_response
{
    /* response buffer */
    char *buf;			/**< @brief 响应数据缓冲区 */
    /* the maximum response buffer size */
    rt_size_t buf_size;	/**< @brief 最大支持的接收数据的长度 */
    /* the number of setting response lines
     * == 0: the response data will auto return when received 'OK' or 'ERROR'
     * != 0: the response data will return when received setting lines number data */
    rt_size_t line_num;	/**< @brief 本次响应数据需要接收的行数 */
    /* the count of received response lines */
    rt_size_t line_counts;	/**< @brief 本次响应数据总行数 */
    /* the maximum response time */
    rt_int32_t timeout;		/**< @brief 最大响应时间 */
};

/**
 * @brief AT Server 响应数据类型定义
 */
typedef struct at_response *at_response_t;

/* URC(Unsolicited Result Code) object, such as: 'RING', 'READY' request by AT server */
struct at_urc
{
    const char *cmd_prefix;
    const char *cmd_suffix;
    void (*func)(const char *data, rt_size_t size);
};
typedef struct at_urc *at_urc_t;
/**
 * @brief AT Client 控制块
 */
struct at_client
{
    rt_device_t device;

    at_status_t status;
    char end_sign;

    char *recv_buffer;
    rt_size_t recv_bufsz;
    rt_size_t cur_recv_len;
    rt_sem_t rx_notice;
    rt_mutex_t lock;

    at_response_t resp;
    rt_sem_t resp_notice;
    at_resp_status_t resp_status;

    const struct at_urc *urc_table;
    rt_size_t urc_table_size;

    rt_thread_t parser;
};
typedef struct at_client *at_client_t;
#endif /* AT_USING_CLIENT */

#ifdef AT_USING_SERVER
/* AT server initialize and start */
int at_server_init(void);

/* AT server send command execute result to AT device */
/**
 * @brief 发送数据至客户端（不换行）
 *
 * 该函数用于 AT Server 通过串口设备发送固定格式的数据到对应的 AT Client 串口设备上，
 * 数据结尾不带换行符。用于自定义 AT Server 中 AT 命令的功能函数中。
 *
 * @param format 自定义输入数据的表达式
 * @param ... 输入数据列表，为可变参数
 */
void at_server_printf(const char *format, ...);
/**
 * @brief 发送数据至客户端（换行）
 *
 * 该函数用于 AT Server 通过串口设备发送固定格式的数据到对应的 AT Client 串口设备上，
 * 数据结尾带换行符。用于自定义 AT Server 中 AT 命令的功能函数中。
 *
 * @param format 自定义输入数据的表达式
 * @param ... 输入数据列表，为可变参数
 */
void at_server_printfln(const char *format, ...);
/**
 * @brief 发送命令执行结果至客户端
 *
 * 该函数用于 AT Server 通过串口设备发送命令执行结果到对应的 AT Client 串口设备上。
 * AT 组件提供多种固定的命令执行结果类型，自定义命令时可以直接使用函数返回结果。
 *
 * AT 组件中命令执行结果类型以枚举类型给出，如下表所示：
 * 
 * | 命令执行结果类型      | 解释               |
 * | --------------------- | ------------------ |
 * | AT_RESULT_OK          | 命令执行成功       |
 * | AT_RESULT_FAILE       | 命令执行失败       |
 * | AT_RESULT_NULL        | 命令无返回结果     |
 * | AT_RESULT_CMD_ERR     | 输入命令错误       |
 * | AT_RESULT_CHECK_FAILE | 参数表达式匹配错误 |
 * | AT_RESULT_PARSE_FAILE | 参数解析错误       |
 *
 * @param result 命令执行结果类型
 */
void at_server_print_result(at_result_t result);

/* AT server request arguments parse */
/**
 * @brief 解析输入命令参数
 *
 * 一个 AT 命令的四种功能函数中，只有设置函数有入参，该入参为去除 AT 命令剩余部分，例如
 * 一个命令输入为 `"AT+TEST=1,2,3,4"`，则设置函数的入参为参数字符串 `"=1,2,3,4"` 部分。
 * 
 * 该命令解析函数主要用于 AT 命令的设置函数中，用于解析传入字符串参数，得到对应的多个输入参数，
 * 用于执行后面操作，这里的解析语法使用的标准 `sscanf` 解析语法，后面 AT Client 参数解析函数中
 * 会详细介绍。
 *
 * @param req_args 请求命令的传入参数字符串
 * @param req_expr 自定义参数解析表达式，用于解析上述传入参数数据
 * @param ... 输出的解析参数列表，为可变参数
 *
 * @return 
 * | **返回** | **描述**                               |
 * | -------- | -------------------------------------- |
 * | >0       | 成功，返回匹配参数表达式的可变参数个数 |
 * | =0       | 失败，无匹配参数表达式的参数           |
 * | -1       | 失败，参数解析错误                     |
 */
int at_req_parse_args(const char *req_args, const char *req_expr, ...);
#endif /* AT_USING_SERVER */

#ifdef AT_USING_CLIENT

/* AT client initialize and start*/
/**
 * @brief AT Client 初始化
 *
 * 配置开启 AT Client 配置之后，需要在启动时对它进行初始化，开启 AT client 功能，如果程序中已经
 * 使用了组件自动初始化，则不再需要额外进行单独的初始化，否则需要在初始化任务中调用该函数。
 * 
 * AT Client 初始化函数，属于应用层函数，需要在使用 AT Client 功能或者使用 AT Client CLI 功能前调用。
 * **at_client_init**?函数完成对 AT Client 设备初始化、AT Client 移植函数的初始化、AT Client 使用的
 * 信号量、互斥锁等资源初始化，并创建?`at_client`?线程用于 AT Client 中数据的接收的解析以及对 URC 
 * 数据的处理。 
 *
 * @param dev_name 设备名称
 * @param recv_bufsz 接收缓冲区大小
 *
 * @return 
 * | **返回**  | **描述**                       |
 * | --------- | ------------------------------ |
 * | RT_EOK    | 成功                           |
 * | -RT_ERROR | 失败，未找到设备               |
 * | -RT_EFULL | 失败，AT客户端已达设定的最大数 |
 */
int at_client_init(const char *dev_name,  rt_size_t recv_bufsz);

/* ========================== multiple AT client function ============================ */

/* get AT client object */
/**
 * @brief 获取 AT 客户端对象
 *
 * 该函数通过传入的设备名称获取该设备创建的 AT 客户端对象，用于多客户端连接时区分不同的客户端。 
 *
 * @param dev_name 获取的客户端对象
 *
 * @return RT_NULL 失败；成功则返回客户端指控制块。
 */
at_client_t at_client_get(const char *dev_name);
/**
 * @brief 获取列表中第一个 AT 模块 
 *
 * @return RT_NULL 列表为空；成功则返回第一个客户端指控制块。
 */
at_client_t at_client_get_first(void);

/* AT client wait for connection to external devices. */
/**
 * @brief 等待模块初始化完成
 *
 * 该函数用于 AT 模块启动时循环发送 AT 命令，直到模块响应数据，说明模块启动成功。 
 *
 * @param client 获取的客户端对象
 * @param timeout 等待超时时间
 *
 * @return 0 成功；小于0 失败，超时时间内无数据返回。
 */
int at_client_obj_wait_connect(at_client_t client, rt_uint32_t timeout);

/* AT client send or receive data */
/**
 * @brief  发送指定长度数据
 *
 * 该函数用于通过 AT Client 设备发送指定长度数据到 AT Server 设备，多用于 AT Socket 功能。  
 *
 * @param client 获取的客户端对象
 * @param buf 发送数据的指针
 * @param size 发送数据的长度
 *
 * @return
 * | **返回** | **描述**                     |
 * | -------- | ---------------------------- |
 * | >0       | 成功，返回发送成功的数据长度 |
 * | <=0      | 失败                         |
 */
rt_size_t at_client_obj_send(at_client_t client, const char *buf, rt_size_t size);
/**
 * @brief 接收指定长度数据
 *
 * 该函数用于通过 AT Client 设备接收指定长度的数据，多用于 AT Socket 功能。**该函数只能在
 *  URC 回调处理函数中使用**。  
 *
 * @param client 获取的客户端对象
 * @param buf 接收数据的指针
 * @param size 最大支持接收数据的长度
 * @param timeout 最大等待时间，数据接收超时返回错误。
 *
 * @return
 * | **返回** | **描述**                     |
 * | -------- | ---------------------------- |
 * | >0       | 成功，返回接收成功的数据长度 |
 * | <=0      | 失败                         |
 */
rt_size_t at_client_obj_recv(at_client_t client, char *buf, rt_size_t size, rt_int32_t timeout);

/* set AT client a line end sign */
/**
 * @brief 设置接收数据的行结束符
 *
 * 该函数用于设置行结束符，用于判断客户端接收一行数据的结束, 多用于 AT Socket 功能。 
 *
 * @param client 获取的客户端对象
 * @param ch 行结束符
 */
void at_obj_set_end_sign(at_client_t client, char ch);

/* Set URC(Unsolicited Result Code) table */
/**
 * @brief URC 数据列表初始化
 *
 * 该函数用于初始化开发者自定义的 URC 数据列表，主要在 AT Client 移植函数中使用。
 *
 * @param client 获取的客户端对象
 * @param table URC 数据结构体数组指针
 * @param size URC 数据的个数
 */
void at_obj_set_urc_table(at_client_t client, const struct at_urc * table, rt_size_t size);

/* AT client send commands to AT server and waiter response */
/**
 * @brief 发送命令并接收响应
 *
 * 该函数用于 AT Client 发送命令到 AT Server，并等待接收响应，其中?`resp`?是已经创建好的
 * 响应结构体的指针，AT 命令的使用匹配表达式的可变参输入，**输入命令的结尾不需要添加命令
 * 结束符**?。 
 *
 * @param client 获取的客户端对象
 * @param resp 创建的响应结构体指针
 * @param cmd_expr 自定义输入命令的表达式
 * @param ... 输入命令数据列表，为可变参数
 *
 * @return
 * | **返回** | **描述**           |
 * | -------- | ------------------ |
 * | >=0      | 成功               |
 * | -1       | 失败               |
 * | -2       | 失败，接收响应超时 |
 */
int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...);

/* AT response object create and delete */
/**
 * @brief  创建响应结构体
 *
 * 该函数用于创建自定义的响应数据接收结构，用于后面接收并解析发送命令响应数据。
 * 
 * @param buf_size 本次响应最大支持的接收数据的长度
 * @param line_num 本次响应需要返回数据的行数，行数是以标准结束符划分
 * 					- 若为 0 ，则接收到 "OK" 或 "ERROR" 数据后结束本次响应接收
 * 					- 若大于 0，接收完当前设置行号的数据后返回成功
 * @param timeout 本次响应数据最大响应时间，数据接收超时返回错误。
 *
 * @return 
 * | **返回** | **描述**                       |
 * | -------- | ------------------------------ |
 * | != NULL  | 成功，返回指向响应结构体的指针 |
 * | = NULL   | 失败，内存不足                 |
 */
at_response_t at_create_resp(rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);
/**
 * @brief  删除响应结构体
 *
 * 该函数用于删除创建的响应结构体对象，一般与?**at_create_resp**?创建函数成对出现。
 *
 * @param resp 准备删除的响应结构体指针
 */
void at_delete_resp(at_response_t resp);
/**
 * @brief  设置响应结构体参数
 *
 * 该函数用于设置已经创建的响应结构体信息，主要设置对响应数据的限制信息，一般用于创建结构体之后，
 * 发送 AT 命令之前。
 *
 * @param resp 已经创建的响应结构体指针
 * @param buf_size 本次响应最大支持的接收数据的长度
 * @param line_num 本次响应需要返回数据的行数，行数是以标准结束符划分
 * 					- 若为 0 ，则接收到 "OK" 或 "ERROR" 数据后结束本次响应接收
 * 					- 若大于 0，接收完当前设置行号的数据后返回成功
 * @param timeout 本次响应数据最大响应时间，数据接收超时返回错误。
 *
 * @return 
 * | **返回** | **描述**                       |
 * | -------- | ------------------------------ |
 * | != NULL  | 成功，返回指向响应结构体的指针 |
 * | = NULL   | 失败，内存不足                 |
 */
at_response_t at_resp_set_info(at_response_t resp, rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);

/* AT response line buffer get and parse response buffer arguments */
/**
 * @brief 获取指定行号的响应数据
 *
 * 该函数用于在 AT Server 响应数据中获取指定行号的一行数据。行号是以标准数据结束符来判断的，
 * 上述发送和接收函数 at_exec_cmd 已经对响应数据的数据和行号进行记录处理存放于 resp 响应结构体中，
 * 这里可以直接获取对应行号的数据信息。
 *
 * @param resp 响应结构体指针
 * @param resp_line 需要获取数据的行号
 *
 * @return
 * | **返回** | **描述**                     |
 * | -------- | ---------------------------- |
 * | != NULL  | 成功，返回对应行号数据的指针 |
 * | = NULL   | 失败，输入行号错误           |
 */
const char *at_resp_get_line(at_response_t resp, rt_size_t resp_line);
/**
 * @brief  获取指定关键字的响应数据
 *
 * 该函数用于在 AT Server 响应数据中通过关键字获取对应的一行数据。
 *
 * @param resp 响应结构体指针
 * @param keyword 关键字信息
 *
 * @return
 * | **返回** | **描述**                     |
 * | -------- | ---------------------------- |
 * | != NULL  | 成功，返回对应行号数据的指针 |
 * | = NULL   | 失败，未找到关键字信息       |
 */
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword);
/**
 * @brief  解析指定行号的响应数据
 *
 * 该函数用于在 AT Server 响应数据中获取指定行号的一行数据, 并解析该行数据中的参数。
 *
 * @param resp 响应结构体指针
 * @param resp_line 需要解析数据的行号
 * @param resp_expr 自定义的参数解析表达式
 * @param ... 解析参数列表，为可变参数
 *
 * @return
 * | **返回** | **描述**                         |
 * | -------- | -------------------------------- |
 * | >0       | 成功，返回解析成功的参数个数     |
 * | =0       | 失败，无匹参配数解析表达式的参数 |
 * | -1       | 失败，参数解析错误               |
 */
int at_resp_parse_line_args(at_response_t resp, rt_size_t resp_line, const char *resp_expr, ...);
/**
 * @brief  解析指定关键字行的响应数据
 *
 * 该函数用于在 AT Server 响应数据中获取包含关键字的一行数据, 并解析该行数据中的参数。
 *
 * @param resp 响应结构体指针
 * @param keyword 关键字信息
 * @param resp_expr 自定义的参数解析表达式
 * @param ... 解析参数列表，为可变参数
 *
 * @return
 * | **返回** | **描述**                         |
 * | -------- | -------------------------------- |
 * | >0       | 成功，返回解析成功的参数个数     |
 * | =0       | 失败，无匹参配数解析表达式的参数 |
 * | -1       | 失败，参数解析错误               |
 */
int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...);

/* ========================== single AT client function ============================ */

/**
 * NOTE: These functions can be used directly when there is only one AT client.
 * If there are multiple AT Client in the program, these functions can operate on the first initialized AT client.
 */

/**
 * @brief 发送命令并接收响应（单客户端模式）
 *
 * @param resp 创建的响应结构体指针
 * @param ... 输入命令数据列表，为可变参数
 */
#define at_exec_cmd(resp, ...)                   at_obj_exec_cmd(at_client_get_first(), resp, __VA_ARGS__)
/**
 * @brief 等待模块初始化完成（单客户端模式）
 *
 * @param timeout 等待超时时间
 */
#define at_client_wait_connect(timeout)          at_client_obj_wait_connect(at_client_get_first(), timeout)
/**
 * @brief 发送指定长度数据（单客户端模式）
 *
 * @param buf 发送数据的指针
 * @param size 发送数据的长度
 */
#define at_client_send(buf, size)                at_client_obj_send(at_client_get_first(), buf, size)
/**
 * @brief 接收指定长度数据（单客户端模式）
 *
 * @param buf 接收数据的指针
 * @param size 最大支持接收数据的长度
 * @param timeout 最大等待时间，数据接收超时返回错误。
 */
#define at_client_recv(buf, size, timeout)       at_client_obj_recv(at_client_get_first(), buf, size, timeout)
/**
 * @brief 设置接收数据的行结束符（单客户端模式）
 *
 * @param ch 行结束符。
 */
#define at_set_end_sign(ch)                      at_obj_set_end_sign(at_client_get_first(), ch)
/**
 * @brief URC 数据列表初始化（单客户端模式）
 *
 * @param table URC 数据结构体数组指针
 * @param size URC 数据的个数。
 */
#define at_set_urc_table(urc_table, table_sz)    at_obj_set_urc_table(at_client_get_first(), urc_table, table_sz)

#endif /* AT_USING_CLIENT */

/* ========================== User port function ============================ */

#ifdef AT_USING_SERVER
/* AT server device reset */
/**
 * @brief 设备重启
 *
 * 该函数完成设备软重启功能，用于 AT Server 中基础命令 AT+RST 的实现。
 */
void at_port_reset(void);

/* AT server device factory reset */
/**
 * @brief 恢复出厂设置
 *
 * 该函数完成设备恢复出厂设置功能，用于 AT Server 中基础命令 ATZ 的实现。
 */
void at_port_factory_reset(void);
#endif

/*@}*/


#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
