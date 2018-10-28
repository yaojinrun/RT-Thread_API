/*
 * �����嵥���ļ���д����
 *
 * ���������ʾ����ζ�дһ���ļ����ر���д��ʱ��Ӧ����β�����
 */

#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

#define TEST_FN     "/test.dat"

/* �����õ����ݺͻ��� */
static char test_data[120], buffer[120];

static void readwrite_sample(void)
{
    int fd;
    int index, length;

    /* ֻд & ���� �� */
    fd = open(TEST_FN, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        rt_kprintf("open file for write failed\n");
        return;
    }

    /* ׼��д������ */
    for (index = 0; index < sizeof(test_data); index ++)
    {
        test_data[index] = index + 48;
    }

    /* д������ */
    length = write(fd, test_data, sizeof(test_data));
    if (length != sizeof(test_data))
    {
        rt_kprintf("write data failed\n");
        close(fd);
        return;
    }

    /* �ر��ļ� */
    close(fd);

    /* ֻд����ĩβ��Ӵ� */
    fd = open(TEST_FN, O_WRONLY | O_CREAT | O_APPEND, 0);
    if (fd < 0)
    {
        rt_kprintf("open file for append write failed\n");
        return;
    }

    length = write(fd, test_data, sizeof(test_data));
    if (length != sizeof(test_data))
    {
        rt_kprintf("append write data failed\n");
        close(fd);          
        return;
    }
    /* �ر��ļ� */
    close(fd);

    /* ֻ���򿪽�������У�� */
    fd = open(TEST_FN, O_RDONLY, 0);
    if (fd < 0)
    {
        rt_kprintf("check: open file for read failed\n");
        return;
    }

    /* ��ȡ����(Ӧ��Ϊ��һ��д�������) */
    length = read(fd, buffer, sizeof(buffer));
    if (length != sizeof(buffer))
    {
        rt_kprintf("check: read file failed\n");
        close(fd);
        return;
    }

    /* ��������Ƿ���ȷ */
    for (index = 0; index < sizeof(test_data); index ++)
    {
        if (test_data[index] != buffer[index])
        {
            rt_kprintf("check: check data failed at %d\n", index);
            close(fd);
            return;
        }
    }

    /* ��ȡ����(Ӧ��Ϊ�ڶ���д�������) */
    length = read(fd, buffer, sizeof(buffer));
    if (length != sizeof(buffer))
    {
        rt_kprintf("check: read file failed\n");
        close(fd);
        return;
    }

    /* ��������Ƿ���ȷ */
    for (index = 0; index < sizeof(test_data); index ++)
    {
        if (test_data[index] != buffer[index])
        {
            rt_kprintf("check: check data failed at %d\n", index);
            close(fd);
            return;
        }
    }

    /* ���������ϣ��ر��ļ� */
    close(fd);
    /* ��ӡ��� */
    rt_kprintf("read/write done.\n");
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(readwrite_sample, readwrite sample);
