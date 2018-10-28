/*
* �����嵥��tcp �����
 *
 * ����һ�� tcp ����˵�����
 * ���� tcpserv ��������ն�
 * ������ø�ʽ��tcpserv
 * �޲���
 * �����ܣ���Ϊһ������ˣ����ղ���ʾ�ͻ��˷��������� �����յ� exit �˳�����
*/
#include <rtthread.h>
#include <sys/socket.h> /* ʹ��BSD socket����Ҫ����socket.hͷ�ļ� */
#include <netdb.h>
#include <string.h>

#define BUFSZ       (1024)

static const char send_data[] = "This is TCP Server from RT-Thread."; /* �����õ������� */
static void tcpserv(int argc, char **argv)
{
    char *recv_data; /* ���ڽ��յ�ָ�룬�������һ�ζ�̬��������������ڴ� */
    socklen_t sin_size;
    int sock, connected, bytes_received;
    struct sockaddr_in server_addr, client_addr;
    rt_bool_t stop = RT_FALSE; /* ֹͣ��־ */
    int ret;

    recv_data = rt_malloc(BUFSZ + 1); /* ��������õ����ݻ��� */
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }

    /* һ��socket��ʹ��ǰ����ҪԤ�ȴ���������ָ��SOCK_STREAMΪTCP��socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* ����ʧ�ܵĴ����� */
        rt_kprintf("Socket error\n");

        /* �ͷ��ѷ���Ľ��ջ��� */
        rt_free(recv_data);
        return;
    }

    /* ��ʼ������˵�ַ */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000); /* ����˹����Ķ˿� */
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* ��socket������˵�ַ */
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* ��ʧ�� */
        rt_kprintf("Unable to bind\n");

        /* �ͷ��ѷ���Ľ��ջ��� */
        rt_free(recv_data);
        return;
    }

    /* ��socket�Ͻ��м��� */
    if (listen(sock, 5) == -1)
    {
        rt_kprintf("Listen error\n");

        /* release recv buffer */
        rt_free(recv_data);
        return;
    }

    rt_kprintf("\nTCPServer Waiting for client on port 5000...\n");
    while (stop != RT_TRUE)
    {
        sin_size = sizeof(struct sockaddr_in);

        /* ����һ���ͻ�������socket�����������������������ʽ�� */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* ���ص������ӳɹ���socket */
        if (connected < 0)
        {
            rt_kprintf("accept connection failed! errno = %d\n", errno);
            continue;
        }

        /* ���ܷ��ص�client_addrָ���˿ͻ��˵ĵ�ַ��Ϣ */
        rt_kprintf("I got a connection from (%s , %d)\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /* �ͻ������ӵĴ��� */
        while (1)
        {
            /* �������ݵ�connected socket */
            ret = send(connected, send_data, strlen(send_data), 0);
            if (ret < 0)
            {
                /* ����ʧ�ܣ��ر�������� */
                closesocket(connected);
                rt_kprintf("\nsend error,close the socket.\r\n");
                break;
            }
            else if (ret == 0)
            {
                /* ��ӡsend��������ֵΪ0�ľ�����Ϣ */
                rt_kprintf("\n Send warning,send function return 0.\r\n");
            }

            /* ��connected socket�н������ݣ�����buffer��1024��С��������һ���ܹ��յ�1024��С������ */
            bytes_received = recv(connected, recv_data, BUFSZ, 0);
            if (bytes_received < 0)
            {
                /* ����ʧ�ܣ��ر����connected socket */
                closesocket(connected);
                break;
            }
            else if (bytes_received == 0)
            {
                /* ��ӡrecv��������ֵΪ0�ľ�����Ϣ */
                rt_kprintf("\nReceived warning,recv function return 0.\r\n");
                closesocket(connected);
                break;
            }

            /* �н��յ����ݣ���ĩ������ */
            recv_data[bytes_received] = '\0';
            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
            {
                /* ���������ĸ��q��Q���ر�������� */
                closesocket(connected);
                break;
            }
            else if (strcmp(recv_data, "exit") == 0)
            {
                /* ������յ���exit����ر���������� */
                closesocket(connected);
                stop = RT_TRUE;
                break;
            }
            else
            {
                /* �ڿ����ն���ʾ�յ������� */
                rt_kprintf("RECEIVED DATA = %s \n", recv_data);
            }
        }
    }

    /* �˳����� */
    closesocket(sock);

    /* �ͷŽ��ջ��� */
    rt_free(recv_data);

    return ;
}
#ifdef FINSH_USING_MSH
#include <finsh.h>

MSH_CMD_EXPORT(tcpserv, a tcp server sample);
#endif
