/*
 * File      : dfs_posix.c
 * This file is part of Device File System in RT-Thread RTOS
 * COPYRIGHT (C) 2004-2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-05-27     Yi.qiu       The first version
 * 2018-02-07     Bernard      Change the 3rd parameter of open/fcntl/ioctl to '...'
 */

#include <dfs.h>
#include <dfs_posix.h>
#include "dfs_private.h"

/**
 * @addtogroup FsPosixApi
 */

/*@{*/

/**
 * @brief ���ļ�
 *
 * �˺�����POSIX���ݰ汾���������ļ�������ָ���ı�־�����ļ���������
 *
 * @param file �򿪻򴴽����ļ���
 * @param flags ָ�����ļ��ķ�ʽ
 *
 * ��ǰ֧�ֵĴ򿪷�ʽ�����¼��֣�
 *
 * flags             | ����
 * ------------------|------------------------------------
 * O_RDONLY   | ֻ����ʽ��
 * O_WRONLY  |  ֻд��ʽ��
 * O_RDWR    | �Զ�д��ʽ��
 * O_CREAT   | ���Ҫ�򿪵��ļ������ڣ��������ļ�
 * O_APPEND  | ����д�ļ�ʱ����ļ�β��ʼ�ƶ���Ҳ����˵д������ݻ��Ը��ӵķ�ʽ��ӵ��ļ���β����
 * O_TRUNC    | ����ļ��Ѿ����ڣ�������ļ��е�����
 *
 * @return �ɹ��򷵻ش��ļ�������, ʧ���򷵻�-1��������뽫���뵱ǰ�̵߳�errno�С�
 */
int open(const char *file, int flags, ...)
{
    int fd, result;
    struct dfs_fd *d;

    /* allocate a fd */
    fd = fd_new();
    if (fd < 0)
    {
        rt_set_errno(-ENOMEM);

        return -1;
    }
    d = fd_get(fd);

    result = dfs_file_open(d, file, flags);
    if (result < 0)
    {
        /* release the ref-count of fd */
        fd_put(d);
        fd_put(d);

        rt_set_errno(result);

        return -1;
    }

    /* release the ref-count of fd */
    fd_put(d);

    return fd;
}
RTM_EXPORT(open);

/**
 * @brief �ر��ļ�
 *
 * �˺�����POSIX���ݰ汾�������ر�ָ�����ļ���
 *
 * @param fd �ļ�������
 *
 * @return �ɹ� 0��ʧ�� -1��������뽫���뵱ǰ�̵߳�errno�С�
 */
int close(int fd)
{
    int result;
    struct dfs_fd *d;

    d = fd_get(fd);
    if (d == NULL)
    {
        rt_set_errno(-EBADF);

        return -1;
    }

    result = dfs_file_close(d);
    fd_put(d);

    if (result < 0)
    {
        rt_set_errno(result);

        return -1;
    }

    fd_put(d);

    return 0;
}
RTM_EXPORT(close);

/**
 * @brief ��ȡ����
 *
 * �ú����ӿڻ�Ѳ���fd ��ָ���ļ���len ���ֽڶ�ȡ��buf ָ����ָ���ڴ��У�
 * �ļ��Ķ�дλ�û����ȡ�����ֽ��ƶ���
 *
 * @param fd �ļ�������
 * @param buf ��ȡ���ݽ�Ҫд��Ļ����ַָ��
 * @param len ��ȡ�ļ����ֽ���
 *
 * @return ʵ�ʶ�ȡ�����ֽ����� ����������᷵��0ֵ��һ�Ƕ�ȡ�����ѵ����ļ���β��
 * �����޿ɶ�ȡ�����ݣ������趨lenΪ0������ȡʧ���򷵻�-1��������뽫���뵱ǰ�̵߳�errno�С�
 */
