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
 * @brief AT �����������
 */
enum at_result
{
    AT_RESULT_OK = 0,                  /**< @brief �޴��� */
    AT_RESULT_FAILE = -1,              /**< @brief һ����� */
    AT_RESULT_NULL = -2,               /**< @brief �������Ҫ���� */
    AT_RESULT_CMD_ERR = -3,            /**< @brief AT�����ʽ������޷�ִ�� */
    AT_RESULT_CHECK_FAILE = -4,        /**< @brief AT������ʽ��ʽ���� */
    AT_RESULT_PARSE_FAILE = -5,        /**< @brief AT��������������� */
};
typedef enum at_result at_result_t;		/**< @brief AT ����������Ͷ��� */

/**
 * @brief AT ������ƿ�
 */
struct at_cmd
{
    char name[AT_CMD_NAME_LEN];					/**< @brief ���� */
    char *args_expr;							/**< @brief �������������ʽ */
    at_result_t (*test)(void);					/**< @brief ���Ժ���ָ�� */
    at_result_t (*query)(void);					/**< @brief ��ѯ����ָ�� */
    at_result_t (*setup)(const char *args);		/**< @brief ���ú���ָ�� */
    at_result_t (*exec)(void);					/**< @brief ִ�к���ָ�� */
};
typedef struct at_cmd *at_cmd_t;				/**< @brief AT �������Ͷ��� */

/**
 * @brief AT Serve ���ƿ�
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
typedef struct at_server *at_server_t;					/**< @brief AT Serve ���Ͷ��� */
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
 * @brief  AT Server ��Ӧ���ݿ��ƿ�
 */
struct at_response
{
    /* response buffer */
    char *buf;			/**< @brief ��Ӧ���ݻ����� */
    /* the maximum response buffer size */
    rt_size_t buf_size;	/**< @brief ���֧�ֵĽ������ݵĳ��� */
    /* the number of setting response lines
     * == 0: the response data will auto return when received 'OK' or 'ERROR'
     * != 0: the response data will return when received setting lines number data */
    rt_size_t line_num;	/**< @brief ������Ӧ������Ҫ���յ����� */
    /* the count of received response lines */
    rt_size_t line_counts;	/**< @brief ������Ӧ���������� */
    /* the maximum response time */
    rt_int32_t timeout;		/**< @brief �����Ӧʱ�� */
};

/**
 * @brief AT Server ��Ӧ�������Ͷ���
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
 * @brief AT Client ���ƿ�
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
 * @brief �����������ͻ��ˣ������У�
 *
 * �ú������� AT Server ͨ�������豸���͹̶���ʽ�����ݵ���Ӧ�� AT Client �����豸�ϣ�
 * ���ݽ�β�������з��������Զ��� AT Server �� AT ����Ĺ��ܺ����С�
 *
 * @param format �Զ����������ݵı��ʽ
 * @param ... ���������б�Ϊ�ɱ����
 */
void at_server_printf(const char *format, ...);
/**
 * @brief �����������ͻ��ˣ����У�
 *
 * �ú������� AT Server ͨ�������豸���͹̶���ʽ�����ݵ���Ӧ�� AT Client �����豸�ϣ�
 * ���ݽ�β�����з��������Զ��� AT Server �� AT ����Ĺ��ܺ����С�
 *
 * @param format �Զ����������ݵı��ʽ
 * @param ... ���������б�Ϊ�ɱ����
 */
void at_server_printfln(const char *format, ...);
/**
 * @brief ��������ִ�н�����ͻ���
 *
 * �ú������� AT Server ͨ�������豸��������ִ�н������Ӧ�� AT Client �����豸�ϡ�
 * AT ����ṩ���̶ֹ�������ִ�н�����ͣ��Զ�������ʱ����ֱ��ʹ�ú������ؽ����
 *
 * AT ���������ִ�н��������ö�����͸��������±���ʾ��
 * 
 * | ����ִ�н������      | ����               |
 * | --------------------- | ------------------ |
 * | AT_RESULT_OK          | ����ִ�гɹ�       |
 * | AT_RESULT_FAILE       | ����ִ��ʧ��       |
 * | AT_RESULT_NULL        | �����޷��ؽ��     |
 * | AT_RESULT_CMD_ERR     | �����������       |
 * | AT_RESULT_CHECK_FAILE | �������ʽƥ����� |
 * | AT_RESULT_PARSE_FAILE | ������������       |
 *
 * @param result ����ִ�н������
 */
