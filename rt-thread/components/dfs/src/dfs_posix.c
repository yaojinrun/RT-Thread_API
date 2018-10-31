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
 * @brief 打开文件
 *
 * 此函数是POSIX兼容版本，它将打开文件并根据指定的标志返回文件描述符。
 *
 * @param file 打开或创建的文件名
 * @param flags 指定打开文件的方式
 *
 * 当前支持的打开方式有以下几种：
 *
 * flags             | 描述
 * ------------------|------------------------------------
 * O_RDONLY   | 只读方式打开
 * O_WRONLY  |  只写方式打开
 * O_RDWR    | 以读写方式打开
 * O_CREAT   | 如果要打开的文件不存在，则建立该文件
 * O_APPEND  | 当读写文件时会从文件尾开始移动，也就是说写入的数据会以附加的方式添加到文件的尾部。
 * O_TRUNC    | 如果文件已经存在，则清空文件中的内容
 *
 * @return 成功则返回打开文件描述符, 失败则返回-1，错误代码将存入当前线程的errno中。
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
 * @brief 关闭文件
 *
 * 此函数是POSIX兼容版本，它将关闭指定的文件。
 *
 * @param fd 文件描述符
 *
 * @return 成功 0，失败 -1，错误代码将存入当前线程的errno中。
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
 * @brief 读取数据
 *
 * 该函数接口会把参数fd 所指的文件的len 个字节读取到buf 指针所指的内存中，
 * 文件的读写位置会随读取到的字节移动。
 *
 * @param fd 文件描述符
 * @param buf 读取数据将要写入的缓存地址指针
 * @param len 读取文件的字节数
 *
 * @return 实际读取到的字节数。 有两种情况会返回0值，一是读取数据已到达文件结尾，
 * 二是无可读取的数据（例如设定len为0）。读取失败则返回-1，错误代码将存入当前线程的errno中。
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
 * @brief 写入数据
 *
 * 函数接口会把buf 指针所指向的内存中len 个字节写入到参数fd 所指向的文件内，
 * 文件的读写位置会写入的字节移动。
 *
 * @param fd 文件描述符
 * @param buf 预写入数据的地址指针
 * @param len 预写入文件的字节数
 *
 * @return 实际写入文件的字节数，失败则返回-1，错误代码将存入当前线程的errno中。
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
 * @brief 移动文件读写指针
 *
 * 该函数的主要作用是移动文件读写指针。
 *
 * @param fd 表示要操作的文件描述符
 * @param offset 相对于whence（基准）的偏移量
 * @param whence 指出偏移的方式，取值如下：
 * - SEEK_SET 将读写位置指向文件头后再增加offset个位移量。
 * - SEEK_CUR 以目前的读写位置往后增加offset个位移量。
 * - SEEK_END 将读写位置指向文件尾后再增加offset个位移量。
 *
 * @return 文件中的当前读/写位置；失败则返回-1，错误代码将存入当前线程的errno中。
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
 * @brief 重命名
 *
 * 该函数会将参数old 所指定的文件名称改为参数new 所指的文件名称。若new
 * 所指定的文件已经存在，则该文件将会被覆盖。
 *
 * @param old 旧文件名
 * @param new 新文件名
 *
 * @return 0 成功，-1 失败，错误代码将存入当前线程的errno中。
 *
 * @note: 旧文件名和新文件名必须属于同一文件系统。
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
 * @brief 删除文件
 *
 * 该函数可删除指定目录下的文件。
 *
 * @param pathname 指定删除文件的绝对路径
 *
 * @return 0 成功，-1 失败，错误代码将存入当前线程的errno中。
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
 * @brief 取得状态
 *
 * 调用此函数可获得文件状态。
 *
 * @param file 文件名
 * @param buf 结构指针，指向一个存放文件状态信息的结构体
 *
 * @return 0 成功，-1 失败，错误代码将存入当前线程的errno中。
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
 * @brief 取得文件状态
 *
 * 该函数用来将参数fildes所指的文件状态，复制到参数buf所指的结构中(struct stat)。
 *
 * @param fildes 已打开的文件的文件描述符
 * @param buf 结构指针，指向一个存放文件状态信息的结构体
 *
 * @return 成功则返回0，失败返回-1，错误代码将存入当前线程的errno中。
 *
 * @note 本函数与 stat() 函数相似，不同的是，它是作用于已打开的文件的文件描述符而不是文件名。
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
 * @brief 同步文件数据到存储设备
 *
 * 该函数可以同步内存中所有已修改的文件数据到储存设备。
 *
 * @param fildes 文件描述符
 *
 * @return RT_EOK 同步文件成功，-RT_ERROR 同步文件失败
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
 * @brief 查询文件系统相关信息
 *
 *
 * @param path 需要查询信息的文件系统的文件路径名
 * @param buf 用于储存文件系统相关信息的结构体指针
 *
 * @return 成功 0，失败 -1，错误代码将存入当前线程的errno中。
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
 * @brief 创建目录
 *
 * @param path 目录的绝对地址
 * @param mode 创建模式，当前未启用，填入默认参数0x777即可。
 *
 * @return 成功 0，失败 -1，错误代码将存入当前线程的errno中。
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
 * @brief 删除目录
 *
 * @param pathname 需要删除目录的绝对路径
 *
 * @return 成功 0，失败 -1，错误代码将存入当前线程的errno中。
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
 * @brief 打开目录
 *
 * @param name 目录的绝对地址
 *
 * @return 打开文件成功，返回指向目录的DIR结构指针, 失败则返回 NULL。
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
 * @brief 读取目录
 *
 * 该函数用来读取目录，每读取一次目录，目录流的指针位置将自动往后递推1个位置。
 *
 * @param d 目录路径名
 *
 * @return 读取成功返回保存目录信息的结构体指针，读取失败则返回NULL。
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
 * @brief 获取目录流的读取位置
 *
 * 该函数的返回值记录着一个目录流的当前位置，此返回值代表距离目录文件开头的
 * 偏移量，返回值返回下个读取位置。你可以在随后的seekdir() 函数调用中利用这个值
 * 来重置目录扫描到当前位置。也就是说telldir() 函数可以和seekdir() 函数配合使用，
 * 重新设置目录流的读取位置到指定的偏移量。
 *
 * @param d 目录路径名
 *
 * @return 读取位置的偏移量
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
 * @brief 设置下次读取目录的位置
 *
 * 该函数用来设置参数d目录流目前的读取位置，在调用readdir() 时便从此新位置开始读取。
 *
 * @param d 目录路径名
 * @param offset 偏移值，距离本次目录的位移
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
 * @brief 重设目录流的读取位置为开头位置
 *
 * 该函数可以用来设置 d 目录流目前的读取位置为目录流的初始位置。
 *
 * @param d 目录路径名
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
 * @brief 关闭目录
 *
 * 该函数用来关闭一个目录，该函数必须和opendir() 函数成对使用。
 *
 * @param d 目录路径名
 *
 * @return 成功 0，失败 -1，错误代码将存入当前线程的errno中。
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
 * @brief 改变当前工作目录
 *
 * @param path 要更改的路径名。
 *
 * @return 成功 0，失败 -1，错误代码将存入当前线程的errno中。
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
 * @brief 检查文件是否存在
 *
 * 该函数将根据amode中包含的位模式检查由path参数指向的路径名所指定的文件。
 *
 * @param path 指定的文件/目录路径
 * @param amode 该值可以是要检查的访问权限（R_OK，W_OK，X_OK）或存在测试（F_OK）
 *
 * @return 允许访问请求，则返回0; 否则返回-1。
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
 * @brief 获取当前工作目录
 *
 * @param buf 保存当前工作目录绝对路径的缓冲区
 * @param size 缓冲区大小
 *
 * @return 返回当前工作目录。
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
