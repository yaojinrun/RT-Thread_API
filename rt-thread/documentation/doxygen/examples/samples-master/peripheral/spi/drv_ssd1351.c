/*
 * File      : drv_ssd1351.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
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
 * 2018-01-10     DQL        first implementation.
 */

#include "board.h"
#include "drv_ssd1351.h"

#define SPI_BUS_NAME                "spi1"
#define SPI_SSD1351_DEVICE_NAME     "spi10"
#define DC_PIN   96   /* PC6,you can find pin defines in gpio.c */
/*RES_PIN reset signal input. When the pin is low,
initialization of the chip is executed.*/
#define RES_PIN  97  /* PC7,you can find pin defines in gpio.c */
#define CS_PIN   98  /* PC8, ,pin�ܽű����drv_gpio.c��pin_index pins[]�п����ҵ�������ʹ�õ�STM32F407ZGT6Ϊ������оƬ�ܽ���Ϊ144 */

#define OLED_DEBUG

#ifdef  OLED_DEBUG
    #define OLED_TRACE         rt_kprintf
#else
    #define OLED_TRACE(...)
#endif

/*
pin defines for 4-wire spi

This pin is Data/Command control pin. When the pin is pulled high,
the input at D7~D0 is treated as display data.When the pin is pulled
low, the input at D7~D0 will be transferred to the command register.

When serial mode is selected, D1 will be the serial data input SDIN
and D0 will be the serial clock input SCLK.

STM32F407ZGT6 SPI1 default GPIOs
    PA5------SPI1SCK
    PA6------SPI1MISO
    PA7------SPI1MOSI

GPIO connections
  STM32F407ZGT6      SSD1351 OLED module
    PA5---SCK----------D0
    PA7---MOSI---------D1
    PC6----------------D/C
    PC7----------------RES
    PC8----------------CS

*/

struct stm32_hw_spi_cs
{
    rt_uint32_t pin;
};

static struct rt_spi_device spi_dev_ssd1351; /* SPI�豸ssd1351���� */
static struct stm32_hw_spi_cs  spi_cs;  /* SPI�豸CSƬѡ���� */


rt_err_t ssd1351_write_cmd(const rt_uint8_t cmd)
{
    rt_size_t len;

    rt_pin_write(DC_PIN, PIN_LOW);    /* ����͵�ƽ */

    len = rt_spi_send(&spi_dev_ssd1351, &cmd, 1);

    if (len != 1)
    {
        OLED_TRACE("ssd1351_write_cmd error. %d\r\n", len);
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }

}

