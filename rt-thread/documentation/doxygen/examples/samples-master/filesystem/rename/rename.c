/*
 * �����嵥����������
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� rename()����
 * int rename(const char *oldpath, const char *newpath);
 * rename()�Ὣ����oldpath ��ָ�����ļ����Ƹ�Ϊ����newpath
 * ��ָ���ļ����ơ���newpath ��ָ�����ļ��Ѿ����ڣ�����ļ����ᱻ���ǡ�
 *
*/
#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

static void rename_sample(void)
{
    rt_kprintf("%s => %s ", "/text.txt", "/text1.txt");

    if (rename("/text.txt", "/text1.txt") < 0)
        rt_kprintf("[error!]\n");
    else
        rt_kprintf("[ok!]\n");
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(rename_sample, rename sample);
