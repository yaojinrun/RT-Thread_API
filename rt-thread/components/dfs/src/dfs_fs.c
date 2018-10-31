/*
 * File      : dfs_fs.c
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
 * 2005-02-22     Bernard      The first version.
 * 2010-06-30     Bernard      Optimize for RT-Thread RTOS
 * 2011-03-12     Bernard      fix the filesystem lookup issue.
 * 2017-11-30     Bernard      fix the filesystem_operation_table issue.
 * 2017-12-05     Bernard      fix the fs type search issue in mkfs.
 */

#include <dfs_fs.h>
#include <dfs_file.h>
#include "dfs_private.h"

/**
 * @addtogroup FsApi
 */
/*@{*/

/**
 * @brief 注册文件系统
 *
 * 在使用文件系统之前需要将文件系统注册到DFS框架中，可以调用该函数来实现。
 *
 * @param ops 文件系统操作方法句柄
 *
 * @return 0 注册成功，-1 失败。
 */
int dfs_register(const struct dfs_filesystem_ops *ops)
{
    int ret = RT_EOK;
    const struct dfs_filesystem_ops **empty = NULL;
    const struct dfs_filesystem_ops **iter;

    /* lock filesystem */
    dfs_lock();
    /* check if this filesystem was already registered */
    for (iter = &filesystem_operation_table[0];
           iter < &filesystem_operation_table[DFS_FILESYSTEM_TYPES_MAX]; iter ++)
    {
        /* find out an empty filesystem type entry */
        if (*iter == NULL)
            (empty == NULL) ? (empty = iter) : 0;
        else if (strcmp((*iter)->name, ops->name) == 0)
        {
            rt_set_errno(-EEXIST);
            ret = -1;
            break;
        }
    }

    /* save the filesystem's operations */
    if (empty == NULL)
    {
        rt_set_errno(-ENOSPC);
        ret = -1;
    }
    else if (ret == RT_EOK)
    {
        *empty = ops;
    }

    dfs_unlock();
    return ret;
}

/**
 * @brief 查找指定路径上的文件系统
 *
 * 此函数将返回挂载在指定路径上的文件系统。
 *
 * @param path 路径字符串。
 *
 * @return 成功返回挂载的文件系统句柄，失败则返回NULL。
 */
struct dfs_filesystem *dfs_filesystem_lookup(const char *path)
{
    struct dfs_filesystem *iter;
    struct dfs_filesystem *fs = NULL;
    uint32_t fspath, prefixlen;

    prefixlen = 0;

    RT_ASSERT(path);

    /* lock filesystem */
    dfs_lock();

    /* lookup it in the filesystem table */
    for (iter = &filesystem_table[0];
            iter < &filesystem_table[DFS_FILESYSTEMS_MAX]; iter++)
    {
        if ((iter->path == NULL) || (iter->ops == NULL))
            continue;

        fspath = strlen(iter->path);
        if ((fspath < prefixlen)
            || (strncmp(iter->path, path, fspath) != 0))
            continue;

        /* check next path separator */
        if (fspath > 1 && (strlen(path) > fspath) && (path[fspath] != '/'))
            continue;

        fs = iter;
        prefixlen = fspath;
    }

    dfs_unlock();

    return fs;
}

/**
 * @brief 返回挂载路径
 *
 * 此函数将返回指定设备挂载路径。
 *
 * @param device 已挂载的设备对象。
 *
 * @return 成功返回已挂载路径；失败则返回NULL。
 */
const char* dfs_filesystem_get_mounted_path(struct rt_device* device)
{
    const char* path = NULL;
    struct dfs_filesystem *iter;

    dfs_lock();
    for (iter = &filesystem_table[0];
            iter < &filesystem_table[DFS_FILESYSTEMS_MAX]; iter++)
    {
        /* fint the mounted device */
        if (iter->ops == NULL) continue;
        else if (iter->dev_id == device)
        {
            path = iter->path;
            break;
        }
    }

    /* release filesystem_table lock */
    dfs_unlock();

    return path;
}

