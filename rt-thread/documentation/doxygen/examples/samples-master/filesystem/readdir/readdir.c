/*
 * �����嵥����ȡĿ¼
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� readdir() ����
 * struct dirent* readdir(DIR *d);
 * readdir()����������ȡĿ¼������d ΪĿ¼·������
 * ����ֵΪ������Ŀ¼��ṹ���������ֵΪRT_NULL�����ʾ�Ѿ�����Ŀ¼β��
 * ���⣬ÿ��ȡһ��Ŀ¼��Ŀ¼����ָ��λ�ý��Զ��������1 ��λ�á�
*/
#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

static void readdir_sample(void)
{
    DIR *dirp;
    struct dirent *d;

    /* ��/dir_test Ŀ¼*/
    dirp = opendir("/dir_test");
    if (dirp == RT_NULL)
    {
        rt_kprintf("open directory error!\n");
    }
    else
    {
        /* ��ȡĿ¼*/
        while ((d = readdir(dirp)) != RT_NULL)
        {
            rt_kprintf("found %s\n", d->d_name);
        }

        /* �ر�Ŀ¼ */
        closedir(dirp);
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(readdir_sample, readdir sample);
