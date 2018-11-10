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
 * @brief �򿪶�̬ģ��
 *
 * �����������`dlmodule_load`�Ĺ��ܣ�����ļ�ϵͳ�ϼ��ض�̬�⣬�����ض�̬��ľ��ָ�롣 
 *
 * @param pathname ��̬��·������
 * @param mode �򿪶�̬��ʱ��ģʽ����RT-Thread�в�δʹ��
 * 
 * @return �򿪳ɹ������ض�̬��ľ��ָ�루ʵ����struct dlmodule�ṹ��ָ�룩�����򷵻�NULL��
 */
void *dlopen (const char *filename, int flag);
const char *dlerror(void);
/**
 * @ingroup Module
 *
 * @brief ���ҷ���
 * 
 * ��������ڶ�̬��`handle`�в����Ƿ����`symbol`�ķ��ţ�������ڷ������ĵ�ַ��  
 *
 * @param handle ��̬������Ӧ����dlopen�ķ���ֵ
 * @param symbol Ҫ���صķ��ŵ�ַ
 * 
 * @return �򿪳ɹ������ض�Ӧ���ŵĵ�ַ�����򷵻�NULL��
 */
void *dlsym(void *handle, const char *symbol);
/**
 * @ingroup Module
 *
 * @brief �رն�̬��
 * 
 * ���������ر�`handle`ָ��Ķ�̬�⣬���ڴ���ж�ص�����Ҫע����ǣ�����̬��رպ�ԭ��
 * ͨ��`dlsym`���صķ��ŵ�ַ�����ٿ��á������Ȼ����ȥ���ʣ����ܻ�����fault���� 
 *
 * @param handle ��̬����
 * 
 * @return �رճɹ�������0�����򷵻ظ�����
 */
int dlclose (void *handle);

#endif
