/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date             Author      Notes
 * 
 */
/*
 * �����嵥��tcp �ͻ���
 *
 * ����һ�� tcp �ͻ��˵�����
 * ���� tcpclient ��������ն�
 * ������ø�ʽ��tcpclient URL PORT
 * URL����������ַ PORT:���˿ں�
 * �����ܣ����ղ���ʾ�ӷ���˷��͹�������Ϣ�����յ���ͷ�� 'q' �� 'Q' ����Ϣ�˳�����
*/
#include <rtthread.h>
#include <sys/socket.h> /* ʹ��BSD socket����Ҫ����socket.hͷ�ļ� */
#include <netdb.h>
#include <string.h>
#include <finsh.h>

#define BUFSZ   1024

static const char send_data[] = "This is TCP Client from RT-Thread."; /* �����õ������� */
void tcpclient(int argc, char **argv)
{
    int ret;
    char *recv_data;
    struct hostent *host;
    int sock, bytes_received;
    struct sockaddr_in server_addr;
    const char *url;
    int port;

    if (argc < 3)
    {
        rt_kprintf("Usage: tcpclient URL PORT\n");
        rt_kprintf("Like: tcpclient 192.168.12.44 5000\n");
        return ;
    }

    url = argv[1];
    port = strtoul(argv[2], 0, 10);

    /* ͨ��������ڲ���url���host��ַ��������������������������� */
    host = gethostbyname(url);

    /* �������ڴ�Ž������ݵĻ��� */
    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }

    /* ����һ��socket��������SOCKET_STREAM��TCP���� */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* ����socketʧ�� */
        rt_kprintf("Socket error\n");

        /* �ͷŽ��ջ��� */
        rt_free(recv_data);
        return;
    }

    /* ��ʼ��Ԥ���ӵķ���˵�ַ */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* ���ӵ������ */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ����ʧ�� */
        rt_kprintf("Connect fail!\n");
        closesocket(sock);

        /*�ͷŽ��ջ��� */
        rt_free(recv_data);
        return;
    }

    while (1)
    {
        /* ��sock�����н������BUFSZ - 1�ֽ����� */
        bytes_received = recv(sock, recv_data, BUFSZ - 1, 0);
        if (bytes_received < 0)
        {
            /* ����ʧ�ܣ��ر�������� */
            closesocket(sock);
            rt_kprintf("\nreceived error,close the socket.\r\n");

            /* �ͷŽ��ջ��� */
            rt_free(recv_data);
            break;
        }
        else if (bytes_received == 0)
        {
            /* Ĭ�� recv Ϊ����ģʽ����ʱ�յ�0��Ϊ���ӳ����ر�������� */
            closesocket(sock);
            rt_kprintf("\nreceived error,close the socket.\r\n");

            /* �ͷŽ��ջ��� */
            rt_free(recv_data);
            break;
        }

        /* �н��յ����ݣ���ĩ������ */
        recv_data[bytes_received] = '\0';

        if (strncmp(recv_data, "q", 1) == 0 || strncmp(recv_data, "Q", 1) == 0)
        {
            /* ���������ĸ��q��Q���ر�������� */
            closesocket(sock);
            rt_kprintf("\n got a 'q' or 'Q',close the socket.\r\n");

            /* �ͷŽ��ջ��� */
            rt_free(recv_data);
            break;
        }
        else
        {
            /* �ڿ����ն���ʾ�յ������� */
            rt_kprintf("\nReceived data = %s ", recv_data);
        }

        /* �������ݵ�sock���� */
        ret = send(sock, send_data, strlen(send_data), 0);
        if (ret < 0)
        {
            /* ����ʧ�ܣ��ر�������� */
            closesocket(sock);
            rt_kprintf("\nsend error,close the socket.\r\n");

            rt_free(recv_data);
            break;
        }
        else if (ret == 0)
        {
            /* ��ӡsend��������ֵΪ0�ľ�����Ϣ */
            rt_kprintf("\n Send warning,send function return 0.\r\n");
        }
    }
    return;
}

MSH_CMD_EXPORT(tcpclient, a tcp client sample);
