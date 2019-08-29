/*!
    \file  ili9320.c
    \brief LCD driver functions

    \version 2019-6-5, V1.0.0, firmware for GD32VF103
*/

/*
    Copyright (c) 2019, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32vf103.h"
#include "ili9320.h"
#include "ili9320_font.h"

/*!
    \brief      write data to the selected LCD register
    \param[in]  register_id: the selected register id
    \param[in]  value: the register value to be written
    \param[out] none
    \retval     none
*/

void lcd_register_write(uint16_t register_id,uint16_t value)
{
    *(__IO uint16_t *) (BANK0_LCD_C)= register_id;
    *(__IO uint16_t *) (BANK0_LCD_D)= value;
}

/*!
    \brief      read the value of LCD register
    \param[in]  register_id: the register id
    \param[out] none
    \retval     the register value
*/
uint16_t lcd_register_read(uint8_t register_id)
{
    uint16_t data;
    *(__IO uint16_t *) (BANK0_LCD_C)= register_id;
    data = *(__IO uint16_t *) (BANK0_LCD_D); 
    return  data;
}

/*!
    \brief      write command to LCD register
    \param[in]  value: the register value to be written
    \param[out] none
    \retval     none
*/
void lcd_command_write(uint16_t value)
{
    /* write 16-bit index, then write reg */
    *(__IO uint16_t *) (BANK0_LCD_D) = value;
}

/*!
    \brief      prepare to write to the LCD GRAM register(R22h)
    \param[in]  none
    \param[out] none
    \retval     none
*/
void lcd_gram_write_prepare(void)
{
    *(__IO uint16_t *) (BANK0_LCD_C) = 0x0022;
}

/*!
    \brief      write RGB code to the LCD GRAM register
    \param[in]  rgb_code: the pixel color in RGB mode (5-6-5)
    \param[out] none
    \retval     none
*/
void lcd_gram_write(uint16_t rgb_code)
{
    /* write 16-bit GRAM register */
    *(__IO uint16_t *) (BANK0_LCD_D) = rgb_code;
}

/*!
    \brief      read data from GRAM
    \param[in]  none
    \param[out] none
    \retval     GRAM value
*/
uint16_t lcd_gram_read(void)
{
  uint16_t data;
    
  /* write GRAM register (R22h) */
  *(__IO uint16_t *) (BANK0_LCD_C) = 0x0022;
  /* dummy read (invalid data) */
  data = *(__IO uint16_t *) (BANK0_LCD_D);

  data = *(__IO uint16_t *) (BANK0_LCD_D); 
  return data;
}

/*!
    \brief      initialize the LCD
    \param[in]  none
    \param[out] none
    \retval     none
*/
void lcd_init(void)
{
    __IO uint16_t i;

    if(1){    /*!< if(device_code == 0x8989) */
        lcd_register_write(0x0000,0x0001);
        lcd_register_write(0x0003,0xA8A4);
        lcd_register_write(0x000C,0x0000);
        lcd_register_write(0x000D,0x080C);
        lcd_register_write(0x000E,0x2B00);
        lcd_register_write(0x001E,0x00B0);
        lcd_register_write(0x0001,0x2B3F);
        lcd_register_write(0x0002,0x0600);
        lcd_register_write(0x0010,0x0000);
        lcd_register_write(0x0011,0x6070);
        lcd_register_write(0x0005,0x0000);
        lcd_register_write(0x0006,0x0000);
        lcd_register_write(0x0016,0xEF1C);
        lcd_register_write(0x0017,0x0003);
        lcd_register_write(0x0007,0x0233);
        lcd_register_write(0x000B,0x0000);
        lcd_register_write(0x000F,0x0000);
        lcd_register_write(0x0041,0x0000);
        lcd_register_write(0x0042,0x0000);
        lcd_register_write(0x0048,0x0000);
        lcd_register_write(0x0049,0x013F);
        lcd_register_write(0x004A,0x0000);
        lcd_register_write(0x004B,0x0000);
        lcd_register_write(0x0044,0xEF00);
        lcd_register_write(0x0045,0x0000);
        lcd_register_write(0x0046,0x013F);
        lcd_register_write(0x0030,0x0707);
        lcd_register_write(0x0031,0x0204);
        lcd_register_write(0x0032,0x0204);
        lcd_register_write(0x0033,0x0502);
        lcd_register_write(0x0034,0x0507);
        lcd_register_write(0x0035,0x0204);
        lcd_register_write(0x0036,0x0204);
        lcd_register_write(0x0037,0x0502);
        lcd_register_write(0x003A,0x0302);
        lcd_register_write(0x003B,0x0302);
        lcd_register_write(0x0023,0x0000);
        lcd_register_write(0x0024,0x0000);
        lcd_register_write(0x0025,0x8000);
        lcd_register_write(0x004e,0);
        lcd_register_write(0x004f,0);
    }else{
        return;
    }
    
    for(i=50000;i>0;i--);
}