#ifdef RT_USING_NEWLIB
_READ_WRITE_RETURN_TYPE _EXFUN(read, (int fd, void *buf, size_t len))
#else
int read(int fd, void *buf, size_t len)
#endif
{
    int result;
    struct dfs_fd *d;

    /* get the fd */
    d = fd_get(fd);
    if (d == NULL)
    {
        rt_set_errno(-EBADF);

        return -1;
    }

    result = dfs_file_read(d, buf, len);
    if (result < 0)
    {
        fd_put(d);
        rt_set_errno(result);

        return -1;
    }

    /* release the ref-count of fd */
    fd_put(d);

    return result;
}
RTM_EXPORT(read);

/**
 * @brief д������
 *
 * �����ӿڻ��buf ָ����ָ����ڴ���len ���ֽ�д�뵽����fd ��ָ����ļ��ڣ�
 * �ļ��Ķ�дλ�û�д����ֽ��ƶ���
 *
 * @param fd �ļ�������
 * @param buf Ԥд�����ݵĵ�ַָ��
 * @param len Ԥд���ļ����ֽ���
 *
 * @return ʵ��д���ļ����ֽ�����ʧ���򷵻�-1��������뽫���뵱ǰ�̵߳�errno�С�
 */
#ifdef RT_USING_NEWLIB
_READ_WRITE_RETURN_TYPE _EXFUN(write, (int fd, const void *buf, size_t len))
#else
int write(int fd, const void *buf, size_t len)
#endif
{
    int result;
    struct dfs_fd *d;

    /* get the fd */
    d = fd_get(fd);
    if (d == NULL)
    {
        rt_set_errno(-EBADF);

        return -1;
    }

    result = dfs_file_write(d, buf, len);
    if (result < 0)
    {
        fd_put(d);
        rt_set_errno(result);

        return -1;
    }

    /* release the ref-count of fd */
    fd_put(d);

    return result;
}
RTM_EXPORT(write);

/*
 * @brief �ƶ��ļ���дָ��
 *
 * �ú�������Ҫ�������ƶ��ļ���дָ�롣
 *
 * @param fd ��ʾҪ�������ļ�������
 * @param offset �����whence����׼����ƫ����
 * @param whence ָ��ƫ�Ƶķ�ʽ��ȡֵ���£�
 * - SEEK_SET ����дλ��ָ���ļ�ͷ��������offset��λ������
 * - SEEK_CUR ��Ŀǰ�Ķ�дλ����������offset��λ������
 * - SEEK_END ����дλ��ָ���ļ�β��������offset��λ������
 *
 * @return �ļ��еĵ�ǰ��/дλ�ã�ʧ���򷵻�-1��������뽫���뵱ǰ�̵߳�errno�С�
 */
off_t lseek(int fd, off_t offset, int whence)
{
    int result;
    struct dfs_fd *d;

    d = fd_get(fd);
    if (d == NULL)
    {
        rt_set_errno(-EBADF);

        return -1;
    }

    switch (whence)
    {
    case SEEK_SET:
        break;

    case SEEK_CUR:
        offset += d->pos;
        break;

    case SEEK_END:
        offset += d->size;
        break;

    default:
        fd_put(d);
        rt_set_errno(-EINVAL);

        return -1;
    }

    if (offset < 0)
    {
        fd_put(d);
        rt_set_errno(-EINVAL);

        return -1;
    }
    result = dfs_file_lseek(d, offset);
    if (result < 0)
    {
        fd_put(d);
        rt_set_errno(result);

        return -1;
    }

    /* release the ref-count of fd */
    fd_put(d);

    return offset;
}
RTM_EXPORT(lseek);

/**
 * @brief ������
 *
 * �ú����Ὣ����old ��ָ�����ļ����Ƹ�Ϊ����new ��ָ���ļ����ơ���new
 * ��ָ�����ļ��Ѿ����ڣ�����ļ����ᱻ���ǡ�
 *
 * @param old ���ļ���
 * @param new ���ļ���
 *
 * @return 0 �ɹ���-1 ʧ�ܣ�������뽫���뵱ǰ�̵߳�errno�С�
 *
 * @note: ���ļ��������ļ�����������ͬһ�ļ�ϵͳ��
 */
