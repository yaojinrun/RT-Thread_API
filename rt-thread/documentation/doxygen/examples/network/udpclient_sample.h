/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup network_sample
 */
/*@{*/

 /**
 * @defgroup udpclient udp�ͻ���
 *
 * ����һ�� udp �ͻ��˵�����
 * ���� udpclient ��������ն�
 * ������ø�ʽ��udpclient URL PORT [COUNT = 10]
 * URL����������ַ  PORT���˿ں�  COUNT����ѡ���� Ĭ��Ϊ 10 
 * �����ܣ����� COUNT �����ݵ�����Զ��
 *
 * **Դ��**
 \code{.c}
 
#include <rtthread.h>
#include <sys/socket.h> /* ʹ��BSD socket����Ҫ����sockets.hͷ�ļ� */
#include <netdb.h>
#include <string.h>
#include <finsh.h>

const char send_data[] = "This is UDP Client from RT-Thread.\n"; /* �����õ������� */
void udpclient(int argc, char **argv)
{
    int sock, port, count;
    struct hostent *host;
    struct sockaddr_in server_addr;
    const char *url;

    if (argc < 3)
    {
        rt_kprintf("Usage: udpclient URL PORT [COUNT = 10]\n");
        rt_kprintf("Like: tcpclient 192.168.12.44 5000\n");
        return ;
    }

    url = argv[1];
    port = strtoul(argv[2], 0, 10);

    if (argc > 3)
        count = strtoul(argv[3], 0, 10);
    else
        count = 10;

    /* ͨ��������ڲ���url���host��ַ��������������������������� */
    host = (struct hostent *) gethostbyname(url);

    /* ����һ��socket��������SOCK_DGRAM��UDP���� */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        return;
    }

    /* ��ʼ��Ԥ���ӵķ���˵�ַ */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* �ܼƷ���count������ */
    while (count)
    {
        /* �������ݵ�����Զ�� */
        sendto(sock, send_data, strlen(send_data), 0,
               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

        /* �߳�����һ��ʱ�� */
        rt_thread_delay(50);

        /* ����ֵ��һ */
        count --;
    }

    /* �ر����socket */
    closesocket(sock);
}

MSH_CMD_EXPORT(udpclient, a udp client sample);

  \endcode
 */


/*@}*/