void at_server_print_result(at_result_t result);

/* AT server request arguments parse */
/**
 * @brief ���������������
 *
 * һ�� AT ��������ֹ��ܺ����У�ֻ�����ú�������Σ������Ϊȥ�� AT ����ʣ�ಿ�֣�����
 * һ����������Ϊ `"AT+TEST=1,2,3,4"`�������ú��������Ϊ�����ַ��� `"=1,2,3,4"` ���֡�
 * 
 * ���������������Ҫ���� AT ��������ú����У����ڽ��������ַ����������õ���Ӧ�Ķ�����������
 * ����ִ�к������������Ľ����﷨ʹ�õı�׼ `sscanf` �����﷨������ AT Client ��������������
 * ����ϸ���ܡ�
 *
 * @param req_args ��������Ĵ�������ַ���
 * @param req_expr �Զ�������������ʽ�����ڽ������������������
 * @param ... ����Ľ��������б�Ϊ�ɱ����
 *
 * @return 
 * | **����** | **����**                               |
 * | -------- | -------------------------------------- |
 * | >0       | �ɹ�������ƥ��������ʽ�Ŀɱ�������� |
 * | =0       | ʧ�ܣ���ƥ��������ʽ�Ĳ���           |
 * | -1       | ʧ�ܣ�������������                     |
 */
int at_req_parse_args(const char *req_args, const char *req_expr, ...);
#endif /* AT_USING_SERVER */

#ifdef AT_USING_CLIENT

/* AT client initialize and start*/
/**
 * @brief AT Client ��ʼ��
 *
 * ���ÿ��� AT Client ����֮����Ҫ������ʱ�������г�ʼ�������� AT client ���ܣ�����������Ѿ�
 * ʹ��������Զ���ʼ����������Ҫ������е����ĳ�ʼ����������Ҫ�ڳ�ʼ�������е��øú�����
 * 
 * AT Client ��ʼ������������Ӧ�ò㺯������Ҫ��ʹ�� AT Client ���ܻ���ʹ�� AT Client CLI ����ǰ���á�
 * **at_client_init**?������ɶ� AT Client �豸��ʼ����AT Client ��ֲ�����ĳ�ʼ����AT Client ʹ�õ�
 * �ź���������������Դ��ʼ����������?`at_client`?�߳����� AT Client �����ݵĽ��յĽ����Լ��� URC 
 * ���ݵĴ��� 
 *
 * @param dev_name �豸����
 * @param recv_bufsz ���ջ�������С
 *
 * @return 
 * | **����**  | **����**                       |
 * | --------- | ------------------------------ |
 * | RT_EOK    | �ɹ�                           |
 * | -RT_ERROR | ʧ�ܣ�δ�ҵ��豸               |
 * | -RT_EFULL | ʧ�ܣ�AT�ͻ����Ѵ��趨������� |
 */
int at_client_init(const char *dev_name,  rt_size_t recv_bufsz);

/* ========================== multiple AT client function ============================ */

/* get AT client object */
/**
 * @brief ��ȡ AT �ͻ��˶���
 *
 * �ú���ͨ��������豸���ƻ�ȡ���豸������ AT �ͻ��˶������ڶ�ͻ�������ʱ���ֲ�ͬ�Ŀͻ��ˡ� 
 *
 * @param dev_name ��ȡ�Ŀͻ��˶���
 *
 * @return RT_NULL ʧ�ܣ��ɹ��򷵻ؿͻ���ָ���ƿ顣
 */
at_client_t at_client_get(const char *dev_name);
/**
 * @brief ��ȡ�б��е�һ�� AT ģ�� 
 *
 * @return RT_NULL �б�Ϊ�գ��ɹ��򷵻ص�һ���ͻ���ָ���ƿ顣
 */
at_client_t at_client_get_first(void);

