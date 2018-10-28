/*
 * �����嵥������Ŀ¼
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� mkdir() ����
 * int mkdir(const char *path, mode_t mode);
 * mkdir()������������һ��Ŀ¼������path ΪĿ¼����
 * ����mode �ڵ�ǰ�汾δ���ã�����0x777 ���ɡ�
*/
#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

static void mkdir_sample(void)
{
    int ret;

    /* ����Ŀ¼*/
    ret = mkdir("/dir_test", 0x777);
    if (ret < 0)
    {
        /* ����Ŀ¼ʧ��*/
        rt_kprintf("dir error!\n");
    }
    else
    {
        /* ����Ŀ¼�ɹ�*/
        rt_kprintf("mkdir ok!\n");
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(mkdir_sample, mkdir sample);