int rename(const char *old, const char *new)
{
    int result;

    result = dfs_file_rename(old, new);
    if (result < 0)
    {
        rt_set_errno(result);

        return -1;
    }

    return 0;
}
RTM_EXPORT(rename);

/**
 * @brief ɾ���ļ�
 *
 * �ú�����ɾ��ָ��Ŀ¼�µ��ļ���
 *
 * @param pathname ָ��ɾ���ļ��ľ���·��
 *
 * @return 0 �ɹ���-1 ʧ�ܣ�������뽫���뵱ǰ�̵߳�errno�С�
 */
int unlink(const char *pathname)
{
    int result;

    result = dfs_file_unlink(pathname);
    if (result < 0)
    {
        rt_set_errno(result);

        return -1;
    }

    return 0;
}
RTM_EXPORT(unlink);

#ifndef _WIN32 /* we can not implement these functions */
/**
 * @brief ȡ��״̬
 *
 * ���ô˺����ɻ���ļ�״̬��
 *
 * @param file �ļ���
 * @param buf �ṹָ�룬ָ��һ������ļ�״̬��Ϣ�Ľṹ��
 *
 * @return 0 �ɹ���-1 ʧ�ܣ�������뽫���뵱ǰ�̵߳�errno�С�
 */
int stat(const char *file, struct stat *buf)
{
    int result;

    result = dfs_file_stat(file, buf);
    if (result < 0)
    {
        rt_set_errno(result);

        return -1;
    }

    return result;
}
RTM_EXPORT(stat);

/**
 * @brief ȡ���ļ�״̬
 *
 * �ú�������������fildes��ָ���ļ�״̬�����Ƶ�����buf��ָ�Ľṹ��(struct stat)��
 *
 * @param fildes �Ѵ򿪵��ļ����ļ�������
 * @param buf �ṹָ�룬ָ��һ������ļ�״̬��Ϣ�Ľṹ��
 *
 * @return �ɹ��򷵻�0��ʧ�ܷ���-1��������뽫���뵱ǰ�̵߳�errno�С�
 *
 * @note �������� stat() �������ƣ���ͬ���ǣ������������Ѵ򿪵��ļ����ļ��������������ļ�����
 */
int fstat(int fildes, struct stat *buf)
{
    struct dfs_fd *d;

    /* get the fd */
    d = fd_get(fildes);
    if (d == NULL)
    {
        rt_set_errno(-EBADF);

        return -1;
    }

    /* it's the root directory */
    buf->st_dev = 0;

    buf->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                   S_IWUSR | S_IWGRP | S_IWOTH;
    if (d->type == FT_DIRECTORY)
    {
        buf->st_mode &= ~S_IFREG;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    }

    buf->st_size    = d->size;
    buf->st_mtime   = 0;

    fd_put(d);

    return RT_EOK;
}
RTM_EXPORT(fstat);
#endif

/**
 * @brief ͬ���ļ����ݵ��洢�豸
 *
 * �ú�������ͬ���ڴ����������޸ĵ��ļ����ݵ������豸��
 *
 * @param fildes �ļ�������
 *
 * @return RT_EOK ͬ���ļ��ɹ���-RT_ERROR ͬ���ļ�ʧ��
 *
 */
int fsync(int fildes)
{
    int ret;
    struct dfs_fd *d;

    /* get the fd */
    d = fd_get(fildes);
    if (d == NULL)
    {
        rt_set_errno(-EBADF);
        return -1;
    }

    ret = dfs_file_flush(d);

    fd_put(d);
    return ret;
}
RTM_EXPORT(fsync);

/*
 * http://blog.chinaunix.net/uid-29261327-id-4230156.html
 * this function is a POSIX compliant version, which shall perform a variety of
 * control functions on devices.
 *
 * @param fildes the file description
 * @param cmd the specified command
 * @param data represents the additional information that is needed by this
 * specific device to perform the requested function.
 *
 * @return 0 on successful completion. Otherwise, -1 shall be returned and errno
 * set to indicate the error.
 */
