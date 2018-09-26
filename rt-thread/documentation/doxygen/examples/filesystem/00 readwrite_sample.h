/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup Fs_example
 */
/*@{*/

 /**
 * @defgroup readwrite �ļ���д
 *
 * ���������ʾ����ζ�дһ���ļ���
 *
 * **Դ��**
 \code{.c}

#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

static void readwrite_sample(void)
{
    int fd, size;
    char s[] = "RT-Thread Programmer!", buffer[80];

    rt_kprintf("Write string %s to test.txt.\n", s);

    /* �Դ����Ͷ�дģʽ�� /text.txt �ļ���������ļ��������򴴽����ļ�*/
    fd = open("/text.txt", O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        write(fd, s, sizeof(s));
        close(fd);
    }

    rt_kprintf("Write done.\n");

    /* ��ֻ��ģʽ�� /text.txt �ļ� */
    fd = open("/text.txt", O_RDONLY);
    if (fd >= 0)
    {
        size = read(fd, buffer, sizeof(buffer));
        close(fd);
        if (size < 0)
            return ;
    }

    rt_kprintf("Read from file test.txt : %s \n", buffer);
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(readwrite_sample, readwrite sample);

  \endcode
 */


/*@}*/
