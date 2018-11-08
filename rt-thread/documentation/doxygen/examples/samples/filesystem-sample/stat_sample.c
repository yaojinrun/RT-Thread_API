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
 * �����嵥��ȡ��״̬
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� stat() ����
 * int stat(const char *file_name, struct stat *buf);
 * stat()��������������file_name ��ָ����ļ�״̬��
 * ���Ƶ�buf ָ����ָ�Ľṹ��(struct stat)��
*/
#include <rtthread.h>
#include <dfs_posix.h> /* ����Ҫʹ���ļ�����ʱ����Ҫ�������ͷ�ļ� */

static void stat_sample(void)
{
    int ret;
    struct stat buf;
    ret=stat("/text.txt", &buf);
    if(ret==0)
        rt_kprintf("text.txt file size = %d\n", buf.st_size);
    else
        rt_kprintf("text.txt file not fonud\n");
}
/* ������ msh �����б��� */
MSH_CMD_EXPORT(stat_sample, show text.txt stat sample);