int fcntl(int fildes, int cmd, ...)
{
    int ret = -1;
    struct dfs_fd *d;

    /* get the fd */
    d = fd_get(fildes);
    if (d)
    {
        void* arg;
        va_list ap;

        va_start(ap, cmd);
        arg = va_arg(ap, void*);
        va_end(ap);

        ret = dfs_file_ioctl(d, cmd, arg);
        fd_put(d);
    }
    else ret = -EBADF;

    if (ret != 0)
    {
        rt_set_errno(ret);
        ret = -1;
    }

    return 0;
}
RTM_EXPORT(fcntl);

/*
 * this function is a POSIX compliant version, which shall perform a variety of
 * control functions on devices.
 *
 * @param fildes the file description
 * @param cmd the specified command
 * @param data represents the additional information that is needed by this
 * specific device to perform the requested function.
 *
 * @return 0 on successful completion. Otherwise, -1 shall be returned and errno
 * set to indicate the error.
 */
int ioctl(int fildes, int cmd, ...)
{
    void* arg;
    va_list ap;

    va_start(ap, cmd);
    arg = va_arg(ap, void*);
    va_end(ap);

    /* we use fcntl for this API. */
    return fcntl(fildes, cmd, arg);
}
RTM_EXPORT(ioctl);

/**
 * @brief ��ѯ�ļ�ϵͳ�����Ϣ
 *
 *
 * @param path ��Ҫ��ѯ��Ϣ���ļ�ϵͳ���ļ�·����
 * @param buf ���ڴ����ļ�ϵͳ�����Ϣ�Ľṹ��ָ��
 *
 * @return �ɹ� 0��ʧ�� -1��������뽫���뵱ǰ�̵߳�errno�С�
 */
int statfs(const char *path, struct statfs *buf)
{
    int result;

    result = dfs_statfs(path, buf);
    if (result < 0)
    {
        rt_set_errno(result);

        return -1;
    }

    return result;
}
RTM_EXPORT(statfs);

/**
 * @brief ����Ŀ¼
 *
 * @param path Ŀ¼�ľ��Ե�ַ
 * @param mode ����ģʽ����ǰδ���ã�����Ĭ�ϲ���0x777���ɡ�
 *
 * @return �ɹ� 0��ʧ�� -1��������뽫���뵱ǰ�̵߳�errno�С�
 */
int mkdir(const char *path, mode_t mode)
{
    int fd;
    struct dfs_fd *d;
    int result;

    fd = fd_new();
    if (fd == -1)
    {
        rt_set_errno(-ENOMEM);

        return -1;
    }

    d = fd_get(fd);

    result = dfs_file_open(d, path, O_DIRECTORY | O_CREAT);

    if (result < 0)
    {
        fd_put(d);
        fd_put(d);
        rt_set_errno(result);

        return -1;
    }

    dfs_file_close(d);
    fd_put(d);
    fd_put(d);

    return 0;
}
RTM_EXPORT(mkdir);

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(mkdir, create a directory);
#endif

/**
 * @brief ɾ��Ŀ¼
 *
 * @param pathname ��Ҫɾ��Ŀ¼�ľ���·��
 *
 * @return �ɹ� 0��ʧ�� -1��������뽫���뵱ǰ�̵߳�errno�С�
 */
int rmdir(const char *pathname)
{
    int result;

    result = dfs_file_unlink(pathname);
    if (result < 0)
    {
        rt_set_errno(result);

        return -1;
    }

    return 0;
}
RTM_EXPORT(rmdir);

/**
 * @brief ��Ŀ¼
 *
 * @param name Ŀ¼�ľ��Ե�ַ
 *
 * @return ���ļ��ɹ�������ָ��Ŀ¼��DIR�ṹָ��, ʧ���򷵻� NULL��
 */
