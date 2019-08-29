/*!
    \file  usbh_usr.c
    \brief the user callback function definitions

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

#include "drv_usb_hw.h"
#include "usbh_usr.h"
#include "usbh_hid_mouse.h"
#include "usbh_hid_keybd.h"
#include "lcd_log.h"
#include <string.h>
#include "gd32vf103v_lcd_eval.h"

#define SMALL_FONT_COLUMN_WIDTH    8
#define SMALL_FONT_LINE_WIDTH      16

#define KYBRD_FIRST_COLUMN         (uint16_t)0
#define KYBRD_LAST_COLUMN          (uint16_t)320
#define KYBRD_FIRST_LINE           (uint16_t)144
#define KYBRD_LAST_LINE            (uint16_t)40

uint16_t keybrd_char_xpos = 0;
uint16_t keybrd_char_ypos = 0;
uint8_t  need_clear = 0;
uint8_t  user_input_flag = 0;
uint8_t  button_pressed_flag = 0;

extern int16_t  x_loc, y_loc;
extern __IO int16_t prev_x, prev_y;
extern uint16_t LINE;

extern char_format_struct charform;

/* points to the DEVICE_PROP structure of current device */
usbh_user_cb user_callback_funs =
{
    usbh_user_init,
    usbh_user_deinit,
    usbh_user_device_connected,
    usbh_user_device_reset,
    usbh_user_device_disconnected,
    usbh_user_over_current_detected,
    usbh_user_device_speed_detected,
    usbh_user_device_descavailable,
    usbh_user_device_address_assigned,
    usbh_user_configuration_descavailable,
    usbh_user_manufacturer_string,
    usbh_user_product_string,
    usbh_user_serialnum_string,
    usbh_user_enumeration_finish,
    usbh_user_userinput,
    NULL,
    usbh_user_device_not_supported,
    usbh_user_unrecovered_error
};

/* printf messages */
const uint8_t MSG_HOST_INIT[]          = "> Host Library Initialized.";
const uint8_t MSG_DEV_ATTACHED[]       = "> Device Attached.";
const uint8_t MSG_DEV_DISCONNECTED[]   = "> Device Disconnected.";
const uint8_t MSG_DEV_ENUMERATED[]     = "> Enumeration completed.";
const uint8_t MSG_DEV_HIGHSPEED[]      = "> High speed device detected.";
const uint8_t MSG_DEV_FULLSPEED[]      = "> Full speed device detected.";
const uint8_t MSG_DEV_LOWSPEED[]       = "> Low speed device detected.";
const uint8_t MSG_DEV_ERROR[]          = "> Device fault.";

const uint8_t MSG_MSC_CLASS[]          = "> Mass storage device connected.";
const uint8_t MSG_HID_CLASS[]          = "> HID device connected.";

const uint8_t USB_HID_MouseStatus[]    = "> HID Demo Device : Mouse.";
const uint8_t USB_HID_KeybrdStatus[]   = "> HID Demo Device : Keyboard.";
const uint8_t MSG_UNREC_ERROR[]        = "> UNRECOVERED ERROR STATE.";

const uint8_t MSG_HOST_HEADER[]        = "USBFS HID Host";
const uint8_t MSG_HOST_FOOTER[]        = "USB Host Library v2.0.0";

const uint8_t MSG_LIB_START[]          = "##### USB Host library started #####";
const uint8_t MSG_DEV_NOSUP[]          = "> Device not supported.";
const uint8_t MSG_OVERCURRENT[]        = "> Overcurrent detected.";
const uint8_t MSG_USE_KYBD[]           = "> Use Keyboard to tape characters: ";
const uint8_t MSG_RESET_DEV[]          = "> Reset the USB device.";

const uint8_t MSG_WAIT_INPUT[]         = "> Wait for user input!";
const uint8_t MSG_USER_INPUT[]         = "> User has input!";
  