/* AT client wait for connection to external devices. */
/**
 * @brief �ȴ�ģ���ʼ�����
 *
 * �ú������� AT ģ������ʱѭ������ AT ���ֱ��ģ����Ӧ���ݣ�˵��ģ�������ɹ��� 
 *
 * @param client ��ȡ�Ŀͻ��˶���
 * @param timeout �ȴ���ʱʱ��
 *
 * @return 0 �ɹ���С��0 ʧ�ܣ���ʱʱ���������ݷ��ء�
 */
int at_client_obj_wait_connect(at_client_t client, rt_uint32_t timeout);

/* AT client send or receive data */
/**
 * @brief  ����ָ����������
 *
 * �ú�������ͨ�� AT Client �豸����ָ���������ݵ� AT Server �豸�������� AT Socket ���ܡ�  
 *
 * @param client ��ȡ�Ŀͻ��˶���
 * @param buf �������ݵ�ָ��
 * @param size �������ݵĳ���
 *
 * @return
 * | **����** | **����**                     |
 * | -------- | ---------------------------- |
 * | >0       | �ɹ������ط��ͳɹ������ݳ��� |
 * | <=0      | ʧ��                         |
 */
rt_size_t at_client_obj_send(at_client_t client, const char *buf, rt_size_t size);
/**
 * @brief ����ָ����������
 *
 * �ú�������ͨ�� AT Client �豸����ָ�����ȵ����ݣ������� AT Socket ���ܡ�**�ú���ֻ����
 *  URC �ص���������ʹ��**��  
 *
 * @param client ��ȡ�Ŀͻ��˶���
 * @param buf �������ݵ�ָ��
 * @param size ���֧�ֽ������ݵĳ���
 * @param timeout ���ȴ�ʱ�䣬���ݽ��ճ�ʱ���ش���
 *
 * @return
 * | **����** | **����**                     |
 * | -------- | ---------------------------- |
 * | >0       | �ɹ������ؽ��ճɹ������ݳ��� |
 * | <=0      | ʧ��                         |
 */
rt_size_t at_client_obj_recv(at_client_t client, char *buf, rt_size_t size, rt_int32_t timeout);

/* set AT client a line end sign */
/**
 * @brief ���ý������ݵ��н�����
 *
 * �ú������������н������������жϿͻ��˽���һ�����ݵĽ���, ������ AT Socket ���ܡ� 
 *
 * @param client ��ȡ�Ŀͻ��˶���
 * @param ch �н�����
 */
void at_obj_set_end_sign(at_client_t client, char ch);

/* Set URC(Unsolicited Result Code) table */
/**
 * @brief URC �����б��ʼ��
 *
 * �ú������ڳ�ʼ���������Զ���� URC �����б���Ҫ�� AT Client ��ֲ������ʹ�á�
 *
 * @param client ��ȡ�Ŀͻ��˶���
 * @param table URC ���ݽṹ������ָ��
 * @param size URC ���ݵĸ���
 */
void at_obj_set_urc_table(at_client_t client, const struct at_urc * table, rt_size_t size);

/* AT client send commands to AT server and waiter response */
/**
 * @brief �������������Ӧ
 *
 * �ú������� AT Client ������� AT Server�����ȴ�������Ӧ������?`resp`?���Ѿ������õ�
 * ��Ӧ�ṹ���ָ�룬AT �����ʹ��ƥ����ʽ�Ŀɱ�����룬**��������Ľ�β����Ҫ�������
 * ������**?�� 
 *
 * @param client ��ȡ�Ŀͻ��˶���
 * @param resp ��������Ӧ�ṹ��ָ��
 * @param cmd_expr �Զ�����������ı��ʽ
 * @param ... �������������б�Ϊ�ɱ����
 *
 * @return
 * | **����** | **����**           |
 * | -------- | ------------------ |
 * | >=0      | �ɹ�               |
 * | -1       | ʧ��               |
 * | -2       | ʧ�ܣ�������Ӧ��ʱ |
 */
int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...);

