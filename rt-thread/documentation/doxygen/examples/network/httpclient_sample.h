/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup network_sample
 */
/*@{*/

 /**
 * @defgroup httpclient http�ͻ���
 *
 *
 * **Դ��**
 \code{.c}
 
#include <webclient.h>  /* ʹ�� HTTP Э���������ͨ����Ҫ������ͷ�ļ� */
#include <sys/socket.h> /* ʹ��BSD socket����Ҫ����socket.hͷ�ļ� */
#include <netdb.h>
#include <cJSON.h>
#include <finsh.h>

#define GET_HEADER_BUFSZ        1024        //ͷ����С
#define GET_RESP_BUFSZ          1024        //��Ӧ��������С
#define GET_URL_LEN_MAX         256         //��ַ��󳤶�
#define GET_URI                 "http://mobile.weather.com.cn/data/sk/%s.html" //��ȡ������ API
#define AREA_ID                 "101021300" //�Ϻ��ֶ����� ID

/* �������ݽ��� */
void weather_data_parse(rt_uint8_t *data)
{
    cJSON *root = RT_NULL, *object = RT_NULL, *item = RT_NULL;

    root = cJSON_Parse((const char *)data);
    if (!root)
    {
        rt_kprintf("No memory for cJSON root!\n");
        return;
    }
    object = cJSON_GetObjectItem(root, "sk_info");

    item = cJSON_GetObjectItem(object, "cityName");
    rt_kprintf("\ncityName:%s ", item->valuestring);

    item = cJSON_GetObjectItem(object, "temp");
    rt_kprintf("\ntemp    :%s ", item->valuestring);

    item = cJSON_GetObjectItem(object, "wd");
    rt_kprintf("\nwd      :%s ", item->valuestring);

    item = cJSON_GetObjectItem(object, "ws");
    rt_kprintf("\nws      :%s ", item->valuestring);

    item = cJSON_GetObjectItem(object, "sd");
    rt_kprintf("\nsd      :%s ", item->valuestring);

    item = cJSON_GetObjectItem(object, "date");
    rt_kprintf("\ndate    :%s", item->valuestring);

    item = cJSON_GetObjectItem(object, "time");
    rt_kprintf("\ntime    :%s \n", item->valuestring);

    if (root != RT_NULL)
        cJSON_Delete(root);
}
void weather(int argc, char **argv)
{
    rt_uint8_t *buffer = RT_NULL;
    int resp_status;
    struct webclient_session *session = RT_NULL;
    char *weather_url = RT_NULL;
    int content_length = -1, bytes_read = 0;
    int content_pos = 0;

    /* Ϊ weather_url ����ռ� */
    weather_url = rt_calloc(1, GET_URL_LEN_MAX);
    if (weather_url == RT_NULL)
    {
        rt_kprintf("No memory for weather_url!\n");
        goto __exit;
    }
    /* ƴ�� GET ��ַ */
    rt_snprintf(weather_url, GET_URL_LEN_MAX, GET_URI, AREA_ID);

    /* �����Ự����������Ӧ�Ĵ�С */
    session = webclient_session_create(GET_HEADER_BUFSZ);
    if (session == RT_NULL)
    {
        rt_kprintf("No memory for get header!\n");
        goto __exit;
    }

    /* ���� GET ����ʹ��Ĭ�ϵ�ͷ�� */
    if ((resp_status = webclient_get(session, weather_url)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        goto __exit;
    }

    /* �������ڴ�Ž������ݵĻ��� */
    buffer = rt_calloc(1, GET_RESP_BUFSZ);
    if(buffer == RT_NULL)
    {
        rt_kprintf("No memory for data receive buffer!\n");
        goto __exit;
    }

    content_length = webclient_content_length_get(session);
    if (content_length < 0)
    {
        /* ���ص������Ƿֿ鴫���. */
        do
        {
            bytes_read = webclient_read(session, buffer, GET_RESP_BUFSZ);
            if (bytes_read <= 0)
            {
                break;
            }
        } while (1);
    }
    else
    {
        do
        {
            bytes_read = webclient_read(session, buffer, 
                    content_length - content_pos > GET_RESP_BUFSZ ?
                            GET_RESP_BUFSZ : content_length - content_pos);
            if (bytes_read <= 0)
            {
                break;
            }
            content_pos += bytes_read;
        } while (content_pos < content_length);
    }

    /* �������ݽ��� */
    weather_data_parse(buffer);

__exit:
    /* �ͷ���ַ�ռ� */
    if (weather_url != RT_NULL)
        rt_free(weather_url);
    /* �رջỰ */
    if (session != RT_NULL)
        webclient_close(session);
    /* �ͷŻ������ռ� */
    if (buffer != RT_NULL)
        rt_free(buffer);
}

MSH_CMD_EXPORT(weather, Get weather by webclient);

  \endcode
 */


/*@}*/

