/*
 * This file is only used for doxygen document generation.
 */

 
/**
 * @addtogroup Fs_example
 */
/*@{*/

 /**
 * @defgroup stat ȡ��״̬
 *
 * ����ᴴ��һ�������ļ��ĺ�����������msh�����б�
 * �ں����е��� stat() ����
 * int stat(const char *file_name, struct stat *buf);
 * stat()��������������file_name ��ָ����ļ�״̬��
 * ���Ƶ�buf ָ����ָ�Ľṹ��(struct stat)��
 *
 * **Դ��**
 \code{.c}
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

  \endcode
 */


/*@}*/