/**
 * @brief 获取分区表
 *
 * 此函数将获取指定缓冲区上的分区表。
 *
 * @param part 返回的分区表结构。
 * @param buf 存储分区表的缓冲区。
 * @param pindex 要获取的分区表的索引。
 *
 * @return RT_EOK 成功；-RT_ERROR 失败。
 */
int dfs_filesystem_get_partition(struct dfs_partition *part,
                                      uint8_t         *buf,
                                      uint32_t        pindex)
{
#define DPT_ADDRESS     0x1be       /* device partition offset in Boot Sector */
#define DPT_ITEM_SIZE   16          /* partition item size */

    uint8_t *dpt;
    uint8_t type;

    RT_ASSERT(part != NULL);
    RT_ASSERT(buf != NULL);

    dpt = buf + DPT_ADDRESS + pindex * DPT_ITEM_SIZE;

    /* check if it is a valid partition table */
    if ((*dpt != 0x80) && (*dpt != 0x00))
        return -EIO;

    /* get partition type */
    type = *(dpt+4);
    if (type == 0)
        return -EIO;

    /* set partition information
     *    size is the number of 512-Byte */
    part->type = type;
    part->offset = *(dpt+8) | *(dpt+9)<<8 | *(dpt+10)<<16 | *(dpt+11)<<24;
    part->size = *(dpt+12) | *(dpt+13)<<8 | *(dpt+14)<<16 | *(dpt+15)<<24;

    rt_kprintf("found part[%d], begin: %d, size: ",
               pindex, part->offset*512);
    if ((part->size>>11) == 0)
        rt_kprintf("%d%s",part->size>>1,"KB\n");     /* KB */
    else
    {
        unsigned int part_size;
        part_size = part->size >> 11;                /* MB */
        if ((part_size>>10) == 0)
            rt_kprintf("%d.%d%s",part_size,(part->size>>1)&0x3FF,"MB\n");
        else
            rt_kprintf("%d.%d%s",part_size>>10,part_size&0x3FF,"GB\n");
    }

    return RT_EOK;
}

/**
 * @brief 挂载文件系统
 *
 * 该函数会将文件系统和具体的存储设备关联起来。
 *
 * @param device_name 一个包含了文件系统的设备名
 * @param path 挂载文件系统的路径，即挂载点
 * @param filesystemtype 需要挂载的文件系统类型
 * @param rwflag 读写标志位
 * @param data 文件系统的私有数据
 *
 * @return  0 挂载成功，-1 失败。
 */