/* AT response object create and delete */
/**
 * @brief  ������Ӧ�ṹ��
 *
 * �ú������ڴ����Զ������Ӧ���ݽ��սṹ�����ں�����ղ���������������Ӧ���ݡ�
 * 
 * @param buf_size ������Ӧ���֧�ֵĽ������ݵĳ���
 * @param line_num ������Ӧ��Ҫ�������ݵ��������������Ա�׼����������
 * 					- ��Ϊ 0 ������յ� "OK" �� "ERROR" ���ݺ����������Ӧ����
 * 					- ������ 0�������굱ǰ�����кŵ����ݺ󷵻سɹ�
 * @param timeout ������Ӧ���������Ӧʱ�䣬���ݽ��ճ�ʱ���ش���
 *
 * @return 
 * | **����** | **����**                       |
 * | -------- | ------------------------------ |
 * | != NULL  | �ɹ�������ָ����Ӧ�ṹ���ָ�� |
 * | = NULL   | ʧ�ܣ��ڴ治��                 |
 */
at_response_t at_create_resp(rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);
/**
 * @brief  ɾ����Ӧ�ṹ��
 *
 * �ú�������ɾ����������Ӧ�ṹ�����һ����?**at_create_resp**?���������ɶԳ��֡�
 *
 * @param resp ׼��ɾ������Ӧ�ṹ��ָ��
 */
void at_delete_resp(at_response_t resp);
/**
 * @brief  ������Ӧ�ṹ�����
 *
 * �ú������������Ѿ���������Ӧ�ṹ����Ϣ����Ҫ���ö���Ӧ���ݵ�������Ϣ��һ�����ڴ����ṹ��֮��
 * ���� AT ����֮ǰ��
 *
 * @param resp �Ѿ���������Ӧ�ṹ��ָ��
 * @param buf_size ������Ӧ���֧�ֵĽ������ݵĳ���
 * @param line_num ������Ӧ��Ҫ�������ݵ��������������Ա�׼����������
 * 					- ��Ϊ 0 ������յ� "OK" �� "ERROR" ���ݺ����������Ӧ����
 * 					- ������ 0�������굱ǰ�����кŵ����ݺ󷵻سɹ�
 * @param timeout ������Ӧ���������Ӧʱ�䣬���ݽ��ճ�ʱ���ش���
 *
 * @return 
 * | **����** | **����**                       |
 * | -------- | ------------------------------ |
 * | != NULL  | �ɹ�������ָ����Ӧ�ṹ���ָ�� |
 * | = NULL   | ʧ�ܣ��ڴ治��                 |
 */
at_response_t at_resp_set_info(at_response_t resp, rt_size_t buf_size, rt_size_t line_num, rt_int32_t timeout);

/* AT response line buffer get and parse response buffer arguments */
/**
 * @brief ��ȡָ���кŵ���Ӧ����
 *
 * �ú��������� AT Server ��Ӧ�����л�ȡָ���кŵ�һ�����ݡ��к����Ա�׼���ݽ��������жϵģ�
 * �������ͺͽ��պ��� at_exec_cmd �Ѿ�����Ӧ���ݵ����ݺ��кŽ��м�¼�������� resp ��Ӧ�ṹ���У�
 * �������ֱ�ӻ�ȡ��Ӧ�кŵ�������Ϣ��
 *
 * @param resp ��Ӧ�ṹ��ָ��
 * @param resp_line ��Ҫ��ȡ���ݵ��к�
 *
 * @return
 * | **����** | **����**                     |
 * | -------- | ---------------------------- |
 * | != NULL  | �ɹ������ض�Ӧ�к����ݵ�ָ�� |
 * | = NULL   | ʧ�ܣ������кŴ���           |
 */
const char *at_resp_get_line(at_response_t resp, rt_size_t resp_line);
/**
 * @brief  ��ȡָ���ؼ��ֵ���Ӧ����
 *
 * �ú��������� AT Server ��Ӧ������ͨ���ؼ��ֻ�ȡ��Ӧ��һ�����ݡ�
 *
 * @param resp ��Ӧ�ṹ��ָ��
 * @param keyword �ؼ�����Ϣ
 *
 * @return
 * | **����** | **����**                     |
 * | -------- | ---------------------------- |
 * | != NULL  | �ɹ������ض�Ӧ�к����ݵ�ָ�� |
 * | = NULL   | ʧ�ܣ�δ�ҵ��ؼ�����Ϣ       |
 */
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword);
/**
 * @brief  ����ָ���кŵ���Ӧ����
 *
 * �ú��������� AT Server ��Ӧ�����л�ȡָ���кŵ�һ������, ���������������еĲ�����
 *
 * @param resp ��Ӧ�ṹ��ָ��
 * @param resp_line ��Ҫ�������ݵ��к�
 * @param resp_expr �Զ���Ĳ����������ʽ
 * @param ... ���������б�Ϊ�ɱ����
 *
 * @return
 * | **����** | **����**                         |
 * | -------- | -------------------------------- |
 * | >0       | �ɹ������ؽ����ɹ��Ĳ�������     |
 * | =0       | ʧ�ܣ���ƥ�������������ʽ�Ĳ��� |
 * | -1       | ʧ�ܣ�������������               |
 */
