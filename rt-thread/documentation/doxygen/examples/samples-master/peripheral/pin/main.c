/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
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
 * 2018-1-20       DQL         the first version 
 */

#include <rthw.h>
#include <rtthread.h>

#include <drivers/pin.h>

#define LED0 21 //PF9--21���� drv_gpio.c �ļ� pin_index pins[]�в鵽 PF9 ���Ϊ 21
#define LED1 22 //PF10--21���� drv_gpio.c �ļ� pin_index pins[]�в鵽 PF110 ���Ϊ 22

#define KEY0    3 //PE4--3���� drv_gpio.c �ļ� pin_index pins[]�в鵽 PE4 ���Ϊ 3
#define KEY1    2	//PE3--2���� drv_gpio.c �ļ� pin_index pins[]�в鵽 PE3 ���Ϊ 2
#define KEY2    1	//PE2--1���� drv_gpio.c �ļ� pin_index pins[]�в鵽 PE2 ���Ϊ 1
#define WK_UP   34	//PA0--34���� drv_gpio.c �ļ� pin_index pins[]�в鵽 PA0 ���Ϊ 34

//�ܽ��ⲿ�жϻص�����
void hdr_callback(void *args)
{
    //��ȡ����
    char *a = args;
    rt_kprintf("key0 down! %s\n",a);
}

void irq_thread_entry(void* parameter)
{    
	//��������
    rt_pin_mode(KEY0, PIN_MODE_INPUT_PULLUP);
    //���жϣ��½���ģʽ���ص�������Ϊhdr_callback
    rt_pin_attach_irq(KEY0, PIN_IRQ_MODE_FALLING,hdr_callback,(void*)"callback args");
    //ʹ���ж�
    rt_pin_irq_enable(KEY0, PIN_IRQ_ENABLE);    
           
}

void key_thread_entry(void* parameter)
{    
   
    //PE1��PE2������������
    rt_pin_mode(KEY1, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY2, PIN_MODE_INPUT_PULLUP);
    //PA0����Ϊ��������
    rt_pin_mode(WK_UP, PIN_MODE_INPUT_PULLDOWN);
          
    while (1)
    {   
        //��⵽�͵�ƽ��������1������
        if (rt_pin_read(KEY1) == PIN_LOW)
        {
            rt_kprintf("key1 pressed!\n");
        }
        //��⵽�͵�ƽ��������2������
        if (rt_pin_read(KEY2) == PIN_LOW)
        {
            rt_kprintf("key2 pressed!\n");
        }
        //��⵽�ߵ�ƽ��������wp������
        if (rt_pin_read(WK_UP) == PIN_HIGH)
        {
            rt_kprintf("WK_UP pressed!\n");
        }
        //����10ms
        rt_thread_delay(rt_tick_from_millisecond(10));        
    }            
}

void led_thread_entry(void* parameter)
{    
    //���ùܽ�Ϊ���ģʽ      
    rt_pin_mode(LED0, PIN_MODE_OUTPUT); 
    //���ùܽ�Ϊ���ģʽ
    rt_pin_mode(LED1, PIN_MODE_OUTPUT); 
         
    while (1)
    {   
        //����͵�ƽ��LED0 ��
        rt_pin_write(LED0, PIN_LOW);
        //����͵�ƽ��LED1 ��   
        rt_pin_write(LED1, PIN_LOW);     
        //���� 500ms
        rt_thread_delay(rt_tick_from_millisecond(500));
        //����ߵ�ƽ��LED0 ��
        rt_pin_write(LED0, PIN_HIGH);
        //����ߵ�ƽ��LED1 ��
        rt_pin_write(LED1, PIN_HIGH);
        //���� 500ms
        rt_thread_delay(rt_tick_from_millisecond(500));       
    }            
}

int main(void)
{
    
    rt_thread_t tid;
       
    /* ����key�߳� */
    tid = rt_thread_create("key",
                    key_thread_entry, 
                    RT_NULL,
                    1024, 
                    2, 
                    10);
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    
    /* ����led�߳� */
    tid = rt_thread_create("led",
                    led_thread_entry, 
                    RT_NULL,
                    1024, 
                    3, 
                    10);
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    
    /* ����irq�߳� */
    tid = rt_thread_create("exirq",
                    irq_thread_entry, 
                    RT_NULL,
                    1024, 
                    4, 
                    10);
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);
              
    return 0;
}