int dfs_mount(const char   *device_name,
              const char   *path,
              const char   *filesystemtype,
              unsigned long rwflag,
              const void   *data)
{
    const struct dfs_filesystem_ops **ops;
    struct dfs_filesystem *iter;
    struct dfs_filesystem *fs = NULL;
    char *fullpath = NULL;
    rt_device_t dev_id;

    /* open specific device */
    if (device_name == NULL)
    {
        /* which is a non-device filesystem mount */
        dev_id = NULL;
    }
    else if ((dev_id = rt_device_find(device_name)) == NULL)
    {
        /* no this device */
        rt_set_errno(-ENODEV);
        return -1;
    }

    /* find out the specific filesystem */
    dfs_lock();

    for (ops = &filesystem_operation_table[0];
           ops < &filesystem_operation_table[DFS_FILESYSTEM_TYPES_MAX]; ops++)
        if ((*ops != NULL) && (strcmp((*ops)->name, filesystemtype) == 0))
            break;

    dfs_unlock();

    if (ops == &filesystem_operation_table[DFS_FILESYSTEM_TYPES_MAX])
    {
        /* can't find filesystem */
        rt_set_errno(-ENODEV);
        return -1;
    }

    /* check if there is mount implementation */
    if ((*ops == NULL) || ((*ops)->mount == NULL))
    {
        rt_set_errno(-ENOSYS);
        return -1;
    }

    /* make full path for special file */
    fullpath = dfs_normalize_path(NULL, path);
    if (fullpath == NULL) /* not an abstract path */
    {
        rt_set_errno(-ENOTDIR);
        return -1;
    }

    /* Check if the path exists or not, raw APIs call, fixme */
    if ((strcmp(fullpath, "/") != 0) && (strcmp(fullpath, "/dev") != 0))
    {
        struct dfs_fd fd;

        if (dfs_file_open(&fd, fullpath, O_RDONLY | O_DIRECTORY) < 0)
        {
            rt_free(fullpath);
            rt_set_errno(-ENOTDIR);

            return -1;
        }
        dfs_file_close(&fd);
    }

    /* check whether the file system mounted or not  in the filesystem table
     * if it is unmounted yet, find out an empty entry */
    dfs_lock();

    for (iter = &filesystem_table[0];
            iter < &filesystem_table[DFS_FILESYSTEMS_MAX]; iter++)
    {
        /* check if it is an empty filesystem table entry? if it is, save fs */
        if (iter->ops == NULL)
            (fs == NULL) ? (fs = iter) : 0;
        /* check if the PATH is mounted */
        else if (strcmp(iter->path, path) == 0)
        {
            rt_set_errno(-EINVAL);
            goto err1;
        }
    }

    if ((fs == NULL) && (iter == &filesystem_table[DFS_FILESYSTEMS_MAX]))
    {
        rt_set_errno(-ENOSPC);
        goto err1;
    }

    /* register file system */
    fs->path   = fullpath;
    fs->ops    = *ops;
    fs->dev_id = dev_id;
    /* release filesystem_table lock */
    dfs_unlock();

    /* open device, but do not check the status of device */
    if (dev_id != NULL)
    {
        if (rt_device_open(fs->dev_id,
                           RT_DEVICE_OFLAG_RDWR) != RT_EOK)
        {
            /* The underlaying device has error, clear the entry. */
            dfs_lock();
            memset(fs, 0, sizeof(struct dfs_filesystem));

            goto err1;
        }
    }

    /* call mount of this filesystem */
    if ((*ops)->mount(fs, rwflag, data) < 0)
    {
        /* close device */
        if (dev_id != NULL)
            rt_device_close(fs->dev_id);

        /* mount failed */
        dfs_lock();
        /* clear filesystem table entry */
        memset(fs, 0, sizeof(struct dfs_filesystem));

        goto err1;
    }

    return 0;

err1:
    dfs_unlock();
    rt_free(fullpath);

    return -1;
}

/**
 * @brief 卸载文件系统
 *
 * 当某个文件系统不需要再使用了，那么可以调用该函数将它卸载掉。
 *
 * @param specialfile 挂载了文件系统的指定路径
 *
 * @return  0 卸载成功，-1 失败。
 */
int dfs_unmount(const char *specialfile)
{
    char *fullpath;
    struct dfs_filesystem *iter;
    struct dfs_filesystem *fs = NULL;

    fullpath = dfs_normalize_path(NULL, specialfile);
    if (fullpath == NULL)
    {
        rt_set_errno(-ENOTDIR);

        return -1;
    }

    /* lock filesystem */
    dfs_lock();

    for (iter = &filesystem_table[0];
            iter < &filesystem_table[DFS_FILESYSTEMS_MAX]; iter++)
    {
        /* check if the PATH is mounted */
        if ((iter->path != NULL) && (strcmp(iter->path, fullpath) == 0))
        {
            fs = iter;
            break;
        }
    }

    if (fs == NULL ||
        fs->ops->unmount == NULL ||
        fs->ops->unmount(fs) < 0)
    {
        goto err1;
    }

    /* close device, but do not check the status of device */
    if (fs->dev_id != NULL)
        rt_device_close(fs->dev_id);

    if (fs->path != NULL)
        rt_free(fs->path);

    /* clear this filesystem table entry */
    memset(fs, 0, sizeof(struct dfs_filesystem));

    dfs_unlock();
    rt_free(fullpath);

    return 0;

err1:
    dfs_unlock();
    rt_free(fullpath);

    return -1;
}

