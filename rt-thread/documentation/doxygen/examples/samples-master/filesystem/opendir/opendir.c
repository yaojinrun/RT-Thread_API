/*
 * �����嵥����Ŀ¼
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� opendir() ����
 * DIR* opendir(const char* name);
 * opendir()����������һ��Ŀ¼������name ΪĿ¼·������
 * ����ȡĿ¼�ɹ������ظ�Ŀ¼�ṹ������ȡĿ¼ʧ�ܣ�����RT_NULL��
*/
#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

static void opendir_sample(void)
{
    DIR *dirp;
    /* ��/dir_test Ŀ¼*/
    dirp = opendir("/dir_test");
    if (dirp == RT_NULL)
    {
        rt_kprintf("open directory error!\n");
    }
    else
    {
        rt_kprintf("open directory OK!\n");
        /* ��������ж�ȡĿ¼��ز���*/
        /* ...... */

        /* �ر�Ŀ¼ */
        closedir(dirp);
    }
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(opendir_sample, open dir sample);
