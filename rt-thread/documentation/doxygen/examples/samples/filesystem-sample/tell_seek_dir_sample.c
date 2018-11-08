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
 * �����嵥�����������ö�ȡĿ¼λ��
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� telldir() ����
 * off_t telldir(DIR *d); ��ȡĿ¼���Ķ�ȡλ��
 * void seekdir(DIR *d, off_t offset); �����´ζ�ȡĿ¼��λ��
*/
#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

/* �����ļ���������һ���߳������ */
static void telldir_sample(void)
{
    DIR *dirp;
    int save3 = 0;
    int cur;
    int i = 0;
    struct dirent *dp;

    /* �򿪸�Ŀ¼ */
    rt_kprintf("the directory is:\n");
    dirp = opendir("/");
    
    for (dp = readdir(dirp); dp != RT_NULL; dp = readdir(dirp))
    {
        /* ���������Ŀ¼���Ŀ¼ָ��*/
        i++;
        if (i == 3)
            save3 = telldir(dirp);

        rt_kprintf("%s\n", dp->d_name);
    }

    /* �ص��ղű���ĵ�����Ŀ¼���Ŀ¼ָ��*/
    seekdir(dirp, save3);

    /* ��鵱ǰĿ¼ָ���Ƿ���ڱ�����ĵ�����Ŀ¼���ָ��. */
    cur = telldir(dirp);
    if (cur != save3)
    {
        rt_kprintf("seekdir (d, %ld); telldir (d) == %ld\n", save3, cur);
    }

    /* �ӵ�����Ŀ¼�ʼ��ӡ*/
    rt_kprintf("the result of tell_seek_dir is:\n");
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
        rt_kprintf("%s\n", dp->d_name);
    }

    /* �ر�Ŀ¼*/
    closedir(dirp);
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(telldir_sample, telldir sample);