/**
 * @brief 格式化文件系统
 *
 * 使用文件系统功能前，要先在存储器中创建指定类型的文件系统。
 *
 * @param fs_name 文件系统名
 * @param device_name 需要格式化文件系统的设备名
 *
 * @return  0 格式化成功，-1 失败。
 */
int dfs_mkfs(const char *fs_name, const char *device_name)
{
    int index;
    rt_device_t dev_id = NULL;

    /* check device name, and it should not be NULL */
    if (device_name != NULL)
        dev_id = rt_device_find(device_name);

    if (dev_id == NULL)
    {
        rt_set_errno(-ENODEV);
        return -1;
    }

    /* lock file system */
    dfs_lock();
    /* find the file system operations */
    for (index = 0; index <= DFS_FILESYSTEM_TYPES_MAX; index ++)
    {
        if (filesystem_operation_table[index] != NULL &&
            strcmp(filesystem_operation_table[index]->name, fs_name) == 0)
            break;
    }
    dfs_unlock();

    if (index <= DFS_FILESYSTEM_TYPES_MAX)
    {
        /* find file system operation */
        const struct dfs_filesystem_ops *ops = filesystem_operation_table[index];
        if (ops->mkfs == NULL)
        {
            rt_set_errno(-ENOSYS);
            return -1;
        }

        return ops->mkfs(dev_id);
    }

    rt_kprintf("Can not find the file system which named as %s.\n", fs_name);
    return -1;
}

/*
 * @brief 获取文件系统信息
 *
 * 该函数返回一个挂载的文件系统的信息。
 *
 * @param path 文件系统挂载的路径
 * @param buffer 用于储存文件系统相关信息的结构体指针
 *
 * @return  0 格式化成功，-1 失败。
 *
 */
int dfs_statfs(const char *path, struct statfs *buffer)
{
    struct dfs_filesystem *fs;

    fs = dfs_filesystem_lookup(path);
    if (fs != NULL)
    {
        if (fs->ops->statfs != NULL)
            return fs->ops->statfs(fs, buffer);
    }

    return -1;
}

#ifdef RT_USING_DFS_MNTTABLE
int dfs_mount_table(void)
{
    int index = 0;

    while (1)
    {
        if (mount_table[index].path == NULL) break;

        if (dfs_mount(mount_table[index].device_name,
                mount_table[index].path,
                mount_table[index].filesystemtype,
                mount_table[index].rwflag,
                mount_table[index].data) != 0)
        {
            rt_kprintf("mount fs[%s] on %s failed.\n", mount_table[index].filesystemtype,
                mount_table[index].path);
            return -RT_ERROR;
        }

        index ++;
    }
    return 0;
}
INIT_ENV_EXPORT(dfs_mount_table);
#endif

#ifdef RT_USING_FINSH
#include <finsh.h>
void mkfs(const char *fs_name, const char *device_name)
{
    dfs_mkfs(fs_name, device_name);
}
FINSH_FUNCTION_EXPORT(mkfs, make a file system);

int df(const char *path)
{
    int result;
    int minor = 0;
    long long cap;
    struct statfs buffer;

    int unit_index = 0;
    char *unit_str[] = {"KB", "MB", "GB"};

    result = dfs_statfs(path ? path : NULL, &buffer);
    if (result != 0)
    {
        rt_kprintf("dfs_statfs failed.\n");
        return -1;
    }

    cap = ((long long)buffer.f_bsize) * ((long long)buffer.f_bfree) / 1024LL;
    for (unit_index = 0; unit_index < 2; unit_index ++)
    {
        if (cap < 1024) break;

        minor = (cap % 1024) * 10 / 1024; /* only one decimal point */
        cap = cap / 1024;
    }

    rt_kprintf("disk free: %d.%d %s [ %d block, %d bytes per block ]\n",
        (unsigned long)cap, minor, unit_str[unit_index], buffer.f_bfree, buffer.f_bsize);
    return 0;
}
FINSH_FUNCTION_EXPORT(df, get disk free);
#endif

/* @} */