DIR *opendir(const char *name)
{
    struct dfs_fd *d;
    int fd, result;
    DIR *t;

    t = NULL;

    /* allocate a fd */
    fd = fd_new();
    if (fd == -1)
    {
        rt_set_errno(-ENOMEM);

        return NULL;
    }
    d = fd_get(fd);

    result = dfs_file_open(d, name, O_RDONLY | O_DIRECTORY);
    if (result >= 0)
    {
        /* open successfully */
        t = (DIR *) rt_malloc(sizeof(DIR));
        if (t == NULL)
        {
            dfs_file_close(d);
            fd_put(d);
        }
        else
        {
            memset(t, 0, sizeof(DIR));

            t->fd = fd;
        }
        fd_put(d);

        return t;
    }

    /* open failed */
    fd_put(d);
    fd_put(d);
    rt_set_errno(result);

    return NULL;
}
RTM_EXPORT(opendir);

/**
 * @brief ��ȡĿ¼
 *
 * �ú���������ȡĿ¼��ÿ��ȡһ��Ŀ¼��Ŀ¼����ָ��λ�ý��Զ��������1��λ�á�
 *
 * @param d Ŀ¼·����
 *
 * @return ��ȡ�ɹ����ر���Ŀ¼��Ϣ�Ľṹ��ָ�룬��ȡʧ���򷵻�NULL��
 */
struct dirent *readdir(DIR *d)
{
    int result;
    struct dfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        rt_set_errno(-EBADF);
        return NULL;
    }

    if (d->num)
    {
        struct dirent* dirent_ptr;
        dirent_ptr = (struct dirent*)&d->buf[d->cur];
        d->cur += dirent_ptr->d_reclen;
    }

    if (!d->num || d->cur >= d->num)
    {
        /* get a new entry */
        result = dfs_file_getdents(fd,
                                   (struct dirent*)d->buf,
                                   sizeof(d->buf) - 1);
        if (result <= 0)
        {
            fd_put(fd);
            rt_set_errno(result);

            return NULL;
        }

        d->num = result;
        d->cur = 0; /* current entry index */
    }

    fd_put(fd);

    return (struct dirent *)(d->buf+d->cur);
}
RTM_EXPORT(readdir);

/**
 * @brief ��ȡĿ¼���Ķ�ȡλ��
 *
 * �ú����ķ���ֵ��¼��һ��Ŀ¼���ĵ�ǰλ�ã��˷���ֵ�������Ŀ¼�ļ���ͷ��
 * ƫ����������ֵ�����¸���ȡλ�á������������seekdir() �����������������ֵ
 * ������Ŀ¼ɨ�赽��ǰλ�á�Ҳ����˵telldir() �������Ժ�seekdir() �������ʹ�ã�
 * ��������Ŀ¼���Ķ�ȡλ�õ�ָ����ƫ������
 *
 * @param d Ŀ¼·����
 *
 * @return ��ȡλ�õ�ƫ����
 */
long telldir(DIR *d)
{
    struct dfs_fd *fd;
    long result;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        rt_set_errno(-EBADF);

        return 0;
    }

    result = fd->pos - d->num + d->cur;
    fd_put(fd);

    return result;
}
RTM_EXPORT(telldir);

/**
 * @brief �����´ζ�ȡĿ¼��λ��
 *
 * �ú����������ò���dĿ¼��Ŀǰ�Ķ�ȡλ�ã��ڵ���readdir() ʱ��Ӵ���λ�ÿ�ʼ��ȡ��
 *
 * @param d Ŀ¼·����
 * @param offset ƫ��ֵ�����뱾��Ŀ¼��λ��
 */
void seekdir(DIR *d, off_t offset)
{
    struct dfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        rt_set_errno(-EBADF);

        return ;
    }

    /* seek to the offset position of directory */
    if (dfs_file_lseek(fd, offset) >= 0)
        d->num = d->cur = 0;
    fd_put(fd);
}
RTM_EXPORT(seekdir);

/**
 * @brief ����Ŀ¼���Ķ�ȡλ��Ϊ��ͷλ��
 *
 * �ú��������������� d Ŀ¼��Ŀǰ�Ķ�ȡλ��ΪĿ¼���ĳ�ʼλ�á�
 *
 * @param d Ŀ¼·����
 */