/*!
    \brief      set the cursor of LCD
    \param[in]  x: the row-coordinate
    \param[in]  y: the column-coordinate
    \param[out] none
    \retval     none
*/
void lcd_cursor_set(uint16_t x,uint16_t y)
{
    lcd_register_write(0x004e,x);
    lcd_register_write(0x004f,y);
}

/*!
    \brief      clear the LCD screen to the specified color
    \param[in]  color: specified screen color
    \param[out] none
    \retval     none
*/
void lcd_clear(uint16_t color)
{
    uint32_t index=0;
    lcd_cursor_set(0,0);
    /* prepare to write GRAM */
    lcd_gram_write_prepare();
    for(index=0;index<76800;index++){
        *(__IO uint16_t *) (BANK0_LCD_D) = color;
    }
}

/*!
    \brief      set the point according to the specified position and color
    \param[in]  x: the row-coordinate
    \param[in]  y: the column-coordinate
    \param[in]  point: specified color of the point
    \param[out] none
    \retval     none
*/
void lcd_point_set(uint16_t x,uint16_t y,uint16_t point)
{
    if ((x > 240)||(y > 320)){
        return;
    }
    lcd_cursor_set(x,y);
    lcd_gram_write_prepare();
    lcd_gram_write(point);
}

/*!
    \brief      get point GRAM according to the specified position
    \param[in]  x: the row-coordinate
    \param[in]  y: the column-coordinate
    \param[out] none
    \retval     GRAM value of point 
*/
uint16_t lcd_point_get(uint16_t x,uint16_t y)
{
    uint16_t data;
    
    if ((x > 240)||(y > 320)){
        return 0;
    }
    
    lcd_cursor_set(x,y);
    data = lcd_gram_read();

    return  data;
}

/*!
    \brief      set window area
    \param[in]  start_x: the start position of row-coordinate
    \param[in]  start_y: the start position of column-coordinate
    \param[in]  end_x: the end position of row-coordinate
    \param[in]  end_y: the end position of column-coordinate
    \param[out] none
    \retval     none
*/
void lcd_windows_set(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y)
{
    lcd_cursor_set(start_x, start_y);

    lcd_register_write(0x0050, start_x);
    lcd_register_write(0x0052, start_y);
    lcd_register_write(0x0051, end_x);
    lcd_register_write(0x0053, end_y);
}

/*!
    \brief      draw a horizontal line on LCD screen
    \param[in]  x: the row-coordinate
    \param[in]  start_y: the start column-coordinate
    \param[in]  end_y: the end column-coordinate
    \param[in]  color: specified color of the point
    \param[in]  width: line width
    \param[out] none
    \retval     none
*/

void lcd_hline_draw(uint16_t x,uint16_t start_y,uint16_t end_y,uint16_t color,uint16_t width)
{
    uint16_t i, y;

    for (i = 0; i < width; i++) {
        uint16_t sx = x + i;

        for (y = start_y; y < end_y; y++) {
            lcd_point_set(sx, y, color);
        }
    }
}

/*!
    \brief      draw a rectangle according to the specified position and color
    \param[in]  start_x: the start position of row-coordinate
    \param[in]  start_y: the start position of column-coordinate
    \param[in]  end_x: the end position of row-coordinate
    \param[in]  end_y: the end position of column-coordinate
    \param[in]  point: specified color of the point
    \param[out] none
    \retval     none
*/
void lcd_rectangle_draw(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y,uint16_t point)
{
    uint16_t x,y;
    x=start_x;
    y=start_y;
    /* draw four lines */
    for(x=start_x;x<end_x;x++){
        /* draw a point */
        lcd_point_set(x,y,point);
    }
    for(y=start_y;y<end_y;y++){
        lcd_point_set(x,y,point);
    }
    for(x=end_x;x>start_x;x--){
        lcd_point_set(x,y,point);
    }
    for(y=end_y;y>start_y;y--){
        lcd_point_set(x,y,point);
    }
}

