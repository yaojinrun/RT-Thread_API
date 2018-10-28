/*
 * �����嵥�����ļ�
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� open() close() ����
 * int open(const char *pathname, int oflag, int mode); �򿪻򴴽�һ���ļ�
 * int close(int fd);�ر��ļ�
 *
*/
#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

static void openfile_sample(void *parameter)
{
    int fd, size;
    char s[] = "RT-Thread Programmer!\n", buffer[80];

    /* ��/text.txt ��д�룬������ļ��������������ļ�*/
    fd = open("/text.txt", O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        write(fd, s, sizeof(s));
        close(fd);
    }

    /* ��/text.txt ׼������ȡ����*/
    fd = open("/text.txt", O_RDONLY);
    if (fd >= 0)
    {
        size = read(fd, buffer, sizeof(buffer));
        close(fd);
    }

    rt_kprintf("%s", buffer);
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(openfile_sample, open file sample);