void rewinddir(DIR *d)
{
    struct dfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        rt_set_errno(-EBADF);

        return ;
    }

    /* seek to the beginning of directory */
    if (dfs_file_lseek(fd, 0) >= 0)
        d->num = d->cur = 0;
    fd_put(fd);
}
RTM_EXPORT(rewinddir);

/**
 * @brief �ر�Ŀ¼
 *
 * �ú��������ر�һ��Ŀ¼���ú��������opendir() �����ɶ�ʹ�á�
 *
 * @param d Ŀ¼·����
 *
 * @return �ɹ� 0��ʧ�� -1��������뽫���뵱ǰ�̵߳�errno�С�
 */
int closedir(DIR *d)
{
    int result;
    struct dfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        rt_set_errno(-EBADF);

        return -1;
    }

    result = dfs_file_close(fd);
    fd_put(fd);

    fd_put(fd);
    rt_free(d);

    if (result < 0)
    {
        rt_set_errno(result);

        return -1;
    }
    else
        return 0;
}
RTM_EXPORT(closedir);

#ifdef DFS_USING_WORKDIR
/*
 * @brief �ı䵱ǰ����Ŀ¼
 *
 * @param path Ҫ���ĵ�·������
 *
 * @return �ɹ� 0��ʧ�� -1��������뽫���뵱ǰ�̵߳�errno�С�
 */
int chdir(const char *path)
{
    char *fullpath;
    DIR *d;

    if (path == NULL)
    {
        dfs_lock();
        rt_kprintf("%s\n", working_directory);
        dfs_unlock();

        return 0;
    }

    if (strlen(path) > DFS_PATH_MAX)
    {
        rt_set_errno(-ENOTDIR);

        return -1;
    }

    fullpath = dfs_normalize_path(NULL, path);
    if (fullpath == NULL)
    {
        rt_set_errno(-ENOTDIR);

        return -1; /* build path failed */
    }

    dfs_lock();
    d = opendir(fullpath);
    if (d == NULL)
    {
        rt_free(fullpath);
        /* this is a not exist directory */
        dfs_unlock();

        return -1;
    }

    /* close directory stream */
    closedir(d);

    /* copy full path to working directory */
    strncpy(working_directory, fullpath, DFS_PATH_MAX);
    /* release normalize directory path name */
    rt_free(fullpath);

    dfs_unlock();

    return 0;
}
RTM_EXPORT(chdir);

#ifdef RT_USING_FINSH
FINSH_FUNCTION_EXPORT_ALIAS(chdir, cd, change current working directory);
#endif
#endif

/**
 * @brief ����ļ��Ƿ����
 *
 * �ú���������amode�а�����λģʽ�����path����ָ���·������ָ�����ļ���
 *
 * @param path ָ�����ļ�/Ŀ¼·��
 * @param amode ��ֵ������Ҫ���ķ���Ȩ�ޣ�R_OK��W_OK��X_OK������ڲ��ԣ�F_OK��
 *
 * @return ������������򷵻�0; ���򷵻�-1��
 */
int access(const char *path, int amode)
{
    struct stat sb;
    if (stat(path, &sb) < 0)
        return -1; /* already sets errno */

    /* ignore R_OK,W_OK,X_OK condition */
    return 0;
}

/*
 * @brief ��ȡ��ǰ����Ŀ¼
 *
 * @param buf ���浱ǰ����Ŀ¼����·���Ļ�����
 * @param size ��������С
 *
 * @return ���ص�ǰ����Ŀ¼��
 */
char *getcwd(char *buf, size_t size)
{
#ifdef DFS_USING_WORKDIR
    rt_enter_critical();
    strncpy(buf, working_directory, size);
    rt_exit_critical();
#else
    rt_kprintf(NO_WORKING_DIR);
#endif

    return buf;
}
RTM_EXPORT(getcwd);

/* @} */