int at_resp_parse_line_args(at_response_t resp, rt_size_t resp_line, const char *resp_expr, ...);
/**
 * @brief  ����ָ���ؼ����е���Ӧ����
 *
 * �ú��������� AT Server ��Ӧ�����л�ȡ�����ؼ��ֵ�һ������, ���������������еĲ�����
 *
 * @param resp ��Ӧ�ṹ��ָ��
 * @param keyword �ؼ�����Ϣ
 * @param resp_expr �Զ���Ĳ����������ʽ
 * @param ... ���������б�Ϊ�ɱ����
 *
 * @return
 * | **����** | **����**                         |
 * | -------- | -------------------------------- |
 * | >0       | �ɹ������ؽ����ɹ��Ĳ�������     |
 * | =0       | ʧ�ܣ���ƥ�������������ʽ�Ĳ��� |
 * | -1       | ʧ�ܣ�������������               |
 */
int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...);

/* ========================== single AT client function ============================ */

/**
 * NOTE: These functions can be used directly when there is only one AT client.
 * If there are multiple AT Client in the program, these functions can operate on the first initialized AT client.
 */

/**
 * @brief �������������Ӧ�����ͻ���ģʽ��
 *
 * @param resp ��������Ӧ�ṹ��ָ��
 * @param ... �������������б�Ϊ�ɱ����
 */
#define at_exec_cmd(resp, ...)                   at_obj_exec_cmd(at_client_get_first(), resp, __VA_ARGS__)
/**
 * @brief �ȴ�ģ���ʼ����ɣ����ͻ���ģʽ��
 *
 * @param timeout �ȴ���ʱʱ��
 */
#define at_client_wait_connect(timeout)          at_client_obj_wait_connect(at_client_get_first(), timeout)
/**
 * @brief ����ָ���������ݣ����ͻ���ģʽ��
 *
 * @param buf �������ݵ�ָ��
 * @param size �������ݵĳ���
 */
#define at_client_send(buf, size)                at_client_obj_send(at_client_get_first(), buf, size)
/**
 * @brief ����ָ���������ݣ����ͻ���ģʽ��
 *
 * @param buf �������ݵ�ָ��
 * @param size ���֧�ֽ������ݵĳ���
 * @param timeout ���ȴ�ʱ�䣬���ݽ��ճ�ʱ���ش���
 */
#define at_client_recv(buf, size, timeout)       at_client_obj_recv(at_client_get_first(), buf, size, timeout)
/**
 * @brief ���ý������ݵ��н����������ͻ���ģʽ��
 *
 * @param ch �н�������
 */
#define at_set_end_sign(ch)                      at_obj_set_end_sign(at_client_get_first(), ch)
/**
 * @brief URC �����б��ʼ�������ͻ���ģʽ��
 *
 * @param table URC ���ݽṹ������ָ��
 * @param size URC ���ݵĸ�����
 */
#define at_set_urc_table(urc_table, table_sz)    at_obj_set_urc_table(at_client_get_first(), urc_table, table_sz)

#endif /* AT_USING_CLIENT */

/* ========================== User port function ============================ */

#ifdef AT_USING_SERVER
/* AT server device reset */
/**
 * @brief �豸����
 *
 * �ú�������豸���������ܣ����� AT Server �л������� AT+RST ��ʵ�֡�
 */
void at_port_reset(void);

/* AT server device factory reset */
/**
 * @brief �ָ���������
 *
 * �ú�������豸�ָ��������ù��ܣ����� AT Server �л������� ATZ ��ʵ�֡�
 */
void at_port_factory_reset(void);
#endif

/*@}*/


#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