/*!
    \brief      user operation for host-mode initialization
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_init(void)
{
    static uint8_t startup = 0;

    if (0U == startup) {
        startup = 1U;

        /* configure the User key */
        gd_eval_key_init(KEY_CET, KEY_MODE_GPIO);

        exmc_lcd_init();

        usb_mdelay(50);

        lcd_init();

        lcd_log_init();

        lcd_log_header_set((uint8_t *)MSG_HOST_HEADER, 80);

        lcd_log_print((uint8_t *)MSG_LIB_START, sizeof(MSG_LIB_START) - 1, LCD_COLOR_WHITE);

        charform.char_color = LCD_COLOR_RED;

        lcd_log_footer_set((uint8_t *)MSG_HOST_FOOTER, 60);
    }
}

/*!
    \brief      user operation for device attached
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_connected(void)
{
    if (need_clear != 0) {
        lcd_log_text_zone_clear(30, 0, 210, 320);

        need_clear = 0;
    }

    lcd_log_print((uint8_t *)MSG_DEV_ATTACHED, sizeof(MSG_DEV_ATTACHED) - 1, LCD_COLOR_WHITE);
}

/*!
    \brief      user operation when unrecoveredError happens
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_unrecovered_error (void)
{
    lcd_log_print((uint8_t *)MSG_UNREC_ERROR, sizeof(MSG_UNREC_ERROR) - 1, LCD_COLOR_WHITE);
}

/*!
    \brief      user operation for device disconnect event
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_disconnected (void)
{
    LINE = 30;

    lcd_log_text_zone_clear(30, 0, 210, 320);

    lcd_log_print((uint8_t *)MSG_DEV_DISCONNECTED, sizeof(MSG_DEV_DISCONNECTED) - 1, LCD_COLOR_WHITE);

    need_clear = 1;
}

/*!
    \brief      user operation for reset USB device
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_reset(void)
{
    lcd_log_print((uint8_t *)MSG_RESET_DEV, sizeof(MSG_RESET_DEV) - 1, LCD_COLOR_WHITE);
}

/*!
    \brief      user operation for detectting device speed
    \param[in]  device_speed: the device speed
    \param[out] none
    \retval     none
*/
void usbh_user_device_speed_detected(uint32_t device_speed)
{
    if (PORT_SPEED_HIGH == device_speed) {
        lcd_log_print((uint8_t *)MSG_DEV_HIGHSPEED, sizeof(MSG_DEV_HIGHSPEED) - 1, LCD_COLOR_WHITE);
    } else if (PORT_SPEED_FULL == device_speed) {
        lcd_log_print((uint8_t *)MSG_DEV_FULLSPEED, sizeof(MSG_DEV_FULLSPEED) - 1, LCD_COLOR_WHITE);
    } else if (PORT_SPEED_LOW == device_speed) {
        lcd_log_print((uint8_t *)MSG_DEV_LOWSPEED, sizeof(MSG_DEV_LOWSPEED) - 1, LCD_COLOR_WHITE);
    } else {
        lcd_log_print((uint8_t *)MSG_DEV_ERROR, sizeof(MSG_DEV_ERROR) - 1, LCD_COLOR_WHITE);
    }
}

/*!
    \brief      user operation when device descriptor is available
    \param[in]  device_desc: the device descripter
    \param[out] none
    \retval     none
*/
void usbh_user_device_descavailable(void *device_desc)
{
    uint8_t TempStr[64], str_len = 0;
    usb_desc_dev *pDevStr = device_desc;

    sprintf((char *)TempStr, "VID: %04Xh", (uint32_t)pDevStr->idVendor);
    str_len = strlen((const char *)TempStr);
    lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);

    sprintf((char *)TempStr, "PID: %04Xh" , (uint32_t)pDevStr->idProduct);
    str_len = strlen((const char *)TempStr);
    lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);
}

/*!
    \brief      USB device is successfully assigned the address 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_address_assigned(void)
{

}

/*!
    \brief      user operation when configuration descriptor is available
    \param[in]  cfg_desc: pointer to configuration descriptor
    \param[in]  itf_desc: pointer to interface descriptor
    \param[in]  ep_desc: pointer to endpoint descriptor
    \param[out] none
    \retval     none
*/
void usbh_user_configuration_descavailable(usb_desc_config *cfg_desc,
                                          usb_desc_itf *itf_desc,
                                          usb_desc_ep *ep_desc)
{
    usb_desc_itf *id = itf_desc;  

    if (0x08U  == (*id).bInterfaceClass) {
        lcd_log_print((uint8_t *)MSG_MSC_CLASS, sizeof(MSG_MSC_CLASS) - 1, LCD_COLOR_WHITE);
    } else if (0x03U  == (*id).bInterfaceClass) {
        lcd_log_print((uint8_t *)MSG_HID_CLASS, sizeof(MSG_HID_CLASS) - 1, LCD_COLOR_WHITE);
    }
}