/*!
    \brief      fill the specified color to a rectangle
    \param[in]  start_x: the start position of row-coordinate
    \param[in]  start_y: the start position of column-coordinate
    \param[in]  end_x: the end position of row-coordinate
    \param[in]  end_y: the end position of column-coordinate
    \param[in]  color: specified color
    \param[out] none
    \retval     none
*/
void lcd_rectangle_fill(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y,uint16_t color)
{
    uint16_t x, y;
    x = start_x;
    y = start_y;

    for (x = start_x; x < end_x; x++) {
        for (y = start_y; y < end_y; y++) {
            lcd_point_set(x, y, color);
        }
    }
}

/*!
    \brief      draw a picture on LCD screen according to the specified position
    \param[in]  start_x: the start position of row-coordinate
    \param[in]  start_y: the start position of column-coordinate
    \param[in]  end_x: the end position of row-coordinate
    \param[in]  end_y: the end position of column-coordinate
    \param[in]  pic: the picture pointer
    \param[out] none
    \retval     none
*/
void lcd_picture_draw(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y,uint16_t *pic)
{
    uint32_t i, total;
    uint16_t *picturepointer = pic;
    uint16_t x,y;

    x = start_x;
    y = start_y;

    total = (end_x - start_x + 1) * (end_y - start_y + 1);
    
    for(i = 0; i < total; i ++){
        /* set point according to the specified position and color */
        lcd_point_set(x,y,*picturepointer++);
        x++;
        if(x > end_x){
            y++;
            x = start_x;
        }
    }
}

/*!
    \brief      display a char on LCD screen according to the specified position
    \param[in]  x: the start position of row-coordinate
    \param[in]  y: the start position of column-coordinate
    \param[in]  c: the char
    \param[in]  char_color: the color of char
    \param[in]  c_format: the structure of char format
                  font: CHAR_FONT_8_16 or CHAR_FONT_16_24
                  direction: CHAR_DIRECTION_HORIZONTAL or CHAR_DIRECTION_VERTICAL
                  char_color: the color of char
                  bk_color: the color of background
    \param[out] none
    \retval     none
*/
void lcd_char_display(uint16_t x,uint16_t y,uint8_t c,char_format_struct c_format)
{
    uint16_t i = 0, j = 0;
    uint8_t temp_char = 0;
    uint16_t temp_char_16 = 0;
    
    if(CHAR_FONT_8_16 == c_format.font){ /* 8x16 ASCII */
        for (i = 0; i < 16; i++) {
            temp_char = ascii_8x16[((c - 0x20) * 16) + i];
            if(CHAR_DIRECTION_HORIZONTAL == c_format.direction){
                for (j = 0; j < 8; j++) {
                    if (((temp_char >> (7 - j)) & 0x01) == 0x01) {
                        /* set point of char */
                        lcd_point_set(x - i, y + j, c_format.char_color);
                    } else {
                        /* set point of background */
                        lcd_point_set(x - i, y + j, c_format.bk_color);
                    }
                }
            }else{
                for (j = 0; j < 8; j++) {
                    if (((temp_char >> (7 - j)) & 0x01) == 0x01) {
                        /* set point of char */
                        lcd_point_set(x + j, y + i, c_format.char_color);
                    } else {
                        /* set point of background */
                        lcd_point_set(x + j, y + i, c_format.bk_color);
                    }
                }
            }
        }
    }else if(CHAR_FONT_16_24 == c_format.font){ /* 16x24 ASCII */
        for (i = 0; i < 24; i++) {
            temp_char_16 = ASCII_Table_16x24[((c - 0x20) * 24) + i];
            if(CHAR_DIRECTION_HORIZONTAL == c_format.direction){
                for (j = 0; j < 16; j++) {
                    if (((temp_char_16 >> j) & 0x01) == 0x01) {
                        /* set point of char */
                        lcd_point_set(x - i, y + j, c_format.char_color);
                    } else {
                        /* set point of background */
                        lcd_point_set(x - i, y + j, c_format.bk_color);
                    }
                }
            }else{
                for (j = 0; j < 16; j++) {
                    if (((temp_char_16 >> j) & 0x01) == 0x01) {
                        /* set point of char */
                        lcd_point_set(x + j, y + i, c_format.char_color);
                    } else {
                        /* set point of background */
                        lcd_point_set(x + j, y + i, c_format.bk_color);
                    }
                }
            }
        }
    }
}
