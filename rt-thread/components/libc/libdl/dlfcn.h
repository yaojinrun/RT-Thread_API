/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Change Logs:
 * Date           Author      Notes
 * 2010-11-17     yi.qiu      first version
 */

#ifndef __DLFCN_H_
#define __DLFCN_H_

#define RTLD_LAZY       0x00000
#define RTLD_NOW        0x00001

#define RTLD_LOCAL      0x00000
#define RTLD_GLOBAL     0x10000

#define RTLD_DEFAULT    ((void*)1)
#define RTLD_NEXT       ((void*)2)

/**
 * @ingroup Module
 *
 * @brief 打开动态模块
 *
 * 这个函数类似`dlmodule_load`的功能，会从文件系统上加载动态库，并返回动态库的句柄指针。 
 *
 * @param pathname 动态库路径名称
 * @param mode 打开动态库时的模式，在RT-Thread中并未使用
 * 
 * @return 打开成功，返回动态库的句柄指针（实质是struct dlmodule结构体指针）；否则返回NULL。
 */
void *dlopen (const char *filename, int flag);
const char *dlerror(void);
/**
 * @ingroup Module
 *
 * @brief 查找符号
 * 
 * 这个函数在动态库`handle`中查找是否存在`symbol`的符号，如果存在返回它的地址。  
 *
 * @param handle 动态库句柄，应该是dlopen的返回值
 * @param symbol 要返回的符号地址
 * 
 * @return 打开成功，返回对应符号的地址，否则返回NULL。
 */
void *dlsym(void *handle, const char *symbol);
/**
 * @ingroup Module
 *
 * @brief 关闭动态库
 * 
 * 这个函数会关闭`handle`指向的动态库，从内存中卸载掉。需要注意的是，当动态库关闭后，原来
 * 通过`dlsym`返回的符号地址将不再可用。如果依然尝试去访问，可能会引起fault错误。 
 *
 * @param handle 动态库句柄
 * 
 * @return 关闭成功，返回0；否则返回负数。
 */
int dlclose (void *handle);

#endif