/*!
    \brief      user operation when manufacturer string exists
    \param[in]  manufacturer_string: manufacturer string of usb device
    \param[out] none
    \retval     none
*/
void usbh_user_manufacturer_string(void *manufacturer_string)
{
    uint8_t TempStr[64], str_len = 0;

    sprintf((char *)TempStr, "Manufacturer: %s", (char *)manufacturer_string);
    str_len = strlen((const char *)TempStr);
    lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);
}

/*!
    \brief      user operation when product string exists
    \param[in]  product_string: product string of usb device
    \param[out] none
    \retval     none
*/
void usbh_user_product_string(void *product_string)
{
    uint8_t TempStr[64], str_len = 0;

    sprintf((char *)TempStr, "Product: %s", (char *)product_string);
    str_len = strlen((const char *)TempStr);
    lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);
}

/*!
    \brief      user operatin when serialNum string exists
    \param[in]  serial_num_string: serialNum string of usb device
    \param[out] none
    \retval     none
*/
void usbh_user_serialnum_string(void *serial_num_string)
{
    uint8_t TempStr[64], str_len = 0;

    sprintf((char *)TempStr, "Serial Number: %s", (char *)serial_num_string);
    str_len = strlen((const char *)TempStr);
    lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);
}

/*!
    \brief      user response request is displayed to ask for application jump to class
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_enumeration_finish(void)
{
    /* enumeration complete */
    uint8_t Str1[] = "To start the HID class operations: ";
    uint8_t Str2[] = "Press CET Key...                  ";

    lcd_log_print((uint8_t *)MSG_DEV_ENUMERATED, sizeof(MSG_DEV_ENUMERATED) - 1, LCD_COLOR_WHITE);
    lcd_log_print((uint8_t *)Str1, sizeof(Str1) - 1, LCD_COLOR_GREEN);
    lcd_log_print((uint8_t *)Str2, sizeof(Str2) - 1, LCD_COLOR_GREEN);
} 

/*!
    \brief      user operation when device is not supported
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_not_supported(void)
{
    lcd_log_print((uint8_t *)MSG_DEV_NOSUP, sizeof(MSG_DEV_NOSUP) - 1, LCD_COLOR_WHITE);
}

/*!
    \brief      user action for application state entry
    \param[in]  none
    \param[out] none
    \retval     none
*/
usbh_user_status usbh_user_userinput(void)
{
    usbh_user_status usbh_usr_status = USBH_USER_NO_RESP;

    /*key B3 is in polling mode to detect user action */
    if (SET != gd_eval_key_state_get(KEY_CET)) {
        usbh_usr_status = USBH_USER_RESP_OK;
    }

    return usbh_usr_status;
}

/*!
    \brief      user operation for device overcurrent detection event
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_over_current_detected (void)
{
    lcd_log_print((uint8_t *)MSG_OVERCURRENT, sizeof(MSG_OVERCURRENT) - 1, LCD_COLOR_WHITE);
}

/*!
    \brief      init mouse window
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usr_mouse_init (void)
{
    lcd_log_print((uint8_t *)USB_HID_MouseStatus, sizeof(USB_HID_MouseStatus) - 1, LCD_COLOR_WHITE);

    lcd_rectangle_fill(30, 0, 210, 320, LCD_COLOR_BLACK);

    lcd_rectangle_draw(80, 40, 190, 280, LCD_COLOR_YELLOW);
  
    lcd_rectangle_fill(40, 40, 70, 110, LCD_COLOR_WHITE);
    lcd_rectangle_fill(40, 130, 70, 190, LCD_COLOR_WHITE);
    lcd_rectangle_fill(40, 210, 70, 280, LCD_COLOR_WHITE);

    hid_mouse_button_released(0);
    hid_mouse_button_released(1);
    hid_mouse_button_released(2);

    charform.char_color = LCD_COLOR_GREEN;
    charform.bk_color = LCD_COLOR_BLACK;

    lcd_char_display (184, 44, 'x', charform);

    x_loc  = 0;
    y_loc  = 0; 
    prev_x = 0;
    prev_y = 0;
}

/*!
    \brief      process mouse data
    \param[in]  data: mouse data to be displayed
    \param[out] none
    \retval     none
*/
void usr_mouse_process_data(hid_mouse_data_struct *data)
{
    uint8_t idx = 1;
    static uint8_t b_state[3] = {0};

    if ((data->x != 0) && (data->y != 0)) {
        hid_mouse_update_position(data->x, data->y);
    }

    for (idx = 0; idx < 3; idx ++) {
        if (data->button & 1 << idx) {
            if (b_state[idx] == 0) {
                hid_mouse_button_pressed(idx);
                b_state[idx] = 1;
            }
        } else {
            if (b_state[idx] == 1) {
                hid_mouse_button_released(idx);
                b_state[idx] = 0;
            }
        }
    }
}