rt_err_t ssd1351_write_data(const rt_uint8_t data)
{
    rt_size_t len;

    rt_pin_write(DC_PIN, PIN_HIGH);        /* ���ݸߵ�ƽ */

    len = rt_spi_send(&spi_dev_ssd1351, &data, 1);

    if (len != 1)
    {
        OLED_TRACE("ssd1351_write_data error. %d\r\n", len);
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

static int rt_hw_ssd1351_config(void)
{
    rt_err_t res;

    /* oled use PC8 as CS */
    spi_cs.pin = CS_PIN;
    rt_pin_mode(spi_cs.pin, PIN_MODE_OUTPUT);    /* ����Ƭѡ�ܽ�ģʽΪ��� */

    res = rt_spi_bus_attach_device(&spi_dev_ssd1351, SPI_SSD1351_DEVICE_NAME, SPI_BUS_NAME, (void *)&spi_cs);
    if (res != RT_EOK)
    {
        OLED_TRACE("rt_spi_bus_attach_device!\r\n");
        return res;
    }

    /* config spi */
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
        cfg.max_hz = 20 * 1000 * 1000; /* 20M,SPI max 42MHz,ssd1351 4-wire spi */

        rt_spi_configure(&spi_dev_ssd1351, &cfg);
    }

    return RT_EOK;
}

static int rt_hw_ssd1351_init(void)
{
    rt_hw_ssd1351_config();

    rt_pin_mode(DC_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(RES_PIN, PIN_MODE_OUTPUT);

    rt_pin_write(RES_PIN, PIN_LOW);
    //wait at least 100ms for reset
    rt_thread_delay(100);
    rt_pin_write(RES_PIN, PIN_HIGH);

    ssd1351_write_cmd(0xfd); // Command Lock
    ssd1351_write_data(0x12);

    ssd1351_write_cmd(0xfd); // Command Lock
    ssd1351_write_data(0xb1);

    //ssd1351_write_cmd(0xae); // Set Sleep Mode On
    ssd1351_write_cmd(0xae | 0x00);        // Display Off

    ssd1351_write_cmd(0xb3);//Set Display Clock Divide Ratio/Oscillator Frequency
    ssd1351_write_data(0xf1);

    ssd1351_write_cmd(0xca);//Set Multiplex Ratio
    ssd1351_write_data(0x7f);

    ssd1351_write_cmd(0xa2);//Set Display Offset
    ssd1351_write_data(0x00);

    ssd1351_write_cmd(0xa1);//Set Display Start Line
    ssd1351_write_data(0x00);

    ssd1351_write_cmd(0xa0);//Set Re-Map & Color Depth
    ssd1351_write_data(0x74);

    ssd1351_write_cmd(0xb5);//Set GPIO
    ssd1351_write_data(0x00);

    ssd1351_write_cmd(0xab);//Function Selection
    ssd1351_write_data(0x01);

    ssd1351_write_cmd(0xb4);//Set Segment Low Voltage
    ssd1351_write_data(0xa0);
    ssd1351_write_data(0xb5);
    ssd1351_write_data(0x55);

    ssd1351_write_cmd(0xc1);//Set Contrast Current
    ssd1351_write_data(0xc8);
    ssd1351_write_data(0x80);
    ssd1351_write_data(0xc8);

    ssd1351_write_cmd(0xc7);//Set Master Current Control
    ssd1351_write_data(0x0f);

    ssd1351_write_cmd(0xb8); //Gamma Look up Table
    ssd1351_write_data(2); //1
    ssd1351_write_data(3); //2
    ssd1351_write_data(4); //3
    ssd1351_write_data(5); //4
    ssd1351_write_data(6); //5
    ssd1351_write_data(7); //6
    ssd1351_write_data(8); //7
    ssd1351_write_data(9); //8
    ssd1351_write_data(10); //9
    ssd1351_write_data(11); //10
    ssd1351_write_data(12); //11
    ssd1351_write_data(13); //12
    ssd1351_write_data(14); //13
    ssd1351_write_data(15); //14
    ssd1351_write_data(16); //15
    ssd1351_write_data(17); //16
    ssd1351_write_data(18); //17
    ssd1351_write_data(19); //18
    ssd1351_write_data(21); //19
    ssd1351_write_data(23); //20
    ssd1351_write_data(25); //21
    ssd1351_write_data(27); //22
    ssd1351_write_data(29); //23
    ssd1351_write_data(31); //24
    ssd1351_write_data(33); //25
    ssd1351_write_data(35); //26
    ssd1351_write_data(37); //27
    ssd1351_write_data(39); //28
    ssd1351_write_data(42); //29
    ssd1351_write_data(45); //30
    ssd1351_write_data(48); //31
    ssd1351_write_data(51); //32
    ssd1351_write_data(54); //33
    ssd1351_write_data(57); //34
    ssd1351_write_data(60); //35
    ssd1351_write_data(63); //36
    ssd1351_write_data(66); //37
    ssd1351_write_data(69); //38
    ssd1351_write_data(72); //39
    ssd1351_write_data(76); //40
    ssd1351_write_data(80); //41
    ssd1351_write_data(84); //42
    ssd1351_write_data(88); //43
    ssd1351_write_data(92); //44
    ssd1351_write_data(96); //45
    ssd1351_write_data(100); //46
    ssd1351_write_data(104); //47
    ssd1351_write_data(108); //48
    ssd1351_write_data(112); //49
    ssd1351_write_data(116); //50
    ssd1351_write_data(120); //51
    ssd1351_write_data(125); //52
    ssd1351_write_data(130); //53
    ssd1351_write_data(135); //54
    ssd1351_write_data(140); //55
    ssd1351_write_data(145); //56
    ssd1351_write_data(150); //57
    ssd1351_write_data(155); //58
    ssd1351_write_data(160); //59
    ssd1351_write_data(165); //60
    ssd1351_write_data(170); //61
    ssd1351_write_data(175); //62
    ssd1351_write_data(180); //63

    ssd1351_write_cmd(0xb1); //Set Phase Length
    ssd1351_write_data(0x32);

    ssd1351_write_cmd(0xb2); //Enhance Driving Scheme Capability
    ssd1351_write_data(0xa4);
    ssd1351_write_data(0x00);
    ssd1351_write_data(0x00);

    ssd1351_write_cmd(0xbb); //Set Pre-Charge Voltage
    ssd1351_write_data(0x17);

    ssd1351_write_cmd(0xb6); // Set Second Pre-Charge Period
    ssd1351_write_data(0x01);

    ssd1351_write_cmd(0xbe); //Set VCOMH Voltage
    ssd1351_write_data(0x05);

    ssd1351_write_cmd(0xa6); //Set Display Mode

    //Clear Screen
    ssd1351_clear();

    //ssd1351_write_cmd(0xaf); //Set Sleep Mode Off
    ssd1351_write_cmd(0xae | 0x01); //Display On

    return 0;
}
INIT_PREV_EXPORT(rt_hw_ssd1351_init);   /* ʹ��RT-Thread������Զ���ʼ������ */


void set_column_address(rt_uint8_t start_address, rt_uint8_t end_address)
{
    ssd1351_write_cmd(0x15);              //   Set Column Address
    ssd1351_write_data(start_address);    //   Default => 0x00 (Start Address)
    ssd1351_write_data(end_address);      //   Default => 0x7F (End Address)
}
void set_row_address(rt_uint8_t start_address, rt_uint8_t end_address)
{
    ssd1351_write_cmd(0x75);              //   Set Row Address
    ssd1351_write_data(start_address);    //   Default => 0x00 (Start Address)
    ssd1351_write_data(end_address);      //   Default => 0x7F (End Address)
}

void set_write_ram(void)
{
    ssd1351_write_cmd(0x5C);            // Enable MCU to Write into RAM
}
void set_read_ram(void)
{
    ssd1351_write_cmd(0x5D);            // Enable MCU to Read from RAM
}
void set_remap_format(rt_uint8_t re_map_color_depth)
{
    ssd1351_write_cmd(0xA0);            // Set Re-Map / Color Depth
    ssd1351_write_data(re_map_color_depth);        // Default => 0x40
    //     Horizontal Address Increment
    //     Column Address 0 Mapped to SEG0
    //     Color Sequence: A => B => C
    //     Scan from COM0 to COM[N-1]
    //     Disable COM Split Odd Even
    //     65,536 Colors
}
void set_start_line(rt_uint8_t start_line)
{
    ssd1351_write_cmd(0xA1);               // Set Vertical Scroll by RAM
    ssd1351_write_data(start_line);        //   Default => 0x00
}
void set_display_on_off(rt_uint8_t onoff)
{
    ssd1351_write_cmd(0xAE | onoff);    // Set Display On/Off
    //     Default => 0xAE
    //     0xAE (0x00) => Display Off (Sleep Mode On)
    //     0xAF (0x01) => Display On (Sleep Mode Off)
}
void set_master_current(rt_uint8_t current)
{
    ssd1351_write_cmd(0xC7);            //   Master Contrast Current Control
    ssd1351_write_data(current);        //   Default => 0x0F (Maximum)
}

void set_display_offset(rt_uint8_t offset)
{
    ssd1351_write_cmd(0xA2);            // Set Vertical Scroll by Row
    ssd1351_write_data(offset);         //   Default => 0x60
}

void set_display_mode(rt_uint8_t mode)
{
    ssd1351_write_cmd(0xA4 | mode);        // Set Display Mode
    //     Default => 0xA6
    //     0xA4 (0x00) => Entire Display Off, All Pixels Turn Off
    //     0xA5 (0x01) => Entire Display On, All Pixels Turn On at GS Level 63
    //     0xA6 (0x02) => Normal Display
    //     0xA7 (0x03) => Inverse Display
}
void set_function_selection(rt_uint8_t fun)
{
    ssd1351_write_cmd(0xAB);            // Function Selection
    ssd1351_write_data(fun);            //   Default => 0x01
    //     Enable Internal VDD Regulator
    //     Select 8-bit Parallel Interface
}

void set_phaselength(rt_uint8_t length)
{
    ssd1351_write_cmd(0xB1);            // Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment
    ssd1351_write_data(length);         // Default => 0x82 (8 Display Clocks [Phase 2] / 5 Display Clocks [Phase 1])
    //     D[3:0] => Phase 1 Period in 5~31 Display Clocks
    //     D[7:4] => Phase 2 Period in 3~15 Display Clocks
}


void set_display_enhancement(rt_uint8_t enhancement)
{
    ssd1351_write_cmd(0xB2);            //   Display Enhancement
    ssd1351_write_data(enhancement);    //   Default => 0x00 (Normal)
    ssd1351_write_data(0x00);
    ssd1351_write_data(0x00);
}


void set_display_clock(rt_uint8_t clk)
{
    ssd1351_write_cmd(0xB3);            // Set Display Clock Divider / Oscillator Frequency
    ssd1351_write_data(clk);            //   Default => 0x00
    //     A[3:0] => Display Clock Divider
    //     A[7:4] => Oscillator Frequency
}


void set_vsl(rt_uint8_t vsl)
{
    ssd1351_write_cmd(0xB4);            // Set Segment Low Voltage
    ssd1351_write_data(0xA0 | vsl);     //   Default => 0xA0
    //     0xA0 (0x00) => Enable External VSL
    //     0xA2 (0x02) => Enable Internal VSL (Kept VSL Pin N.C.)
    ssd1351_write_data(0xB5);
    ssd1351_write_data(0x55);
}


void set_gpio(rt_uint8_t d)
{
    ssd1351_write_cmd(0xB5);            //   General Purpose IO
    ssd1351_write_data(d);              //   Default => 0x0A (GPIO Pins output Low Level.)
}


void set_precharge_period(rt_uint8_t period)
{
    ssd1351_write_cmd(0xB6);            // Set Second Pre-Charge Period
    ssd1351_write_data(period);         // Default => 0x08 (8 Display Clocks)
}


void set_precharge_voltage(rt_uint8_t v)
{
    ssd1351_write_cmd(0xBB);            //   Set Pre-Charge Voltage Level
    ssd1351_write_data(v);              //   Default => 0x17 (0.50*VCC)
}


void set_vcomh(rt_uint8_t v)
{
    ssd1351_write_cmd(0xBE);            //   Set COM Deselect Voltage Level
    ssd1351_write_data(v);              //   Default => 0x05 (0.82*VCC)
}


void set_contrast_color(rt_uint8_t a, rt_uint8_t b, rt_uint8_t c)
{
    ssd1351_write_cmd(0xC1);              //   Set Contrast Current for Color A, B, C
    ssd1351_write_data(a);                //   Default => 0x8A (Color A)
    ssd1351_write_data(b);                //   Default => 0x51 (Color B)
    ssd1351_write_data(c);                //   Default => 0x8A (Color C)
}

void set_multiplex_ratio(rt_uint8_t ratio)
{
    ssd1351_write_cmd(0xCA);              //   Set Multiplex Ratio
    ssd1351_write_data(ratio);            //   Default => 0x7F (1/128 Duty)
}

void set_command_lock(rt_uint8_t clk)
{
    ssd1351_write_cmd(0xFD);            // Set Command Lock
    ssd1351_write_data(clk);            // Default => 0x12
    //     0x12 => Driver IC interface is unlocked from entering command.
    //     0x16 => All Commands are locked except 0xFD.
    //     0xB0 => Command 0xA2, 0xB1, 0xB3, 0xBB & 0xBE are inaccessible.
    //     0xB1 => All Commands are accessible.
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//
//    a: RRRRRGGG
//    b: GGGBBBBB
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void ssd1351_fill_ram(rt_uint16_t rgb565_color)
{
    rt_uint8_t i, j;

    rt_uint16_t a = rgb565_color >> 8;
    rt_uint16_t b = rgb565_color;

    set_column_address(0, MAX_COLUMN);
    set_row_address(0, MAX_ROW);
    set_write_ram();

    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 128; j++)
        {
            ssd1351_write_data(a);
            ssd1351_write_data(b);
        }
    }
}
void ssd1351_clear(void)
{
    ssd1351_fill_ram(0x0000);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Partial or Full Screen)
//
//    col_star: Column Address of Start
//    col_end: Column Address of End
//    row_star: Row Address of Start
//    row_end: Row Address of End
//    rgb565_color: RGB565 format color,RRRRRGGGGGGBBBBB
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void ssd1351_fill_block(rt_uint8_t col_star, rt_uint8_t col_end, rt_uint8_t row_star, rt_uint8_t row_end, rt_uint16_t rgb565_color)
{
    rt_uint8_t i, j;

    set_column_address(col_star, col_end);
    set_row_address(row_star, row_end);
    set_write_ram();

    rt_uint8_t e = rgb565_color >> 8;
    rt_uint8_t f = rgb565_color;

    for (i = 0; i < (row_end - row_star + 1); i++)
    {
        for (j = 0; j < (col_end - col_star + 1); j++)
        {
            ssd1351_write_data(e);
            ssd1351_write_data(f);
        }
    }
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Patterns
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
unsigned char Format[][192] =
{
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0X21, 0X24, 0X10, 0X82, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XC7, 0X84, 0X30, 0X10, 0X82, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X42, 0X08, 0XEF, 0X5D, 0XF7, 0XBE, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XE7, 0XF7, 0XBE, 0XF7, 0XBE, 0XE7, 0X1C, 0XDE, 0XFB, 0XDE, 0XFB, 0XDE, 0XFB, 0XF7, 0X9E, 0XEF, 0X7D, 0X18, 0XE3, 0X00, 0X00, 0X00, 0X00, 0X18, 0XC3, 0XAD, 0X75, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XC6, 0X18, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0XAD, 0X75, 0XCE, 0X59, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X38, 0XFF, 0XFF, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X73, 0X8E, 0XFF, 0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XC7, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF7, 0X9E, 0X18, 0XE3, 0X00, 0X00, 0X10, 0XA2, 0XE7, 0X1C, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XEF, 0X7D, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0XFF, 0XFF, 0XFF, 0XFF, 0X73, 0X8E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XBD, 0XD7, 0XFF, 0XFF, 0X9C, 0XD3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X63, 0X0C, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X04, 0XFF, 0XFF, 0XFF, 0XFF, 0X8C, 0X51, 0X5A, 0XEB, 0X5A, 0XEB, 0X5A, 0XEB, 0X52, 0X8A, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0XAD, 0X55, 0XFF, 0XFF, 0XF7, 0X9E, 0X5A, 0XEB, 0X18, 0XC3, 0X4A, 0X69, 0XEF, 0X7D, 0XFF, 0XFF, 0XB5, 0X96, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X94, 0X92, 0XAD, 0X75, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XAD, 0X75, 0XFF, 0XFF, 0X9C, 0XF3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X4A, 0X49, 0XF7, 0XBE, 0XD6, 0X9A, 0XFF, 0XFF, 0XFF, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X31, 0X86, 0XFF, 0XFF, 0XFF, 0XFF, 0X4A, 0X49, 0X63, 0X2C, 0X7B, 0XEF, 0X6B, 0X6D, 0X18, 0XE3, 0X00, 0X00, 0X00, 0X00, 0X21, 0X04, 0XFF, 0XFF, 0XFF, 0XFF, 0X5A, 0XEB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X4D, 0XFF, 0XFF, 0XFF, 0XFF, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XB5, 0XB6, 0XFF, 0XFF, 0X94, 0X92, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XFF, 0XFF, 0XF7, 0XBE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X49, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF7, 0XBE, 0X4A, 0X69, 0X00, 0X00, 0X6B, 0X6D, 0XFF, 0XFF, 0XE7, 0X3C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X18, 0XE3, 0XFF, 0XFF, 0XFF, 0XFF, 0X52, 0XAA, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XBD, 0XF7, 0X7B, 0XEF, 0X00, 0X00, 0X00, 0X00, 0XD6, 0XBA, 0XEF, 0X7D, 0X21, 0X04, 0X94, 0X92, 0XE7, 0X3C, 0XDE, 0XDB, 0X5A, 0XEB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X7B, 0XCF, 0XC6, 0X38, 0XBD, 0XD7, 0X73, 0X8E, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0XBD, 0XF7, 0XFF, 0XFF, 0X84, 0X10, 0X52, 0X8A, 0XB5, 0X96, 0XCE, 0X59, 0X9C, 0XD3, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X63, 0X0C, 0XA5, 0X14, 0XA5, 0X34, 0X7B, 0XEF, 0X21, 0X24, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0XA5, 0X14, 0XD6, 0XBA, 0XEF, 0X5D, 0XEF, 0X7D, 0XD6, 0X9A, 0X21, 0X04,
    0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XFF, 0XFF, 0XF7, 0X9E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X0C, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XEF, 0X7D, 0XC6, 0X18, 0XEF, 0X5D, 0XFF, 0XFF, 0XEF, 0X7D, 0X10, 0X82, 0X9C, 0XD3, 0XFF, 0XFF, 0XB5, 0X96, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XFF, 0XFF, 0XFF, 0X73, 0X8E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XCB, 0XFF, 0XFF, 0XEF, 0X7D, 0X00, 0X00, 0X00, 0X20, 0XFF, 0XFF, 0XFF, 0XFF, 0XEF, 0X7D, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0XB5, 0X96, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XE7, 0X1C, 0X10, 0XA2, 0X00, 0X00, 0XC6, 0X18, 0XFF, 0XFF, 0XE7, 0X1C, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XAD, 0X75, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0XCE, 0X59, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0X42, 0X28, 0X00, 0X00, 0X21, 0X24, 0XF7, 0XBE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X5A, 0XCB,
    0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XFF, 0XFF, 0XEF, 0X7D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XCF, 0XFF, 0XFF, 0XFF, 0XFF, 0XA5, 0X34, 0X08, 0X41, 0X00, 0X00, 0X10, 0XA2, 0XF7, 0X9E, 0XFF, 0XFF, 0X63, 0X2C, 0XA5, 0X34, 0XFF, 0XFF, 0XA5, 0X14, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF7, 0XBE, 0XFF, 0XFF, 0X73, 0X8E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X0C, 0XFF, 0XFF, 0XEF, 0X5D, 0X00, 0X00, 0X08, 0X41, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0X8C, 0X71, 0XEF, 0X5D, 0XFF, 0XFF, 0X63, 0X2C, 0X00, 0X00, 0X73, 0X8E, 0XFF, 0XFF, 0XFF, 0XFF, 0X8C, 0X71, 0X84, 0X10, 0XE7, 0X3C, 0XFF, 0XFF, 0X31, 0X86, 0X00, 0X00, 0XC6, 0X18, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XD6, 0X9A, 0XB5, 0X96, 0XFF, 0XFF, 0XF7, 0X9E, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X34, 0XFF, 0XFF, 0XF7, 0X9E, 0X84, 0X30, 0X7B, 0XCF, 0XE7, 0X1C, 0XFF, 0XFF, 0XAD, 0X75, 0X00, 0X00, 0X73, 0X8E, 0XFF, 0XFF, 0XEF, 0X5D, 0X42, 0X28, 0X29, 0X65, 0X63, 0X0C, 0XCE, 0X79, 0X21, 0X04,
    0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XFF, 0XFF, 0XEF, 0X7D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0X8A, 0XFF, 0XFF, 0XBD, 0XD7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XB5, 0X96, 0XFF, 0XFF, 0X9C, 0XD3, 0X9C, 0XF3, 0XFF, 0XFF, 0XAD, 0X55, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XFF, 0XFF, 0XFF, 0X63, 0X2C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X6D, 0XFF, 0XFF, 0XDE, 0XFB, 0X00, 0X00, 0X08, 0X41, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X6B, 0X4D, 0X00, 0X00, 0XC6, 0X38, 0XFF, 0XFF, 0X7B, 0XEF, 0X08, 0X41, 0XEF, 0X7D, 0XFF, 0XFF, 0X84, 0X30, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0X29, 0X65, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X38, 0XFF, 0XFF, 0XFF, 0XFF, 0XB5, 0X96, 0X00, 0X00, 0X18, 0XE3, 0XFF, 0XFF, 0XFF, 0XFF, 0X08, 0X41, 0X18, 0XE3, 0XFF, 0XFF, 0XFF, 0XFF, 0X52, 0X8A, 0X21, 0X04, 0XA5, 0X14, 0XFF, 0XFF, 0XFF, 0XFF, 0X94, 0XB2, 0X00, 0X00, 0X5A, 0XCB, 0XFF, 0XFF, 0XFF, 0XFF, 0X9C, 0XF3, 0X18, 0XE3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XFF, 0XFF, 0XEF, 0X7D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X34, 0XFF, 0XFF, 0XA5, 0X34, 0X84, 0X30, 0XFF, 0XFF, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X04, 0XFF, 0XFF, 0XFF, 0XFF, 0X4A, 0X49, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XCF, 0XFF, 0XFF, 0XD6, 0X9A, 0X00, 0X00, 0X10, 0XA2, 0XFF, 0XFF, 0XFF, 0XFF, 0XD6, 0XBA, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X18, 0XFF, 0XFF, 0X84, 0X30, 0X52, 0X8A, 0XFF, 0XFF, 0XEF, 0X7D, 0X08, 0X41, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X38, 0XFF, 0XFF, 0XEF, 0X7D, 0X10, 0X82, 0X00, 0X00, 0X21, 0X04, 0XFF, 0XFF, 0XFF, 0XFF, 0X10, 0X82, 0X63, 0X0C, 0XFF, 0XFF, 0XF7, 0X9E, 0XA5, 0X34, 0XFF, 0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0XB5, 0XB6, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0XB5, 0XB6, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0X8C, 0X71, 0X08, 0X41, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XFF, 0XFF, 0XEF, 0X5D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XCE, 0X79, 0XFF, 0XFF, 0X94, 0X92, 0X52, 0XAA, 0XFF, 0XFF, 0XF7, 0X9E, 0X08, 0X41, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X0C, 0XFF, 0XFF, 0XFF, 0XFF, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XEF, 0XFF, 0XFF, 0XCE, 0X79, 0X00, 0X00, 0X18, 0XE3, 0XFF, 0XFF, 0XFF, 0XFF, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X18, 0XFF, 0XFF, 0X84, 0X30, 0X7B, 0XEF, 0XFF, 0XFF, 0XAD, 0X75, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X18, 0XFF, 0XFF, 0XA5, 0X14, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0XFF, 0XFF, 0XFF, 0XFF, 0X21, 0X04, 0X7B, 0XCF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XBD, 0XF7, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XDE, 0XDB, 0XFF, 0XFF, 0XFF, 0XFF, 0XCE, 0X79, 0X08, 0X41,
    0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XFF, 0XFF, 0XEF, 0X5D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0X94, 0XB2, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X49, 0XFF, 0XFF, 0XFF, 0XFF, 0X52, 0X8A, 0X10, 0X82, 0XFF, 0XDF, 0XFF, 0XFF, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XD6, 0XBA, 0XFF, 0XFF, 0XCE, 0X79, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XEF, 0XFF, 0XFF, 0XCE, 0X79, 0X00, 0X00, 0X21, 0X24, 0XFF, 0XFF, 0XFF, 0XFF, 0X52, 0X8A, 0X00, 0X00, 0X00, 0X00, 0XBD, 0XD7, 0XFF, 0XFF, 0X94, 0XB2, 0X73, 0XAE, 0XFF, 0XFF, 0XC6, 0X18, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X38, 0XFF, 0XFF, 0X9C, 0XD3, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XFF, 0XFF, 0XFF, 0XFF, 0X39, 0XC7, 0X6B, 0X6D, 0XFF, 0XFF, 0XFF, 0XFF, 0XAD, 0X75, 0X31, 0XA6, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0XFF, 0XFF, 0XFF, 0XFF, 0X6B, 0X4D,
    0X21, 0X24, 0XAD, 0X75, 0XDE, 0XFB, 0XFF, 0XFF, 0XFF, 0XDF, 0XBD, 0XF7, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X9C, 0XD3, 0XFF, 0XFF, 0XEF, 0X5D, 0X5A, 0XCB, 0X39, 0XE7, 0X7B, 0XCF, 0XF7, 0XBE, 0XFF, 0XFF, 0XDE, 0XDB, 0X00, 0X20, 0X00, 0X00, 0X94, 0XB2, 0XFF, 0XFF, 0XFF, 0XFF, 0XAD, 0X75, 0X8C, 0X51, 0XD6, 0XBA, 0XFF, 0XFF, 0XFF, 0XFF, 0X5A, 0XCB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XEF, 0XFF, 0XFF, 0XCE, 0X79, 0X00, 0X00, 0X21, 0X24, 0XFF, 0XFF, 0XFF, 0XFF, 0X42, 0X08, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X34, 0XFF, 0XFF, 0XB5, 0X96, 0X31, 0XA6, 0XFF, 0XFF, 0XFF, 0XFF, 0X94, 0XB2, 0X42, 0X08, 0X52, 0XAA, 0XBD, 0XF7, 0XFF, 0XDF, 0X39, 0XC7, 0X00, 0X00, 0XCE, 0X79, 0XFF, 0XFF, 0X94, 0X92, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XDF, 0XFF, 0XFF, 0X52, 0XAA, 0X21, 0X04, 0XFF, 0XDF, 0XFF, 0XFF, 0XA5, 0X34, 0X4A, 0X49, 0X39, 0XE7, 0X6B, 0X6D, 0XE7, 0X3C, 0XFF, 0XFF, 0X10, 0XA2, 0X8C, 0X51, 0XB5, 0X96, 0X63, 0X2C, 0X39, 0XE7, 0X63, 0X0C, 0XFF, 0XDF, 0XFF, 0XFF, 0X73, 0XAE,
    0X73, 0XAE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XA5, 0X14, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XC7, 0XDE, 0XFB, 0XC6, 0X18, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0X31, 0X86, 0XF7, 0XBE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XEF, 0X5D, 0X29, 0X65, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XC6, 0X38, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X8C, 0X71, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XCF, 0XFF, 0XFF, 0XCE, 0X59, 0X00, 0X00, 0X10, 0XA2, 0XFF, 0XFF, 0XFF, 0XFF, 0X42, 0X08, 0X00, 0X00, 0X00, 0X00, 0X8C, 0X51, 0XFF, 0XFF, 0XC6, 0X38, 0X00, 0X00, 0X94, 0XB2, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0X39, 0XC7, 0X00, 0X00, 0XE7, 0X3C, 0XFF, 0XFF, 0X73, 0XAE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XDE, 0XDB, 0XFF, 0XFF, 0X84, 0X10, 0X00, 0X00, 0X73, 0X8E, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XCE, 0X79, 0X00, 0X00, 0XCE, 0X79, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF7, 0X9E, 0X21, 0X04,
    0X08, 0X41, 0X63, 0X0C, 0X7B, 0XEF, 0X7B, 0XEF, 0X7B, 0XEF, 0X73, 0XAE, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X84, 0X10, 0XFF, 0XFF, 0XFF, 0XFF, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0XB5, 0X96, 0XF7, 0XBE, 0XFF, 0XFF, 0XEF, 0X5D, 0X9C, 0XD3, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X63, 0X2C, 0XAD, 0X75, 0XBD, 0XF7, 0X9C, 0XF3, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X04, 0XB5, 0X96, 0X52, 0XAA, 0X00, 0X00, 0X00, 0X00, 0XCE, 0X79, 0XEF, 0X5D, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X49, 0XFF, 0XDF, 0X8C, 0X71, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0XD6, 0X9A, 0XFF, 0XFF, 0XF7, 0X9E, 0XA5, 0X14, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0XB5, 0X96, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X84, 0X10, 0XFF, 0XDF, 0X52, 0X8A, 0X00, 0X00, 0X00, 0X00, 0X39, 0XE7, 0XA5, 0X34, 0XDE, 0XDB, 0XDE, 0XFB, 0XBD, 0XF7, 0X73, 0X8E, 0X08, 0X41, 0X00, 0X00, 0X21, 0X24, 0XAD, 0X75, 0XF7, 0X9E, 0XFF, 0XFF, 0XF7, 0X9E, 0XAD, 0X75, 0X29, 0X45, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0X63, 0X0C, 0X42, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XA5, 0X34, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0X8A, 0X7B, 0XEF, 0X7B, 0XEF, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X29, 0X45, 0X7B, 0XEF, 0X7B, 0XEF, 0X18, 0XE3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X34, 0X7B, 0XEF, 0X10, 0X00, 0X78, 0X00, 0X78, 0X00, 0X60, 0X00, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0X60, 0X05, 0X60, 0X05, 0XE0, 0X03, 0X80, 0X00, 0X40, 0X00, 0X00, 0X00, 0X00, 0X00, 0X11, 0X00, 0X17, 0X00, 0X17, 0X00, 0X0E, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0XE7, 0X3C, 0X39, 0XC7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X8C, 0X71, 0X31, 0X86, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X31, 0X86, 0X7B, 0XEF, 0X7B, 0XEF, 0X39, 0XC7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0X73, 0X8E, 0X7B, 0XEF, 0X39, 0XE7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0XDE, 0XFB, 0XFF, 0XFF, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X34, 0XE7, 0X3C, 0X84, 0X30, 0XBD, 0XD7, 0XEF, 0X7D, 0X10, 0XA2, 0X00, 0X00, 0X42, 0X28, 0XEF, 0X5D, 0X84, 0X30, 0X9C, 0XD3, 0XEF, 0X7D, 0X10, 0X82, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XEF, 0XD6, 0XBA, 0X10, 0X82, 0X40, 0X00, 0XF8, 0X00, 0XA8, 0X00, 0XD8, 0X00, 0XE8, 0X00, 0X40, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0X40, 0X07, 0X80, 0X05, 0X40, 0X03, 0XE0, 0X06, 0XC0, 0X03, 0XE0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1D, 0X00, 0X13, 0X00, 0X0F, 0X00, 0X1C, 0X00, 0X15, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X4D, 0XEF, 0X7D, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XBD, 0XD7, 0XFF, 0XFF, 0X4A, 0X69, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X4D, 0XF7, 0XBE, 0X94, 0XB2, 0X9C, 0XD3, 0XFF, 0XFF, 0X42, 0X28, 0X00, 0X00, 0X18, 0XC3, 0XE7, 0X3C, 0X94, 0XB2, 0X84, 0X30, 0XF7, 0X9E, 0X42, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0X8C, 0X71, 0XFF, 0XFF, 0X08, 0X61, 0X00, 0X00, 0X08, 0X41, 0X7B, 0XEF, 0X10, 0X82, 0X00, 0X00, 0X00, 0X00, 0XF7, 0X9E, 0X4A, 0X49, 0X00, 0X00, 0XBD, 0XF7, 0X63, 0X2C, 0X00, 0X00, 0X00, 0X00, 0XD6, 0X9A, 0X4A, 0X69, 0X00, 0X00, 0X08, 0X61, 0XF7, 0X9E, 0X31, 0XA6, 0X00, 0X00, 0X30, 0X00, 0XF8, 0X00, 0X00, 0X00, 0X00, 0X00, 0X68, 0X00, 0XF8, 0X00, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X06, 0X80, 0X05, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1D, 0X00, 0X07, 0X00, 0X00, 0X00, 0X02, 0X00, 0X1F, 0X00, 0X03, 0X00, 0X00, 0X00, 0X00, 0XB5, 0X96, 0X8C, 0X71, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0XA5, 0X14, 0XDE, 0XFB, 0X4A, 0X69, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X4D, 0X29, 0X65, 0X00, 0X00, 0X00, 0X00, 0XB5, 0X96, 0X8C, 0X51, 0X00, 0X00, 0X7B, 0XEF, 0XA5, 0X34, 0X00, 0X00, 0X00, 0X00, 0X94, 0X92, 0X8C, 0X71, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X18, 0XC3, 0XFF, 0XFF, 0X08, 0X41, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X42, 0X08, 0XFF, 0XFF, 0X18, 0XE3, 0X00, 0X00, 0XB5, 0XB6, 0X94, 0XB2, 0X00, 0X00, 0X29, 0X65, 0XEF, 0X7D, 0X39, 0XC7, 0X00, 0X00, 0X63, 0X0C, 0XD6, 0XBA, 0X00, 0X00, 0X00, 0X00, 0X30, 0X00, 0XF8, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XB0, 0X00, 0X88, 0X00, 0X00, 0X00, 0X02, 0XC0, 0X07, 0X40, 0X00, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1F, 0X00, 0X04, 0X00, 0X00, 0X00, 0X02, 0X00, 0X1F, 0X00, 0X03, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X49, 0XEF, 0X7D, 0X00, 0X00, 0X52, 0XAA, 0XC6, 0X38, 0X08, 0X41, 0X00, 0X00, 0X08, 0X61, 0XDE, 0XDB, 0X4A, 0X49, 0X00, 0X00, 0X00, 0X00, 0XDE, 0XDB, 0X4A, 0X49, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XEF, 0X5D, 0X5A, 0XCB, 0X00, 0X00, 0X73, 0XAE, 0XC6, 0X38, 0X08, 0X61, 0X10, 0XA2, 0XCE, 0X59, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0XFF, 0XDF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X0C, 0XF7, 0XBE, 0X7B, 0XEF, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0XFF, 0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0X9C, 0XF3, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X14, 0X94, 0XB2, 0X00, 0X00, 0X00, 0X00, 0X40, 0X00, 0XE8, 0X00, 0X00, 0X00, 0X00, 0X00, 0X18, 0X00, 0XD8, 0X00, 0X80, 0X00, 0X00, 0X00, 0X05, 0XC0, 0X04, 0X00, 0X00, 0X00, 0X00, 0X60, 0X01, 0XE0, 0X02, 0X80, 0X03, 0XE0, 0X00, 0XA0, 0X00, 0X01, 0X00, 0X1F, 0X00, 0X09, 0X00, 0X09, 0X00, 0X19, 0X00, 0X15, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XFF, 0XFF, 0X29, 0X65, 0X00, 0X20, 0XBD, 0XF7, 0XA5, 0X14, 0X10, 0XA2, 0XCE, 0X79, 0XAD, 0X55, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XE7, 0X3C, 0X39, 0XC7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XC7, 0XDE, 0XFB, 0XB5, 0XB6, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0XDE, 0XDB, 0XFF, 0XFF, 0XFF, 0XFF, 0XD6, 0XBA, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X31, 0XA6, 0XEF, 0X5D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XAD, 0X75, 0XE7, 0X3C, 0X4A, 0X69, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X94, 0X92, 0XDE, 0XDB, 0X52, 0X8A, 0X73, 0X8E, 0XF7, 0XBE, 0X5A, 0XCB, 0X00, 0X00, 0XC6, 0X18, 0X73, 0XAE, 0X00, 0X00, 0X00, 0X00, 0X50, 0X00, 0XF0, 0X00, 0XB8, 0X00, 0XB8, 0X00, 0XF8, 0X00, 0XC0, 0X00, 0X08, 0X00, 0X00, 0X00, 0X07, 0XA0, 0X02, 0X00, 0X04, 0X20, 0X07, 0XE0, 0X07, 0XE0, 0X06, 0X80, 0X07, 0XE0, 0X02, 0X80, 0X00, 0X02, 0X00, 0X1F, 0X00, 0X1F, 0X00, 0X1F, 0X00, 0X1F, 0X00, 0X10, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XE7, 0X3C, 0X52, 0X8A, 0X00, 0X00, 0X10, 0XA2, 0XEF, 0X5D, 0XEF, 0X5D, 0XA5, 0X14, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF7, 0XBE, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XCF, 0XF7, 0XBE, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0X8A, 0XF7, 0X9E, 0X6B, 0X4D, 0X5A, 0XCB, 0XE7, 0X1C, 0X9C, 0XD3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XE7, 0XE7, 0X1C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XAD, 0X55, 0XBD, 0XF7, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0XFF, 0XDF, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X84, 0X10, 0XBD, 0XF7, 0X00, 0X00, 0XC6, 0X18, 0X73, 0X8E, 0X00, 0X00, 0X00, 0X00, 0X58, 0X00, 0XF0, 0X00, 0XD8, 0X00, 0XF8, 0X00, 0XA8, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X40, 0X07, 0XE0, 0X01, 0X40, 0X02, 0X80, 0X01, 0XE0, 0X00, 0X00, 0X03, 0X00, 0X07, 0X60, 0X00, 0X00, 0X00, 0X04, 0X00, 0X1F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0A, 0X00, 0X1A, 0X00, 0X11, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XE7, 0X1C, 0X52, 0X8A, 0X00, 0X00, 0X10, 0XA2, 0XDE, 0XDB, 0XFF, 0XFF, 0X31, 0X86, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XFF, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X4D, 0XE7, 0X3C, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XBD, 0XF7, 0X6B, 0X4D, 0X00, 0X00, 0X00, 0X00, 0X42, 0X08, 0XFF, 0XDF, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X31, 0X86, 0XEF, 0X5D, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0XFF, 0XFF, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X7B, 0XCF, 0XC6, 0X18, 0X00, 0X00, 0XB5, 0X96, 0X84, 0X30, 0X00, 0X00, 0X00, 0X00, 0X50, 0X00, 0XD0, 0X00, 0X00, 0X00, 0X50, 0X00, 0XE0, 0X00, 0XC8, 0X00, 0X18, 0X00, 0X00, 0X00, 0X07, 0XA0, 0X02, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0XC0, 0X07, 0X00, 0X03, 0XE0, 0X00, 0X00, 0X00, 0X04, 0X00, 0X1E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0X00, 0X16, 0X00, 0X14, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF7, 0XBE, 0X39, 0XE7, 0X10, 0XA2, 0XD6, 0X9A, 0XA5, 0X14, 0X84, 0X10, 0XDE, 0XFB, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF7, 0X9E, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0XCE, 0X59, 0X63, 0X0C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XCE, 0X79, 0X52, 0XAA, 0X00, 0X00, 0X00, 0X00, 0X39, 0XC7, 0XFF, 0XFF, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0XAA, 0XCE, 0X59, 0XFF, 0XFF, 0XBD, 0XF7, 0X10, 0XA2, 0X18, 0XC3, 0XFF, 0XFF, 0XBD, 0XF7, 0XE7, 0X3C, 0XFF, 0XFF, 0XE7, 0X1C, 0X39, 0XC7, 0X00, 0X00, 0XBD, 0XF7, 0XC6, 0X38, 0X7B, 0XEF, 0X84, 0X10, 0XEF, 0X7D, 0X63, 0X0C, 0X00, 0X00, 0X84, 0X10, 0XB5, 0XB6, 0X00, 0X00, 0X00, 0X00, 0X48, 0X00, 0XD0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X00, 0XB0, 0X00, 0XD8, 0X00, 0X00, 0X00, 0X04, 0XE0, 0X07, 0X00, 0X03, 0XE0, 0X04, 0X40, 0X07, 0X40, 0X05, 0XA0, 0X00, 0X20, 0X00, 0X00, 0X00, 0X04, 0X00, 0X1F, 0X00, 0X11, 0X00, 0X17, 0X00, 0X1E, 0X00, 0X19, 0X00, 0X02, 0X00, 0X00, 0X00, 0X00, 0X29, 0X45, 0XFF, 0XFF, 0X10, 0X82, 0XD6, 0XBA, 0X9C, 0XF3, 0X00, 0X00, 0X00, 0X00, 0XB5, 0XB6, 0XAD, 0X75, 0X00, 0X00, 0X21, 0X24, 0XBD, 0XF7, 0XFF, 0XDF, 0XCE, 0X79, 0X42, 0X28, 0X00, 0X00, 0XDE, 0XDB, 0XCE, 0X79, 0XD6, 0XBA, 0XFF, 0XFF, 0XF7, 0X9E, 0X6B, 0X4D, 0X00, 0X00, 0X7B, 0XEF, 0XE7, 0X3C, 0X7B, 0XEF, 0X7B, 0XEF, 0XCE, 0X79, 0XA5, 0X14, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X18, 0XE3, 0X39, 0XE7, 0X63, 0X0C, 0X63, 0X2C, 0X00, 0X20, 0X00, 0X00, 0X39, 0XE7, 0X39, 0XE7, 0X39, 0XE7, 0X39, 0XE7, 0X39, 0XE7, 0X10, 0X82, 0X00, 0X00, 0X08, 0X41, 0X6B, 0X6D, 0XAD, 0X75, 0X94, 0X92, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X31, 0X86, 0XFF, 0XDF, 0X18, 0XE3, 0X00, 0X00, 0X18, 0X00, 0X78, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X60, 0X00, 0X00, 0X00, 0X00, 0X20, 0X03, 0XC0, 0X05, 0XE0, 0X05, 0X80, 0X03, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X14, 0X00, 0X13, 0X00, 0X0F, 0X00, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X8C, 0X71, 0XC6, 0X18, 0X00, 0X00, 0X31, 0XA6, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X61, 0X8C, 0X71, 0X00, 0X00, 0X08, 0X61, 0X39, 0XE7, 0X52, 0X8A, 0X73, 0XAE, 0X10, 0XA2, 0X00, 0X00, 0X29, 0X65, 0X39, 0XE7, 0X39, 0XE7, 0X39, 0XE7, 0X39, 0XE7, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XCB, 0X9C, 0XF3, 0XA5, 0X14, 0X63, 0X0C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XAD, 0X55, 0XC6, 0X18, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0XAA, 0XFF, 0XDF, 0X39, 0XC7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XA5, 0X14, 0X73, 0XAE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XD6, 0XBA, 0X4A, 0X49, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0X63, 0X0C, 0X7B, 0XEF, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XCB, 0X39, 0XE7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0X73, 0XAE, 0X73, 0XAE, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X73, 0XAE, 0X21, 0X24, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X73, 0XAE, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X42, 0X08, 0XA5, 0X34, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0X82, 0X39, 0XC7, 0X4A, 0X69, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XDE, 0XFB, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0XDE, 0XDB, 0XCE, 0X79, 0XA5, 0X34, 0XF7, 0X9E, 0X84, 0X10, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0XEF, 0X7D, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0XF7, 0XBE, 0XAD, 0X75, 0XBD, 0XF7, 0XF7, 0XBE, 0X29, 0X65, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0XAD, 0X55, 0XFF, 0XFF, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0X8A, 0XFF, 0XFF, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XEF, 0X5D, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X6D, 0XF7, 0XBE, 0XFF, 0XFF, 0XFF, 0XFF, 0X6B, 0X4D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF7, 0X9E, 0X29, 0X65, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0XAA, 0X73, 0XAE, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0XDE, 0XDB, 0X00, 0X00, 0X00, 0X00, 0X42, 0X28, 0XEF, 0X5D, 0X31, 0X86, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X94, 0XB2, 0X39, 0XC7, 0X00, 0X00, 0X00, 0X00, 0XC6, 0X38, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XC7, 0XB5, 0XB6, 0XE7, 0X3C, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XEF, 0X5D, 0XA5, 0X14, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XAD, 0X75, 0XA5, 0X14, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0XF7, 0XBE, 0X5A, 0XCB, 0X00, 0X00, 0X94, 0XB2, 0X5A, 0XCB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF7, 0XBE, 0X29, 0X45, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X18, 0XE3, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X14, 0XB5, 0XB6, 0X00, 0X00, 0X00, 0X00, 0XDE, 0XDB, 0X4A, 0X69, 0X18, 0XE3, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XEF, 0X7D, 0X52, 0XAA, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XDE, 0XFB, 0X42, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XAD, 0X75, 0XAD, 0X55, 0X52, 0XAA, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XEF, 0X5D, 0X18, 0XC3, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0XF7, 0XBE, 0X5A, 0XEB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X39, 0XC7, 0X94, 0X92, 0X84, 0X10, 0X08, 0X61, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XDF, 0X21, 0X04, 0X00, 0X00, 0X00, 0X20, 0X6B, 0X6D, 0X94, 0XB2, 0X52, 0X8A, 0X00, 0X00, 0X00, 0X00, 0X63, 0X2C, 0X21, 0X04, 0X5A, 0XEB, 0XAD, 0X55, 0X63, 0X0C, 0X00, 0X00, 0X08, 0X61, 0X7B, 0XCF, 0XCE, 0X59, 0XFF, 0XFF, 0X63, 0X0C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0X9C, 0XD3, 0XF7, 0X9E, 0X31, 0X86, 0X00, 0X00, 0X42, 0X08, 0XFF, 0XFF, 0XEF, 0X5D, 0XFF, 0XFF, 0XF7, 0XBE, 0X63, 0X2C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0XD6, 0XBA, 0XBD, 0XF7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XEF, 0X7D, 0X31, 0X86, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XCB, 0XEF, 0X7D, 0X18, 0XC3, 0X52, 0XAA, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X18, 0XC3, 0XEF, 0X7D, 0X52, 0XAA, 0X00, 0X20, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XA5, 0X14, 0XBD, 0XD7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X69, 0XFF, 0XDF, 0XAD, 0X55, 0XD6, 0X9A, 0XB5, 0XB6, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XFF, 0X21, 0X04, 0X00, 0X00, 0XBD, 0XD7, 0XDE, 0XDB, 0XA5, 0X34, 0XF7, 0XBE, 0X42, 0X28, 0X00, 0X00, 0XBD, 0XF7, 0XE7, 0X3C, 0XC6, 0X38, 0X9C, 0XF3, 0XD6, 0X9A, 0X00, 0X00, 0XC6, 0X38, 0XD6, 0X9A, 0X63, 0X0C, 0X84, 0X30, 0X73, 0XAE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0XDE, 0XDB, 0XC6, 0X18, 0X21, 0X24, 0X00, 0X00, 0X00, 0X00, 0X73, 0XAE, 0XD6, 0X9A, 0X31, 0XA6, 0X10, 0X82, 0X63, 0X0C, 0XFF, 0XDF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X73, 0X8E, 0XF7, 0XBE, 0X84, 0X30, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XFF, 0X21, 0X04, 0X00, 0X00, 0X21, 0X24, 0XF7, 0X9E, 0X94, 0XB2, 0X39, 0XE7, 0X73, 0XAE, 0XD6, 0X9A, 0X18, 0XE3, 0X00, 0X20, 0XC6, 0X18, 0XD6, 0X9A, 0X39, 0XE7, 0X39, 0XC7, 0XFF, 0XFF, 0X39, 0XE7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF7, 0X9E, 0X4A, 0X69, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XD6, 0XBA, 0X6B, 0X6D, 0X00, 0X00, 0X18, 0XC3, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0XFF, 0XFF, 0X18, 0XC3, 0X42, 0X28, 0XEF, 0X7D, 0X10, 0X82, 0X00, 0X00, 0X84, 0X30, 0XA5, 0X14, 0X00, 0X00, 0XBD, 0XF7, 0XB5, 0XB6, 0X00, 0X00, 0X31, 0X86, 0XA5, 0X34, 0X00, 0X00, 0XFF, 0XDF, 0X84, 0X30, 0X29, 0X65, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X18, 0XC3, 0XF7, 0X9E, 0X7B, 0XCF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X84, 0X10, 0X9C, 0XD3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XDE, 0XFB, 0X42, 0X08, 0X00, 0X00, 0X6B, 0X6D, 0XE7, 0X3C, 0X31, 0XA6, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0XFF, 0XFF, 0X18, 0XC3, 0X00, 0X00, 0X5A, 0XCB, 0XCE, 0X79, 0XDE, 0XDB, 0XE7, 0X1C, 0XF7, 0XBE, 0XFF, 0XDF, 0XE7, 0X1C, 0X29, 0X45, 0XC6, 0X38, 0XD6, 0XBA, 0XDE, 0XFB, 0XEF, 0X7D, 0XFF, 0XFF, 0XF7, 0X9E, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0X82, 0XFF, 0XFF, 0X21, 0X24, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0XFF, 0XFF, 0X18, 0XC3, 0X00, 0X00, 0X00, 0X00, 0XF7, 0X9E, 0X29, 0X45, 0X08, 0X41, 0XFF, 0XFF, 0X10, 0XA2, 0X7B, 0XCF, 0XAD, 0X55, 0X00, 0X00, 0X00, 0X00, 0X63, 0X0C, 0XBD, 0XD7, 0X00, 0X00, 0XBD, 0XF7, 0X5A, 0XEB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XCB, 0XC6, 0X18, 0XF7, 0XBE, 0XB5, 0X96, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X73, 0XAE, 0XBD, 0XD7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X4D, 0XBD, 0XF7, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XEF, 0X7D, 0X29, 0X65, 0X00, 0X00, 0XD6, 0XBA, 0X5A, 0XCB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XDF, 0X21, 0X04, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0XAA, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XEF, 0X7D, 0X63, 0X0C, 0X00, 0X00, 0X00, 0X00, 0X29, 0X45, 0XAD, 0X75, 0X18, 0XC3, 0XF7, 0XBE, 0X21, 0X24, 0X00, 0X00, 0X10, 0XA2, 0XFF, 0XDF, 0X10, 0X82, 0X10, 0X82, 0XFF, 0XFF, 0X08, 0X61, 0X6B, 0X4D, 0XB5, 0XB6, 0X00, 0X00, 0X00, 0X00, 0X84, 0X10, 0X9C, 0XF3, 0X00, 0X00, 0XBD, 0XF7, 0X5A, 0XEB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0XE7, 0X1C, 0X4A, 0X49, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X8C, 0X51, 0XDE, 0XFB, 0XBD, 0XD7, 0XCE, 0X79, 0XCE, 0X79, 0X9C, 0XD3, 0X00, 0X00, 0X18, 0XE3, 0XF7, 0XBE, 0X63, 0X0C, 0X21, 0X04, 0X94, 0X92, 0XCE, 0X79, 0X00, 0X00, 0X00, 0X00, 0XEF, 0X5D, 0XC6, 0X18, 0XC6, 0X38, 0XCE, 0X79, 0XCE, 0X59, 0X52, 0X8A, 0X00, 0X00, 0X00, 0X00, 0X29, 0X65, 0X10, 0X82, 0X21, 0X24, 0XAD, 0X55, 0XFF, 0XDF, 0XBD, 0XF7, 0X31, 0X86, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X52, 0XAA, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X20, 0XFF, 0XFF, 0X10, 0XA2, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X6B, 0X4D, 0XFF, 0XDF, 0X9C, 0XF3, 0XAD, 0X55, 0XFF, 0XDF, 0XA5, 0X14, 0X00, 0X20, 0XA5, 0X14, 0XCE, 0X59, 0X63, 0X2C, 0XCE, 0X59, 0XAD, 0X55, 0X00, 0X00, 0X18, 0XE3, 0XFF, 0XFF, 0X00, 0X20, 0X18, 0XE3, 0XF7, 0X9E, 0X84, 0X30, 0X7B, 0XCF, 0XF7, 0XBE, 0X39, 0XC7, 0X00, 0X00, 0XBD, 0XF7, 0X5A, 0XEB, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X5A, 0XEB, 0XBD, 0XD7, 0X63, 0X2C, 0X73, 0X8E, 0XEF, 0X7D, 0X42, 0X28, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0X6B, 0X6D, 0X63, 0X0C, 0X52, 0X8A, 0X52, 0X8A, 0X5A, 0XCB, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X49, 0XCE, 0X79, 0XEF, 0X5D, 0XAD, 0X75, 0X18, 0XE3, 0X00, 0X00, 0X00, 0X00, 0X52, 0X8A, 0X6B, 0X4D, 0X5A, 0XCB, 0X4A, 0X69, 0X5A, 0XCB, 0X31, 0X86, 0X00, 0X00, 0X00, 0X00, 0XDE, 0XDB, 0X31, 0XA6, 0X18, 0XC3, 0X6B, 0X6D, 0X73, 0XAE, 0X7B, 0XEF, 0X18, 0XE3, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X24, 0X73, 0X8E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X8C, 0X71, 0X00, 0X20, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X69, 0X9C, 0XD3, 0X8C, 0X71, 0X39, 0XE7, 0X00, 0X00, 0X00, 0X00, 0X08, 0X41, 0X94, 0XB2, 0XCE, 0X79, 0X9C, 0XD3, 0X08, 0X61, 0X00, 0X00, 0X08, 0X61, 0XA5, 0X34, 0X00, 0X00, 0X00, 0X00, 0X39, 0XE7, 0XBD, 0XF7, 0XC6, 0X38, 0X52, 0XAA, 0X00, 0X00, 0X00, 0X00, 0X8C, 0X71, 0X42, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X10, 0XA2, 0X9C, 0XD3, 0XCE, 0X79, 0XC6, 0X38, 0X6B, 0X6D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X4A, 0X49, 0XC6, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X21, 0X04, 0X29, 0X65, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
};


unsigned char  Ascii_1[240][5] =    //   Refer to "Times New Roman" Font Database...
{
    //   Basic Characters
    {0x00, 0x00, 0x4F, 0x00, 0x00},    //   (  1)  ! - 0x0021 Exclamation Mark
    {0x00, 0x07, 0x00, 0x07, 0x00},    //   (  2)  " - 0x0022 Quotation Mark
    {0x14, 0x7F, 0x14, 0x7F, 0x14},    //   (  3)  # - 0x0023 Number Sign
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},    //   (  4)  $ - 0x0024 Dollar Sign
    {0x23, 0x13, 0x08, 0x64, 0x62},    //   (  5)  % - 0x0025 Percent Sign
    {0x36, 0x49, 0x55, 0x22, 0x50},    //   (  6)  & - 0x0026 Ampersand
    {0x00, 0x05, 0x03, 0x00, 0x00},    //   (  7)  ' - 0x0027 Apostrophe
    {0x00, 0x1C, 0x22, 0x41, 0x00},    //   (  8)  ( - 0x0028 Left Parenthesis
    {0x00, 0x41, 0x22, 0x1C, 0x00},    //   (  9)  ) - 0x0029 Right Parenthesis
    {0x14, 0x08, 0x3E, 0x08, 0x14},    //   ( 10)  * - 0x002A Asterisk
    {0x08, 0x08, 0x3E, 0x08, 0x08},    //   ( 11)  + - 0x002B Plus Sign
    {0x00, 0x50, 0x30, 0x00, 0x00},    //   ( 12)  , - 0x002C Comma
    {0x08, 0x08, 0x08, 0x08, 0x08},    //   ( 13)  - - 0x002D Hyphen-Minus
    {0x00, 0x60, 0x60, 0x00, 0x00},    //   ( 14)  . - 0x002E Full Stop
    {0x20, 0x10, 0x08, 0x04, 0x02},    //   ( 15)  / - 0x002F Solidus
    {0x3E, 0x51, 0x49, 0x45, 0x3E},    //   ( 16)  0 - 0x0030 Digit Zero
    {0x00, 0x42, 0x7F, 0x40, 0x00},    //   ( 17)  1 - 0x0031 Digit One
    {0x42, 0x61, 0x51, 0x49, 0x46},    //   ( 18)  2 - 0x0032 Digit Two
    {0x21, 0x41, 0x45, 0x4B, 0x31},    //   ( 19)  3 - 0x0033 Digit Three
    {0x18, 0x14, 0x12, 0x7F, 0x10},    //   ( 20)  4 - 0x0034 Digit Four
    {0x27, 0x45, 0x45, 0x45, 0x39},    //   ( 21)  5 - 0x0035 Digit Five
    {0x3C, 0x4A, 0x49, 0x49, 0x30},    //   ( 22)  6 - 0x0036 Digit Six
    {0x01, 0x71, 0x09, 0x05, 0x03},    //   ( 23)  7 - 0x0037 Digit Seven
    {0x36, 0x49, 0x49, 0x49, 0x36},    //   ( 24)  8 - 0x0038 Digit Eight
    {0x06, 0x49, 0x49, 0x29, 0x1E},    //   ( 25)  9 - 0x0039 Dight Nine
    {0x00, 0x36, 0x36, 0x00, 0x00},    //   ( 26)  : - 0x003A Colon
    {0x00, 0x56, 0x36, 0x00, 0x00},    //   ( 27)  ; - 0x003B Semicolon
    {0x08, 0x14, 0x22, 0x41, 0x00},    //   ( 28)  < - 0x003C Less-Than Sign
    {0x14, 0x14, 0x14, 0x14, 0x14},    //   ( 29)  = - 0x003D Equals Sign
    {0x00, 0x41, 0x22, 0x14, 0x08},    //   ( 30)  > - 0x003E Greater-Than Sign
    {0x02, 0x01, 0x51, 0x09, 0x06},    //   ( 31)  ? - 0x003F Question Mark
    {0x32, 0x49, 0x79, 0x41, 0x3E},    //   ( 32)  @ - 0x0040 Commercial At
    {0x7E, 0x11, 0x11, 0x11, 0x7E},    //   ( 33)  A - 0x0041 Latin Capital Letter A
    {0x7F, 0x49, 0x49, 0x49, 0x36},    //   ( 34)  B - 0x0042 Latin Capital Letter B
    {0x3E, 0x41, 0x41, 0x41, 0x22},    //   ( 35)  C - 0x0043 Latin Capital Letter C
    {0x7F, 0x41, 0x41, 0x22, 0x1C},    //   ( 36)  D - 0x0044 Latin Capital Letter D
    {0x7F, 0x49, 0x49, 0x49, 0x41},    //   ( 37)  E - 0x0045 Latin Capital Letter E
    {0x7F, 0x09, 0x09, 0x09, 0x01},    //   ( 38)  F - 0x0046 Latin Capital Letter F
    {0x3E, 0x41, 0x49, 0x49, 0x7A},    //   ( 39)  G - 0x0047 Latin Capital Letter G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},    //   ( 40)  H - 0x0048 Latin Capital Letter H
    {0x00, 0x41, 0x7F, 0x41, 0x00},    //   ( 41)  I - 0x0049 Latin Capital Letter I
    {0x20, 0x40, 0x41, 0x3F, 0x01},    //   ( 42)  J - 0x004A Latin Capital Letter J
    {0x7F, 0x08, 0x14, 0x22, 0x41},    //   ( 43)  K - 0x004B Latin Capital Letter K
    {0x7F, 0x40, 0x40, 0x40, 0x40},    //   ( 44)  L - 0x004C Latin Capital Letter L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},    //   ( 45)  M - 0x004D Latin Capital Letter M
    {0x7F, 0x04, 0x08, 0x10, 0x7F},    //   ( 46)  N - 0x004E Latin Capital Letter N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},    //   ( 47)  O - 0x004F Latin Capital Letter O
    {0x7F, 0x09, 0x09, 0x09, 0x06},    //   ( 48)  P - 0x0050 Latin Capital Letter P
    {0x3E, 0x41, 0x51, 0x21, 0x5E},    //   ( 49)  Q - 0x0051 Latin Capital Letter Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},    //   ( 50)  R - 0x0052 Latin Capital Letter R
    {0x46, 0x49, 0x49, 0x49, 0x31},    //   ( 51)  S - 0x0053 Latin Capital Letter S
    {0x01, 0x01, 0x7F, 0x01, 0x01},    //   ( 52)  T - 0x0054 Latin Capital Letter T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},    //   ( 53)  U - 0x0055 Latin Capital Letter U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},    //   ( 54)  V - 0x0056 Latin Capital Letter V
    {0x3F, 0x40, 0x38, 0x40, 0x3F},    //   ( 55)  W - 0x0057 Latin Capital Letter W
    {0x63, 0x14, 0x08, 0x14, 0x63},    //   ( 56)  X - 0x0058 Latin Capital Letter X
    {0x07, 0x08, 0x70, 0x08, 0x07},    //   ( 57)  Y - 0x0059 Latin Capital Letter Y
    {0x61, 0x51, 0x49, 0x45, 0x43},    //   ( 58)  Z - 0x005A Latin Capital Letter Z
    {0x00, 0x7F, 0x41, 0x41, 0x00},    //   ( 59)  [ - 0x005B Left Square Bracket
    {0x02, 0x04, 0x08, 0x10, 0x20},    //   ( 60)  \ - 0x005C Reverse Solidus
    {0x00, 0x41, 0x41, 0x7F, 0x00},    //   ( 61)  ] - 0x005D Right Square Bracket
    {0x04, 0x02, 0x01, 0x02, 0x04},    //   ( 62)  ^ - 0x005E Circumflex Accent
    {0x40, 0x40, 0x40, 0x40, 0x40},    //   ( 63)  _ - 0x005F Low Line
    {0x01, 0x02, 0x04, 0x00, 0x00},    //   ( 64)  ` - 0x0060 Grave Accent
    {0x20, 0x54, 0x54, 0x54, 0x78},    //   ( 65)  a - 0x0061 Latin Small Letter A
    {0x7F, 0x48, 0x44, 0x44, 0x38},    //   ( 66)  b - 0x0062 Latin Small Letter B
    {0x38, 0x44, 0x44, 0x44, 0x20},    //   ( 67)  c - 0x0063 Latin Small Letter C
    {0x38, 0x44, 0x44, 0x48, 0x7F},    //   ( 68)  d - 0x0064 Latin Small Letter D
    {0x38, 0x54, 0x54, 0x54, 0x18},    //   ( 69)  e - 0x0065 Latin Small Letter E
    {0x08, 0x7E, 0x09, 0x01, 0x02},    //   ( 70)  f - 0x0066 Latin Small Letter F
    {0x06, 0x49, 0x49, 0x49, 0x3F},    //   ( 71)  g - 0x0067 Latin Small Letter G
    {0x7F, 0x08, 0x04, 0x04, 0x78},    //   ( 72)  h - 0x0068 Latin Small Letter H
    {0x00, 0x44, 0x7D, 0x40, 0x00},    //   ( 73)  i - 0x0069 Latin Small Letter I
    {0x20, 0x40, 0x44, 0x3D, 0x00},    //   ( 74)  j - 0x006A Latin Small Letter J
    {0x7F, 0x10, 0x28, 0x44, 0x00},    //   ( 75)  k - 0x006B Latin Small Letter K
    {0x00, 0x41, 0x7F, 0x40, 0x00},    //   ( 76)  l - 0x006C Latin Small Letter L
    {0x7C, 0x04, 0x18, 0x04, 0x7C},    //   ( 77)  m - 0x006D Latin Small Letter M
    {0x7C, 0x08, 0x04, 0x04, 0x78},    //   ( 78)  n - 0x006E Latin Small Letter N
    {0x38, 0x44, 0x44, 0x44, 0x38},    //   ( 79)  o - 0x006F Latin Small Letter O
    {0x7C, 0x14, 0x14, 0x14, 0x08},    //   ( 80)  p - 0x0070 Latin Small Letter P
    {0x08, 0x14, 0x14, 0x18, 0x7C},    //   ( 81)  q - 0x0071 Latin Small Letter Q
    {0x7C, 0x08, 0x04, 0x04, 0x08},    //   ( 82)  r - 0x0072 Latin Small Letter R
    {0x48, 0x54, 0x54, 0x54, 0x20},    //   ( 83)  s - 0x0073 Latin Small Letter S
    {0x04, 0x3F, 0x44, 0x40, 0x20},    //   ( 84)  t - 0x0074 Latin Small Letter T
    {0x3C, 0x40, 0x40, 0x20, 0x7C},    //   ( 85)  u - 0x0075 Latin Small Letter U
    {0x1C, 0x20, 0x40, 0x20, 0x1C},    //   ( 86)  v - 0x0076 Latin Small Letter V
    {0x3C, 0x40, 0x30, 0x40, 0x3C},    //   ( 87)  w - 0x0077 Latin Small Letter W
    {0x44, 0x28, 0x10, 0x28, 0x44},    //   ( 88)  x - 0x0078 Latin Small Letter X
    {0x0C, 0x50, 0x50, 0x50, 0x3C},    //   ( 89)  y - 0x0079 Latin Small Letter Y
    {0x44, 0x64, 0x54, 0x4C, 0x44},    //   ( 90)  z - 0x007A Latin Small Letter Z
    {0x00, 0x08, 0x36, 0x41, 0x00},    //   ( 91)  { - 0x007B Left Curly Bracket
    {0x00, 0x00, 0x7F, 0x00, 0x00},    //   ( 92)  | - 0x007C Vertical Line
    {0x00, 0x41, 0x36, 0x08, 0x00},    //   ( 93)  } - 0x007D Right Curly Bracket
    {0x02, 0x01, 0x02, 0x04, 0x02},    //   ( 94)  ~ - 0x007E Tilde
    {0x3E, 0x55, 0x55, 0x41, 0x22},    //   ( 95)  C - 0x0080 <Control>
    {0x00, 0x00, 0x00, 0x00, 0x00},    //   ( 96)    - 0x00A0 No-Break Space
    {0x00, 0x00, 0x79, 0x00, 0x00},    //   ( 97)  ! - 0x00A1 Inverted Exclamation Mark
    {0x18, 0x24, 0x74, 0x2E, 0x24},    //   ( 98)  c - 0x00A2 Cent Sign
    {0x48, 0x7E, 0x49, 0x42, 0x40},    //   ( 99)  L - 0x00A3 Pound Sign
    {0x5D, 0x22, 0x22, 0x22, 0x5D},    //   (100)  o - 0x00A4 Currency Sign
    {0x15, 0x16, 0x7C, 0x16, 0x15},    //   (101)  Y - 0x00A5 Yen Sign
    {0x00, 0x00, 0x77, 0x00, 0x00},    //   (102)  | - 0x00A6 Broken Bar
    {0x0A, 0x55, 0x55, 0x55, 0x28},    //   (103)    - 0x00A7 Section Sign
    {0x00, 0x01, 0x00, 0x01, 0x00},    //   (104)  " - 0x00A8 Diaeresis
    {0x00, 0x0A, 0x0D, 0x0A, 0x04},    //   (105)    - 0x00AA Feminine Ordinal Indicator
    {0x08, 0x14, 0x2A, 0x14, 0x22},    //   (106) << - 0x00AB Left-Pointing Double Angle Quotation Mark
    {0x04, 0x04, 0x04, 0x04, 0x1C},    //   (107)    - 0x00AC Not Sign
    {0x00, 0x08, 0x08, 0x08, 0x00},    //   (108)  - - 0x00AD Soft Hyphen
    {0x01, 0x01, 0x01, 0x01, 0x01},    //   (109)    - 0x00AF Macron
    {0x00, 0x02, 0x05, 0x02, 0x00},    //   (110)    - 0x00B0 Degree Sign
    {0x44, 0x44, 0x5F, 0x44, 0x44},    //   (111) +- - 0x00B1 Plus-Minus Sign
    {0x00, 0x00, 0x04, 0x02, 0x01},    //   (112)  ` - 0x00B4 Acute Accent
    {0x7E, 0x20, 0x20, 0x10, 0x3E},    //   (113)  u - 0x00B5 Micro Sign
    {0x06, 0x0F, 0x7F, 0x00, 0x7F},    //   (114)    - 0x00B6 Pilcrow Sign
    {0x00, 0x18, 0x18, 0x00, 0x00},    //   (115)  . - 0x00B7 Middle Dot
    {0x00, 0x40, 0x50, 0x20, 0x00},    //   (116)    - 0x00B8 Cedilla
    {0x00, 0x0A, 0x0D, 0x0A, 0x00},    //   (117)    - 0x00BA Masculine Ordinal Indicator
    {0x22, 0x14, 0x2A, 0x14, 0x08},    //   (118) >> - 0x00BB Right-Pointing Double Angle Quotation Mark
    {0x17, 0x08, 0x34, 0x2A, 0x7D},    //   (119) /4 - 0x00BC Vulgar Fraction One Quarter
    {0x17, 0x08, 0x04, 0x6A, 0x59},    //   (120) /2 - 0x00BD Vulgar Fraction One Half
    {0x30, 0x48, 0x45, 0x40, 0x20},    //   (121)  ? - 0x00BF Inverted Question Mark
    {0x70, 0x29, 0x26, 0x28, 0x70},    //   (122) `A - 0x00C0 Latin Capital Letter A with Grave
    {0x70, 0x28, 0x26, 0x29, 0x70},    //   (123) 'A - 0x00C1 Latin Capital Letter A with Acute
    {0x70, 0x2A, 0x25, 0x2A, 0x70},    //   (124) ^A - 0x00C2 Latin Capital Letter A with Circumflex
    {0x72, 0x29, 0x26, 0x29, 0x70},    //   (125) ~A - 0x00C3 Latin Capital Letter A with Tilde
    {0x70, 0x29, 0x24, 0x29, 0x70},    //   (126) "A - 0x00C4 Latin Capital Letter A with Diaeresis
    {0x70, 0x2A, 0x2D, 0x2A, 0x70},    //   (127)  A - 0x00C5 Latin Capital Letter A with Ring Above
    {0x7E, 0x11, 0x7F, 0x49, 0x49},    //   (128) AE - 0x00C6 Latin Capital Letter Ae
    {0x0E, 0x51, 0x51, 0x71, 0x11},    //   (129)  C - 0x00C7 Latin Capital Letter C with Cedilla
    {0x7C, 0x55, 0x56, 0x54, 0x44},    //   (130) `E - 0x00C8 Latin Capital Letter E with Grave
    {0x7C, 0x55, 0x56, 0x54, 0x44},    //   (131) 'E - 0x00C9 Latin Capital Letter E with Acute
    {0x7C, 0x56, 0x55, 0x56, 0x44},    //   (132) ^E - 0x00CA Latin Capital Letter E with Circumflex
    {0x7C, 0x55, 0x54, 0x55, 0x44},    //   (133) "E - 0x00CB Latin Capital Letter E with Diaeresis
    {0x00, 0x45, 0x7E, 0x44, 0x00},    //   (134) `I - 0x00CC Latin Capital Letter I with Grave
    {0x00, 0x44, 0x7E, 0x45, 0x00},    //   (135) 'I - 0x00CD Latin Capital Letter I with Acute
    {0x00, 0x46, 0x7D, 0x46, 0x00},    //   (136) ^I - 0x00CE Latin Capital Letter I with Circumflex
    {0x00, 0x45, 0x7C, 0x45, 0x00},    //   (137) "I - 0x00CF Latin Capital Letter I with Diaeresis
    {0x7F, 0x49, 0x49, 0x41, 0x3E},    //   (138)  D - 0x00D0 Latin Capital Letter Eth
    {0x7C, 0x0A, 0x11, 0x22, 0x7D},    //   (139) ~N - 0x00D1 Latin Capital Letter N with Tilde
    {0x38, 0x45, 0x46, 0x44, 0x38},    //   (140) `O - 0x00D2 Latin Capital Letter O with Grave
    {0x38, 0x44, 0x46, 0x45, 0x38},    //   (141) 'O - 0x00D3 Latin Capital Letter O with Acute
    {0x38, 0x46, 0x45, 0x46, 0x38},    //   (142) ^O - 0x00D4 Latin Capital Letter O with Circumflex
    {0x38, 0x46, 0x45, 0x46, 0x39},    //   (143) ~O - 0x00D5 Latin Capital Letter O with Tilde
    {0x38, 0x45, 0x44, 0x45, 0x38},    //   (144) "O - 0x00D6 Latin Capital Letter O with Diaeresis
    {0x22, 0x14, 0x08, 0x14, 0x22},    //   (145)  x - 0x00D7 Multiplcation Sign
    {0x2E, 0x51, 0x49, 0x45, 0x3A},    //   (146)  O - 0x00D8 Latin Capital Letter O with Stroke
    {0x3C, 0x41, 0x42, 0x40, 0x3C},    //   (147) `U - 0x00D9 Latin Capital Letter U with Grave
    {0x3C, 0x40, 0x42, 0x41, 0x3C},    //   (148) 'U - 0x00DA Latin Capital Letter U with Acute
    {0x3C, 0x42, 0x41, 0x42, 0x3C},    //   (149) ^U - 0x00DB Latin Capital Letter U with Circumflex
    {0x3C, 0x41, 0x40, 0x41, 0x3C},    //   (150) "U - 0x00DC Latin Capital Letter U with Diaeresis
    {0x0C, 0x10, 0x62, 0x11, 0x0C},    //   (151) `Y - 0x00DD Latin Capital Letter Y with Acute
    {0x7F, 0x12, 0x12, 0x12, 0x0C},    //   (152)  P - 0x00DE Latin Capital Letter Thom
    {0x40, 0x3E, 0x01, 0x49, 0x36},    //   (153)  B - 0x00DF Latin Capital Letter Sharp S
    {0x20, 0x55, 0x56, 0x54, 0x78},    //   (154) `a - 0x00E0 Latin Small Letter A with Grave
    {0x20, 0x54, 0x56, 0x55, 0x78},    //   (155) 'a - 0x00E1 Latin Small Letter A with Acute
    {0x20, 0x56, 0x55, 0x56, 0x78},    //   (156) ^a - 0x00E2 Latin Small Letter A with Circumflex
    {0x20, 0x55, 0x56, 0x55, 0x78},    //   (157) ~a - 0x00E3 Latin Small Letter A with Tilde
    {0x20, 0x55, 0x54, 0x55, 0x78},    //   (158) "a - 0x00E4 Latin Small Letter A with Diaeresis
    {0x20, 0x56, 0x57, 0x56, 0x78},    //   (159)  a - 0x00E5 Latin Small Letter A with Ring Above
    {0x24, 0x54, 0x78, 0x54, 0x58},    //   (160) ae - 0x00E6 Latin Small Letter Ae
    {0x0C, 0x52, 0x52, 0x72, 0x13},    //   (161)  c - 0x00E7 Latin Small Letter c with Cedilla
    {0x38, 0x55, 0x56, 0x54, 0x18},    //   (162) `e - 0x00E8 Latin Small Letter E with Grave
    {0x38, 0x54, 0x56, 0x55, 0x18},    //   (163) 'e - 0x00E9 Latin Small Letter E with Acute
    {0x38, 0x56, 0x55, 0x56, 0x18},    //   (164) ^e - 0x00EA Latin Small Letter E with Circumflex
    {0x38, 0x55, 0x54, 0x55, 0x18},    //   (165) "e - 0x00EB Latin Small Letter E with Diaeresis
    {0x00, 0x49, 0x7A, 0x40, 0x00},    //   (166) `i - 0x00EC Latin Small Letter I with Grave
    {0x00, 0x48, 0x7A, 0x41, 0x00},    //   (167) 'i - 0x00ED Latin Small Letter I with Acute
    {0x00, 0x4A, 0x79, 0x42, 0x00},    //   (168) ^i - 0x00EE Latin Small Letter I with Circumflex
    {0x00, 0x4A, 0x78, 0x42, 0x00},    //   (169) "i - 0x00EF Latin Small Letter I with Diaeresis
    {0x31, 0x4A, 0x4E, 0x4A, 0x30},    //   (170)    - 0x00F0 Latin Small Letter Eth
    {0x7A, 0x11, 0x0A, 0x09, 0x70},    //   (171) ~n - 0x00F1 Latin Small Letter N with Tilde
    {0x30, 0x49, 0x4A, 0x48, 0x30},    //   (172) `o - 0x00F2 Latin Small Letter O with Grave
    {0x30, 0x48, 0x4A, 0x49, 0x30},    //   (173) 'o - 0x00F3 Latin Small Letter O with Acute
    {0x30, 0x4A, 0x49, 0x4A, 0x30},    //   (174) ^o - 0x00F4 Latin Small Letter O with Circumflex
    {0x30, 0x4A, 0x49, 0x4A, 0x31},    //   (175) ~o - 0x00F5 Latin Small Letter O with Tilde
    {0x30, 0x4A, 0x48, 0x4A, 0x30},    //   (176) "o - 0x00F6 Latin Small Letter O with Diaeresis
    {0x08, 0x08, 0x2A, 0x08, 0x08},    //   (177)  + - 0x00F7 Division Sign
    {0x38, 0x64, 0x54, 0x4C, 0x38},    //   (178)  o - 0x00F8 Latin Small Letter O with Stroke
    {0x38, 0x41, 0x42, 0x20, 0x78},    //   (179) `u - 0x00F9 Latin Small Letter U with Grave
    {0x38, 0x40, 0x42, 0x21, 0x78},    //   (180) 'u - 0x00FA Latin Small Letter U with Acute
    {0x38, 0x42, 0x41, 0x22, 0x78},    //   (181) ^u - 0x00FB Latin Small Letter U with Circumflex
    {0x38, 0x42, 0x40, 0x22, 0x78},    //   (182) "u - 0x00FC Latin Small Letter U with Diaeresis
    {0x0C, 0x50, 0x52, 0x51, 0x3C},    //   (183) 'y - 0x00FD Latin Small Letter Y with Acute
    {0x7E, 0x14, 0x14, 0x14, 0x08},    //   (184)  p - 0x00FE Latin Small Letter Thom
    {0x0C, 0x51, 0x50, 0x51, 0x3C},    //   (185) "y - 0x00FF Latin Small Letter Y with Diaeresis
    {0x1E, 0x09, 0x09, 0x29, 0x5E},    //   (186)  A - 0x0104 Latin Capital Letter A with Ogonek
    {0x08, 0x15, 0x15, 0x35, 0x4E},    //   (187)  a - 0x0105 Latin Small Letter A with Ogonek
    {0x38, 0x44, 0x46, 0x45, 0x20},    //   (188) 'C - 0x0106 Latin Capital Letter C with Acute
    {0x30, 0x48, 0x4A, 0x49, 0x20},    //   (189) 'c - 0x0107 Latin Small Letter C with Acute
    {0x38, 0x45, 0x46, 0x45, 0x20},    //   (190)  C - 0x010C Latin Capital Letter C with Caron
    {0x30, 0x49, 0x4A, 0x49, 0x20},    //   (191)  c - 0x010D Latin Small Letter C with Caron
    {0x7C, 0x45, 0x46, 0x45, 0x38},    //   (192)  D - 0x010E Latin Capital Letter D with Caron
    {0x20, 0x50, 0x50, 0x7C, 0x03},    //   (193) d' - 0x010F Latin Small Letter D with Caron
    {0x1F, 0x15, 0x15, 0x35, 0x51},    //   (194)  E - 0x0118 Latin Capital Letter E with Ogonek
    {0x0E, 0x15, 0x15, 0x35, 0x46},    //   (195)  e - 0x0119 Latin Small Letter E with Ogonek
    {0x7C, 0x55, 0x56, 0x55, 0x44},    //   (196)  E - 0x011A Latin Capital Letter E with Caron
    {0x38, 0x55, 0x56, 0x55, 0x18},    //   (197)  e - 0x011B Latin Small Letter E with Caron
    {0x00, 0x44, 0x7C, 0x40, 0x00},    //   (198)  i - 0x0131 Latin Small Letter Dotless I
    {0x7F, 0x48, 0x44, 0x40, 0x40},    //   (199)  L - 0x0141 Latin Capital Letter L with Stroke
    {0x00, 0x49, 0x7F, 0x44, 0x00},    //   (200)  l - 0x0142 Latin Small Letter L with Stroke
    {0x7C, 0x08, 0x12, 0x21, 0x7C},    //   (201) 'N - 0x0143 Latin Capital Letter N with Acute
    {0x78, 0x10, 0x0A, 0x09, 0x70},    //   (202) 'n - 0x0144 Latin Small Letter N with Acute
    {0x7C, 0x09, 0x12, 0x21, 0x7C},    //   (203)  N - 0x0147 Latin Capital Letter N with Caron
    {0x78, 0x11, 0x0A, 0x09, 0x70},    //   (204)  n - 0x0148 Latin Small Letter N with Caron
    {0x38, 0x47, 0x44, 0x47, 0x38},    //   (205) "O - 0x0150 Latin Capital Letter O with Double Acute
    {0x30, 0x4B, 0x48, 0x4B, 0x30},    //   (206) "o - 0x0151 Latin Small Letter O with Double Acute
    {0x3E, 0x41, 0x7F, 0x49, 0x49},    //   (207) OE - 0x0152 Latin Capital Ligature Oe
    {0x38, 0x44, 0x38, 0x54, 0x58},    //   (208) oe - 0x0153 Latin Small Ligature Oe
    {0x7C, 0x15, 0x16, 0x35, 0x48},    //   (209)  R - 0x0158 Latin Capital Letter R with Caron
    {0x78, 0x11, 0x0A, 0x09, 0x10},    //   (210)  r - 0x0159 Latin Small Letter R with Caron
    {0x48, 0x54, 0x56, 0x55, 0x20},    //   (211) 'S - 0x015A Latin Capital Letter S with Acute
    {0x20, 0x48, 0x56, 0x55, 0x20},    //   (212) 's - 0x015B Latin Small Letter S with Acute
    {0x48, 0x55, 0x56, 0x55, 0x20},    //   (213)  S - 0x0160 Latin Capital Letter S with Caron
    {0x20, 0x49, 0x56, 0x55, 0x20},    //   (214)  s - 0x0161 Latin Small Letter S with Caron
    {0x04, 0x05, 0x7E, 0x05, 0x04},    //   (215)  T - 0x0164 Latin Capital Letter T with Caron
    {0x08, 0x3C, 0x48, 0x22, 0x01},    //   (216) t' - 0x0165 Latin Small Letter T with Caron
    {0x3C, 0x42, 0x45, 0x42, 0x3C},    //   (217)  U - 0x016E Latin Capital Letter U with Ring Above
    {0x38, 0x42, 0x45, 0x22, 0x78},    //   (218)  u - 0x016F Latin Small Letter U with Ring Above
    {0x3C, 0x43, 0x40, 0x43, 0x3C},    //   (219) "U - 0x0170 Latin Capital Letter U with Double Acute
    {0x38, 0x43, 0x40, 0x23, 0x78},    //   (220) "u - 0x0171 Latin Small Letter U with Double Acute
    {0x0C, 0x11, 0x60, 0x11, 0x0C},    //   (221) "Y - 0x0178 Latin Capital Letter Y with Diaeresis
    {0x44, 0x66, 0x55, 0x4C, 0x44},    //   (222) 'Z - 0x0179 Latin Capital Letter Z with Acute
    {0x48, 0x6A, 0x59, 0x48, 0x00},    //   (223) 'z - 0x017A Latin Small Letter Z with Acute
    {0x44, 0x64, 0x55, 0x4C, 0x44},    //   (224)  Z - 0x017B Latin Capital Letter Z with Dot Above
    {0x48, 0x68, 0x5A, 0x48, 0x00},    //   (225)  z - 0x017C Latin Small Letter Z with Dot Above
    {0x44, 0x65, 0x56, 0x4D, 0x44},    //   (226)  Z - 0x017D Latin Capital Letter Z with Caron
    {0x48, 0x69, 0x5A, 0x49, 0x00},    //   (227)  z - 0x017E Latin Small Letter Z with Caron
    {0x00, 0x02, 0x01, 0x02, 0x00},    //   (228)  ^ - 0x02C6 Modifier Letter Circumflex Accent
    {0x00, 0x01, 0x02, 0x01, 0x00},    //   (229)    - 0x02C7 Caron
    {0x00, 0x01, 0x01, 0x01, 0x00},    //   (230)    - 0x02C9 Modifier Letter Macron
    {0x01, 0x02, 0x02, 0x01, 0x00},    //   (231)    - 0x02D8 Breve
    {0x00, 0x00, 0x01, 0x00, 0x00},    //   (232)    - 0x02D9 Dot Above
    {0x00, 0x02, 0x05, 0x02, 0x00},    //   (233)    - 0x02DA Ring Above
    {0x02, 0x01, 0x02, 0x01, 0x00},    //   (234)  ~ - 0x02DC Small Tilde
    {0x7F, 0x05, 0x15, 0x3A, 0x50},    //   (235) Pt - 0x20A7 Peseta Sign
    {0x3E, 0x55, 0x55, 0x41, 0x22},    //   (236)  C - 0x20AC Euro Sign
    {0x18, 0x14, 0x08, 0x14, 0x0C},    //   (237)    - 0x221E Infinity
    {0x44, 0x4A, 0x4A, 0x51, 0x51},    //   (238)  < - 0x2264 Less-Than or Equal to
    {0x51, 0x51, 0x4A, 0x4A, 0x44},    //   (239)  > - 0x2265 Greater-Than or Equal to
    {0x74, 0x42, 0x41, 0x42, 0x74},    //   (240)    - 0x2302 House
};


unsigned char  Ascii_2[107][5] =    // Refer to "Times New Roman" Font Database...
{
    //   Greek & Japanese Letters
    {0x7E, 0x11, 0x11, 0x11, 0x7E},    //   (  1)  A - 0x0391 Greek Capital Letter Alpha
    {0x7F, 0x49, 0x49, 0x49, 0x36},    //   (  2)  B - 0x0392 Greek Capital Letter Beta
    {0x7F, 0x02, 0x01, 0x01, 0x03},    //   (  3)    - 0x0393 Greek Capital Letter Gamma
    {0x70, 0x4E, 0x41, 0x4E, 0x70},    //   (  4)    - 0x0394 Greek Capital Letter Delta
    {0x7F, 0x49, 0x49, 0x49, 0x41},    //   (  5)  E - 0x0395 Greek Capital Letter Epsilon
    {0x61, 0x51, 0x49, 0x45, 0x43},    //   (  6)  Z - 0x0396 Greek Capital Letter Zeta
    {0x7F, 0x08, 0x08, 0x08, 0x7F},    //   (  7)  H - 0x0397 Greek Capital Letter Eta
    {0x3E, 0x49, 0x49, 0x49, 0x3E},    //   (  8)    - 0x0398 Greek Capital Letter Theta
    {0x00, 0x41, 0x7F, 0x41, 0x00},    //   (  9)  I - 0x0399 Greek Capital Letter Iota
    {0x7F, 0x08, 0x14, 0x22, 0x41},    //   ( 10)  K - 0x039A Greek Capital Letter Kappa
    {0x70, 0x0E, 0x01, 0x0E, 0x70},    //   ( 11)    - 0x039B Greek Capital Letter Lamda
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},    //   ( 12)  M - 0x039C Greek Capital Letter Mu
    {0x7F, 0x04, 0x08, 0x10, 0x7F},    //   ( 13)  N - 0x039D Greek Capital Letter Nu
    {0x63, 0x5D, 0x49, 0x5D, 0x63},    //   ( 14)    - 0x039E Greek Capital Letter Xi
    {0x3E, 0x41, 0x41, 0x41, 0x3E},    //   ( 15)  O - 0x039F Greek Capital Letter Omicron
    {0x41, 0x3F, 0x01, 0x3F, 0x41},    //   ( 16)    - 0x03A0 Greek Capital Letter Pi
    {0x7F, 0x09, 0x09, 0x09, 0x06},    //   ( 17)  P - 0x03A1 Greek Capital Letter Rho
    {0x63, 0x55, 0x49, 0x41, 0x41},    //   ( 18)    - 0x03A3 Greek Capital Letter Sigma
    {0x01, 0x01, 0x7F, 0x01, 0x01},    //   ( 19)  T - 0x03A4 Greek Capital Letter Tau
    {0x03, 0x01, 0x7E, 0x01, 0x03},    //   ( 20)    - 0x03A5 Greek Capital Letter Upsilon
    {0x08, 0x55, 0x7F, 0x55, 0x08},    //   ( 21)    - 0x03A6 Greek Capital Letter Phi
    {0x63, 0x14, 0x08, 0x14, 0x63},    //   ( 22)  X - 0x03A7 Greek Capital Letter Chi
    {0x07, 0x48, 0x7F, 0x48, 0x07},    //   ( 23)    - 0x03A8 Greek Capital Letter Psi
    {0x5E, 0x61, 0x01, 0x61, 0x5E},    //   ( 24)    - 0x03A9 Greek Capital Letter Omega
    {0x38, 0x44, 0x48, 0x30, 0x4C},    //   ( 25)  a - 0x03B1 Greek Small Letter Alpha
    {0x7C, 0x2A, 0x2A, 0x2A, 0x14},    //   ( 26)  B - 0x03B2 Greek Small Letter Beta
    {0x44, 0x38, 0x04, 0x04, 0x08},    //   ( 27)  r - 0x03B3 Greek Small Letter Gamma
    {0x30, 0x4B, 0x4D, 0x59, 0x30},    //   ( 28)    - 0x03B4 Greek Small Letter Delta
    {0x28, 0x54, 0x54, 0x44, 0x20},    //   ( 29)    - 0x03B5 Greek Small Letter Epsilon
    {0x00, 0x18, 0x55, 0x52, 0x22},    //   ( 30)    - 0x03B6 Greek Small Letter Zeta
    {0x3E, 0x04, 0x02, 0x02, 0x7C},    //   ( 31)  n - 0x03B7 Greek Small Letter Eta
    {0x3C, 0x4A, 0x4A, 0x4A, 0x3C},    //   ( 32)    - 0x03B8 Greek Small Letter Theta
    {0x00, 0x3C, 0x40, 0x20, 0x00},    //   ( 33)  i - 0x03B9 Greek Small Letter Iota
    {0x7C, 0x10, 0x28, 0x44, 0x40},    //   ( 34)  k - 0x03BA Greek Small Letter Kappa
    {0x41, 0x32, 0x0C, 0x30, 0x40},    //   ( 35)    - 0x03BB Greek Small Letter Lamda
    {0x7E, 0x20, 0x20, 0x10, 0x3E},    //   ( 36)  u - 0x03BC Greek Small Letter Mu
    {0x1C, 0x20, 0x40, 0x20, 0x1C},    //   ( 37)  v - 0x03BD Greek Small Letter Nu
    {0x14, 0x2B, 0x2A, 0x2A, 0x60},    //   ( 38)    - 0x03BE Greek Small Letter Xi
    {0x38, 0x44, 0x44, 0x44, 0x38},    //   ( 39)  o - 0x03BF Greek Small Letter Omicron
    {0x44, 0x3C, 0x04, 0x7C, 0x44},    //   ( 40)    - 0x03C0 Greek Small Letter Pi
    {0x70, 0x28, 0x24, 0x24, 0x18},    //   ( 41)  p - 0x03C1 Greek Small Letter Rho
    {0x0C, 0x12, 0x12, 0x52, 0x60},    //   ( 42)    - 0x03C2 Greek Small Letter Final Sigma
    {0x38, 0x44, 0x4C, 0x54, 0x24},    //   ( 43)    - 0x03C3 Greek Small Letter Sigma
    {0x04, 0x3C, 0x44, 0x20, 0x00},    //   ( 44)  t - 0x03C4 Greek Small Letter Tau
    {0x3C, 0x40, 0x40, 0x20, 0x1C},    //   ( 45)  v - 0x03C5 Greek Small Letter Upsilon
    {0x18, 0x24, 0x7E, 0x24, 0x18},    //   ( 46)    - 0x03C6 Greek Small Letter Phi
    {0x44, 0x28, 0x10, 0x28, 0x44},    //   ( 47)  x - 0x03C7 Greek Small Letter Chi
    {0x0C, 0x10, 0x7E, 0x10, 0x0C},    //   ( 48)    - 0x03C8 Greek Small Letter Psi
    {0x38, 0x44, 0x30, 0x44, 0x38},    //   ( 49)  w - 0x03C9 Greek Small Letter Omega
    {0x0A, 0x0A, 0x4A, 0x2A, 0x1E},    //   ( 50)    - 0xFF66 Katakana Letter Wo
    {0x04, 0x44, 0x34, 0x14, 0x0C},    //   ( 51)    - 0xFF67 Katakana Letter Small A
    {0x20, 0x10, 0x78, 0x04, 0x00},    //   ( 52)    - 0xFF68 Katakana Letter Small I
    {0x18, 0x08, 0x4C, 0x48, 0x38},    //   ( 53)    - 0xFF69 Katakana Letter Small U
    {0x48, 0x48, 0x78, 0x48, 0x48},    //   ( 54)    - 0xFF6A Katakana Letter Small E
    {0x48, 0x28, 0x18, 0x7C, 0x08},    //   ( 55)    - 0xFF6B Katakana Letter Small O
    {0x08, 0x7C, 0x08, 0x28, 0x18},    //   ( 56)    - 0xFF6C Katakana Letter Small Ya
    {0x40, 0x48, 0x48, 0x78, 0x40},    //   ( 57)    - 0xFF6D Katakana Letter Small Yu
    {0x54, 0x54, 0x54, 0x7C, 0x00},    //   ( 58)    - 0xFF6E Katakana Letter Small Yo
    {0x18, 0x00, 0x58, 0x40, 0x38},    //   ( 59)    - 0xFF6F Katakana Letter Small Tu
    {0x08, 0x08, 0x08, 0x08, 0x08},    //   ( 60)    - 0xFF70 Katakana-Hiragana Prolonged Sound Mark
    {0x01, 0x41, 0x3D, 0x09, 0x07},    //   ( 61)    - 0xFF71 Katakana Letter A
    {0x10, 0x08, 0x7C, 0x02, 0x01},    //   ( 62)    - 0xFF72 Katakana Letter I
    {0x0E, 0x02, 0x43, 0x22, 0x1E},    //   ( 63)    - 0xFF73 Katakana Letter U
    {0x42, 0x42, 0x7E, 0x42, 0x42},    //   ( 64)    - 0xFF74 Katakana Letter E
    {0x22, 0x12, 0x0A, 0x7F, 0x02},    //   ( 65)    - 0xFF75 Katakana Letter O
    {0x42, 0x3F, 0x02, 0x42, 0x3E},    //   ( 66)    - 0xFF76 Katakana Letter Ka
    {0x0A, 0x0A, 0x7F, 0x0A, 0x0A},    //   ( 67)    - 0xFF77 Katakana Letter Ki
    {0x08, 0x46, 0x42, 0x22, 0x1E},    //   ( 68)    - 0xFF78 Katakana Letter Ku
    {0x04, 0x03, 0x42, 0x3E, 0x02},    //   ( 69)    - 0xFF79 Katakana Letter Ke
    {0x42, 0x42, 0x42, 0x42, 0x7E},    //   ( 70)    - 0xFF7A Katakana Letter Ko
    {0x02, 0x4F, 0x22, 0x1F, 0x02},    //   ( 71)    - 0xFF7B Katakana Letter Sa
    {0x4A, 0x4A, 0x40, 0x20, 0x1C},    //   ( 72)    - 0xFF7C Katakana Letter Shi
    {0x42, 0x22, 0x12, 0x2A, 0x46},    //   ( 73)    - 0xFF7D Katakana Letter Su
    {0x02, 0x3F, 0x42, 0x4A, 0x46},    //   ( 74)    - 0xFF7E Katakana Letter Se
    {0x06, 0x48, 0x40, 0x20, 0x1E},    //   ( 75)    - 0xFF7F Katakana Letter So
    {0x08, 0x46, 0x4A, 0x32, 0x1E},    //   ( 76)    - 0xFF80 Katakana Letter Ta
    {0x0A, 0x4A, 0x3E, 0x09, 0x08},    //   ( 77)    - 0xFF81 Katakana Letter Chi
    {0x0E, 0x00, 0x4E, 0x20, 0x1E},    //   ( 78)    - 0xFF82 Katakana Letter Tsu
    {0x04, 0x45, 0x3D, 0x05, 0x04},    //   ( 79)    - 0xFF83 Katakana Letter Te
    {0x00, 0x7F, 0x08, 0x10, 0x00},    //   ( 80)    - 0xFF84 Katakana Letter To
    {0x44, 0x24, 0x1F, 0x04, 0x04},    //   ( 81)    - 0xFF85 Katakana Letter Na
    {0x40, 0x42, 0x42, 0x42, 0x40},    //   ( 82)    - 0xFF86 Katakana Letter Ni
    {0x42, 0x2A, 0x12, 0x2A, 0x06},    //   ( 83)    - 0xFF87 Katakana Letter Nu
    {0x22, 0x12, 0x7B, 0x16, 0x22},    //   ( 84)    - 0xFF88 Katakana Letter Ne
    {0x00, 0x40, 0x20, 0x1F, 0x00},    //   ( 85)    - 0xFF89 Katakana Letter No
    {0x78, 0x00, 0x02, 0x04, 0x78},    //   ( 86)    - 0xFF8A Katakana Letter Ha
    {0x3F, 0x44, 0x44, 0x44, 0x44},    //   ( 87)    - 0xFF8B Katakana Letter Hi
    {0x02, 0x42, 0x42, 0x22, 0x1E},    //   ( 88)    - 0xFF8C Katakana Letter Fu
    {0x04, 0x02, 0x04, 0x08, 0x30},    //   ( 89)    - 0xFF8D Katakana Letter He
    {0x32, 0x02, 0x7F, 0x02, 0x32},    //   ( 90)    - 0xFF8E Katakana Letter Ho
    {0x02, 0x12, 0x22, 0x52, 0x0E},    //   ( 91)    - 0xFF8F Katakana Letter Ma
    {0x00, 0x2A, 0x2A, 0x2A, 0x40},    //   ( 92)    - 0xFF90 Katakana Letter Mi
    {0x38, 0x24, 0x22, 0x20, 0x70},    //   ( 93)    - 0xFF91 Katakana Letter Mu
    {0x40, 0x28, 0x10, 0x28, 0x06},    //   ( 94)    - 0xFF92 Katakana Letter Me
    {0x0A, 0x3E, 0x4A, 0x4A, 0x4A},    //   ( 95)    - 0xFF93 Katakana Letter Mo
    {0x04, 0x7F, 0x04, 0x14, 0x0C},    //   ( 96)    - 0xFF94 Katakana Letter Ya
    {0x40, 0x42, 0x42, 0x7E, 0x40},    //   ( 97)    - 0xFF95 Katakana Letter Yu
    {0x4A, 0x4A, 0x4A, 0x4A, 0x7E},    //   ( 98)    - 0xFF96 Katakana Letter Yo
    {0x04, 0x05, 0x45, 0x25, 0x1C},    //   ( 99)    - 0xFF97 Katakana Letter Ra
    {0x0F, 0x40, 0x20, 0x1F, 0x00},    //   (100)    - 0xFF98 Katakana Letter Ri
    {0x7C, 0x00, 0x7E, 0x40, 0x30},    //   (101)    - 0xFF99 Katakana Letter Ru
    {0x7E, 0x40, 0x20, 0x10, 0x08},    //   (102)    - 0xFF9A Katakana Letter Re
    {0x7E, 0x42, 0x42, 0x42, 0x7E},    //   (103)    - 0xFF9B Katakana Letter Ro
    {0x0E, 0x02, 0x42, 0x22, 0x1E},    //   (104)    - 0xFF9C Katakana Letter Wa
    {0x42, 0x42, 0x40, 0x20, 0x18},    //   (105)    - 0xFF9D Katakana Letter N
    {0x02, 0x04, 0x01, 0x02, 0x00},    //   (106)    - 0xFF9E Katakana Voiced Sound Mark
    {0x07, 0x05, 0x07, 0x00, 0x00},    //   (107)    - 0xFF9F Katakana Semi-Voiced Sound Mark
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Graphic Acceleration (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//    e: Line Width
//    rgb565_color: Line Color,RGB565 format
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void draw_rectangle(rt_uint8_t a, rt_uint8_t b, rt_uint8_t c, rt_uint8_t d, rt_uint8_t e, rt_uint16_t rgb565_color)
{
    rt_uint8_t i, j;

    rt_uint8_t f = rgb565_color >> 8;
    rt_uint8_t g = rgb565_color;

    set_column_address(a, b);
    set_row_address(c, (c + e - 1));
    set_write_ram();
    for (i = 0; i < (b - a + 1); i++)
    {
        for (j = 0; j < e; j++)
        {
            ssd1351_write_data(f);        // Line Color - RRRRRGGG
            ssd1351_write_data(g);        // Line Color - GGGBBBBB
        }
    }

    set_column_address((b - e + 1), b);
    set_row_address(c, d);
    set_write_ram();
    for (i = 0; i < (d - c + 1); i++)
    {
        for (j = 0; j < e; j++)
        {
            ssd1351_write_data(f);        // Line Color - RRRRRGGG
            ssd1351_write_data(g);        // Line Color - GGGBBBBB
        }
    }

    set_column_address(a, b);
    set_row_address((d - e + 1), d);
    set_write_ram();
    for (i = 0; i < (b - a + 1); i++)
    {
        for (j = 0; j < e; j++)
        {
            ssd1351_write_data(f);        // Line Color - RRRRRGGG
            ssd1351_write_data(g);        // Line Color - GGGBBBBB
        }
    }

    set_column_address(a, (a + e - 1));
    set_row_address(c, d);
    set_write_ram();
    for (i = 0; i < (d - c + 1); i++)
    {
        for (j = 0; j < e; j++)
        {
            ssd1351_write_data(f);        // Line Color - RRRRRGGG
            ssd1351_write_data(g);        // Line Color - GGGBBBBB
        }
    }
}




//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Character (5x7)
//
//    a: Database
//    b: Ascii,1 or 2
//    rgb565_color: D[15:8]=[RRRRR:GGG] D[7:0]=[GGG:BBBBB] (65,536 Colors Mode)
//    e: Start X Address
//    f: Start Y Address
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void show_font57(rt_uint8_t a, rt_uint8_t b, rt_uint16_t rgb565_color, rt_uint8_t e, rt_uint8_t f)
{
    rt_uint8_t *Src_Pointer;
    rt_uint8_t i, Font, Pick;

    rt_uint8_t c = rgb565_color >> 8;
    rt_uint8_t d = rgb565_color;

    switch (a)
    {
    case 1:
        Src_Pointer = &Ascii_1[(b - 1)][0];
        break;
    case 2:
        Src_Pointer = &Ascii_2[(b - 1)][0];
        break;
    }

    set_remap_format(0x75);
    for (i = 0; i <= 5; i++)
    {
        Pick = *Src_Pointer;
        set_column_address(e + i, e + i);
        set_row_address(f, f + 7);
        set_write_ram();

        Font = Pick & 0x01;
        Font = Font | (Font << 1) | (Font << 2) | (Font << 3) | (Font << 4) | (Font << 5) | (Font << 6) | (Font << 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);

        Font = (Pick & 0x02) >> 1;
        Font = Font | (Font << 1) | (Font << 2) | (Font << 3) | (Font << 4) | (Font << 5) | (Font << 6) | (Font << 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);

        Font = (Pick & 0x04) >> 2;
        Font = Font | (Font << 1) | (Font << 2) | (Font << 3) | (Font << 4) | (Font << 5) | (Font << 6) | (Font << 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);

        Font = (Pick & 0x08) >> 3;
        Font = Font | (Font << 1) | (Font << 2) | (Font << 3) | (Font << 4) | (Font << 5) | (Font << 6) | (Font << 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);

        Font = (Pick & 0x10) << 3;
        Font = Font | (Font >> 1) | (Font >> 2) | (Font >> 3) | (Font >> 4) | (Font >> 5) | (Font >> 6) | (Font >> 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);

        Font = (Pick & 0x20) << 2;
        Font = Font | (Font >> 1) | (Font >> 2) | (Font >> 3) | (Font >> 4) | (Font >> 5) | (Font >> 6) | (Font >> 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);

        Font = (Pick & 0x40) << 1;
        Font = Font | (Font >> 1) | (Font >> 2) | (Font >> 3) | (Font >> 4) | (Font >> 5) | (Font >> 6) | (Font >> 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);

        Font = Pick & 0x80;
        Font = Font | (Font >> 1) | (Font >> 2) | (Font >> 3) | (Font >> 4) | (Font >> 5) | (Font >> 6) | (Font >> 7);
        ssd1351_write_data(c & Font);
        ssd1351_write_data(d & Font);
        Src_Pointer++;
    }
    set_column_address(e + 5, e + 5);
    set_row_address(f, f + 7);
    set_write_ram();

    for (i = 0; i <= 8; i++)
    {
        ssd1351_write_data(0x00);
        ssd1351_write_data(0x00);
    }
    set_remap_format(0x74);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show String
//
//    a: Database
//    b: Data Format (MSB)
//       D[15:8]=[RRRRR:GGG] (65,536 Colors Mode)
//    c: Data Format (LSB)
//       D[7:0]=[GGG:BBBBB] (65,536 Colors Mode)
//    d: Start X Address
//    e: Start Y Address
//    * Must write "0" in the end...
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void show_string(unsigned char a, unsigned char *Data_Pointer, rt_uint16_t rgb565_color, unsigned char d, unsigned char e)
{
    unsigned char *Src_Pointer;

    Src_Pointer = Data_Pointer;
    show_font57(1, 96, rgb565_color, d, e); // No-Break Space
    //   Must be written first before the string start...

    while (1)
    {
        show_font57(a, *Src_Pointer, rgb565_color, d, e);
        Src_Pointer++;
        d += 6;
        if (*Src_Pointer == 0) break;
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void show_64k_pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    unsigned char *Src_Pointer;
    unsigned char i, j;

    Src_Pointer = Data_Pointer;
    set_column_address(a, b);
    set_row_address(c, d);
    set_write_ram();

    for (i = 0; i < (d - c + 1); i++)
    {
        for (j = 0; j < (b - a + 1); j++)
        {
            ssd1351_write_data(*Src_Pointer);
            Src_Pointer++;
            ssd1351_write_data(*Src_Pointer);
            Src_Pointer++;
        }
    }
}


void show_256k_pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    unsigned char *Src_Pointer;
    unsigned char i, j;

    Src_Pointer = Data_Pointer;
    set_column_address(a, b);
    set_row_address(c, d);
    set_write_ram();

    for (i = 0; i < (d - c + 1); i++)
    {
        for (j = 0; j < (b - a + 1); j++)
        {
            ssd1351_write_data(*Src_Pointer);
            Src_Pointer++;
            ssd1351_write_data(*Src_Pointer);
            Src_Pointer++;
            ssd1351_write_data(*Src_Pointer);
            Src_Pointer++;
        }
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Vertical Scrolling (Full Screen)
//
//    a: Scrolling Direction
//       "0x00" (Upward)
//       "0x01" (Downward)
//    b: Set Numbers of Row Scroll per Step
//    c: Set Time Interval between Each Scroll Step
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void vertical_scroll(unsigned char a, unsigned char b, unsigned char c)
{
    unsigned int i, j;

    switch (a)
    {
    case 0:
        for (i = 0; i < 128; i += b)
        {
            set_start_line(i);
            for (j = 0; j < c; j++)
            {
                //uDelay(200);
            }
        }
        break;
    case 1:
        for (i = 0; i < 128; i += b)
        {
            set_start_line(128 - i);
            for (j = 0; j < c; j++)
            {
                //uDelay(200);
            }
        }
        break;
    }
    set_start_line(0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Automatic Horizontal Scrolling (Instruction)
//
//    a: Scrolling Direction
//       "0x00" (Rightward)
//       "0x01" (Leftward)
//    b: Set Numbers of Column Scroll per Step
//    c: Set Row Address of Start
//    d: Set Numbers of Row to Be Scrolled
//    e: Set Time Interval between Each Scroll Step in Terms of Frame Frequency
//    f: Delay Time
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void horizontal_scroll(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
    ssd1351_write_cmd(0x96);            // Horizontal Scroll Setup
    ssd1351_write_data((a << 7) | b);
    ssd1351_write_data(c);
    ssd1351_write_data(d);
    ssd1351_write_data(0x00);
    ssd1351_write_data(e);
    ssd1351_write_cmd(0x9F);            // Activate Horizontal Scroll
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Deactivate Scrolling (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void deactivate_scroll()
{
    ssd1351_write_cmd(0x9E);            // Deactivate Scrolling
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade In (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void fade_in()
{
    unsigned int i;

    set_display_on_off(0x01);
    for (i = 0; i < (BRIGHTNESS + 1); i++)
    {
        set_master_current(i);

        rt_thread_delay(100);
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade Out (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void fade_out()
{
    unsigned int i;

    for (i = (BRIGHTNESS + 1); i > 0; i--)
    {
        set_master_current(i - 1);

        rt_thread_delay(100);
    }
    set_display_on_off(0x00);
}


/* Show String "RT-Thread +86 021-58995663",the font is 5*7 */
void test_string()
{
    //0 indicate the end of string
    unsigned char Name[] = {50, 52, 13, 52, 72, 82, 69, 65, 68, 0};
    //RT-Thread

    ssd1351_fill_block(0, 127, 0, 127, RGB565_BLACK);
    show_string(1, Name, RGB565_NAVY, 0, 0);
}

void test_rainbow(void)
{
    // White => Column 1~16
    ssd1351_fill_block(0x00, 0x0F, 0x00, MAX_ROW, 0xFFFF);

    // Yellow => Column 17~32
    ssd1351_fill_block(0x10, 0x1F, 0x00, MAX_ROW, 0xFFE0);

    // Purple => Column 33~48
    ssd1351_fill_block(0x20, 0x2F, 0x00, MAX_ROW, 0xF81F);

    // Cyan => Column 49~64
    ssd1351_fill_block(0x30, 0x3F, 0x00, MAX_ROW, 0x07FF);

    // Red => Column 65~80
    ssd1351_fill_block(0x40, 0x4F, 0x00, MAX_ROW, 0xF800);

    // Green => Column 81~96
    ssd1351_fill_block(0x50, 0x5F, 0x00, MAX_ROW, 0x07E0);

    // Blue => Column 97~112
    ssd1351_fill_block(0x60, 0x6F, 0x00, MAX_ROW, 0x001F);

    // Black => Column 113~128
    ssd1351_fill_block(0x70, MAX_COLUMN, 0x00, MAX_ROW, 0x0000);
}

void test_rectangle()
{
    draw_rectangle(0x00, MAX_COLUMN, 0x00, MAX_ROW, 2, RGB565_WHITE);

    draw_rectangle(0x10, 0x6F, 0x10, 0x6F, 4, RGB565_RED);

    draw_rectangle(0x20, 0x5F, 0x20, 0x5F, 6, RGB565_GREEN);

    draw_rectangle(0x30, 0x4F, 0x30, 0x4F, 8, RGB565_BLUE);

}