/*!
    \brief      init keyboard window
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  usr_keybrd_init (void)
{
    LINE = 190;

    lcd_rectangle_fill(30, 0, 210, 320, LCD_COLOR_BLACK);

    lcd_log_print((uint8_t *)USB_HID_KeybrdStatus, sizeof(USB_HID_KeybrdStatus) - 1, LCD_COLOR_WHITE);

    lcd_log_print((uint8_t *)MSG_USE_KYBD, sizeof(MSG_USE_KYBD) - 1, LCD_COLOR_WHITE);

    lcd_hline_draw(150, 0, 320, LCD_COLOR_GREEN, 2);

    keybrd_char_xpos = KYBRD_FIRST_LINE;
    keybrd_char_ypos = KYBRD_FIRST_COLUMN;

    lcd_hline_draw(30, 0, 320, LCD_COLOR_GREEN, 2);
}

/*!
    \brief      process keyboard data
    \param[in]  data: keyboard data to be displayed
    \param[out] none
    \retval     none
*/
void usr_keybrd_process_data (uint8_t data)
{
    if (data == '\n') {
        keybrd_char_ypos = KYBRD_FIRST_COLUMN;

        /* increment char x position */
        keybrd_char_xpos -= SMALL_FONT_LINE_WIDTH;
    } else if (data == '\r') {
        /* manage deletion of charactter and upadte cursor location */
        if (keybrd_char_ypos == KYBRD_FIRST_COLUMN) {
            /* first character of first line to be deleted */
            if (keybrd_char_xpos == KYBRD_FIRST_LINE) {
                keybrd_char_ypos = KYBRD_FIRST_COLUMN;
            } else {
                keybrd_char_xpos -= SMALL_FONT_LINE_WIDTH;
                keybrd_char_ypos =(KYBRD_LAST_COLUMN + SMALL_FONT_COLUMN_WIDTH); 
            }
        } else {
            keybrd_char_ypos += SMALL_FONT_COLUMN_WIDTH;
        }

        charform.char_color = LCD_COLOR_BLACK;

        lcd_char_display(keybrd_char_xpos, keybrd_char_ypos, ' ', charform);
    } else {
        charform.char_color = LCD_COLOR_WHITE;
        lcd_char_display(keybrd_char_xpos, keybrd_char_ypos, data, charform);

        /* update the cursor position on lcd */
        /* check if the y position has reached the last column*/
        if(keybrd_char_ypos == KYBRD_LAST_COLUMN) {
            keybrd_char_ypos = KYBRD_FIRST_COLUMN;

            /* increment char X position */
            keybrd_char_xpos -= SMALL_FONT_LINE_WIDTH;
        } else {
            /* increment char Y position */
            keybrd_char_ypos += SMALL_FONT_COLUMN_WIDTH;
        }
    }

    if (keybrd_char_xpos <= KYBRD_LAST_LINE) {
        lcd_rectangle_fill(32, 0, 144, 320, LCD_COLOR_BLACK);

        keybrd_char_xpos = KYBRD_FIRST_LINE;
    }
}

/*!
    \brief      deinit user state and associated variables
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_deinit(void)
{
}
