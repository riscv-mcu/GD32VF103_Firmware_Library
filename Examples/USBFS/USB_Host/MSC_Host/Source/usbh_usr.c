/*!
    \file  usbh_usr.c
    \brief user application layer for USBFS host-mode MSC class operation

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

#include <string.h>
#include "usbh_usr.h"
#include "drv_usb_hw.h"
#include "ff.h"
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bbb.h"
#include "lcd_log.h"

extern uint16_t LINE;
extern usb_core_driver usbh_msc_core;
extern char_format_struct charform;

FATFS fatfs;
FIL file;

uint8_t line_idx = 0;
uint8_t need_clear = 0;
uint8_t usbh_usr_application_state = USBH_USR_FS_INIT;
uint8_t WriteTextBuff[] = "GD32 Connectivity line Host Demo application using FAT_FS   ";

/*  points to the DEVICE_PROP structure of current device */
usbh_user_cb user_callback_funs =
{
    usbh_user_init,
    usbh_user_deinit,
    usbh_user_device_connected,
    usbh_user_device_reset,
    usbh_user_device_disconnected,
    usbh_user_over_current_detected,
    usbh_user_device_speed_detected,
    usbh_user_device_desc_available,
    usbh_user_device_address_assigned,
    usbh_user_configuration_descavailable,
    usbh_user_manufacturer_string,
    usbh_user_product_string,
    usbh_user_serialnum_string,
    usbh_user_enumeration_finish,
    usbh_user_userinput,
    usbh_usr_msc_application,
    usbh_user_device_not_supported,
    usbh_user_unrecovered_error
};

const uint8_t MSG_HOST_INIT[]        = "> Host Library Initialized.";
const uint8_t MSG_DEV_ATTACHED[]     = "> Device Attached.";
const uint8_t MSG_DEV_DISCONNECTED[] = "> Device Disconnected.";
const uint8_t MSG_DEV_ENUMERATED[]   = "> Enumeration completed.";
const uint8_t MSG_DEV_HIGHSPEED[]    = "> High speed device detected.";
const uint8_t MSG_DEV_FULLSPEED[]    = "> Full speed device detected.";
const uint8_t MSG_DEV_LOWSPEED[]     = "> Low speed device detected.";
const uint8_t MSG_DEV_ERROR[]        = "> Device fault.";

const uint8_t MSG_HOST_HEADER[]      = "> USBFS MSC Host";
const uint8_t MSG_HOST_FOOTER[]      = "> USB Host Library v2.0.0";

const uint8_t MSG_LIB_START[]        = "##### USB Host library started #####";
const uint8_t MSG_DEV_NOSUP[]        = "> Device not supported.";
const uint8_t MSG_OVERCURRENT[]      = "> Overcurrent detected.";
const uint8_t MSG_RESET_DEV[]        = "> Reset the USB device.";

const uint8_t MSG_MSC_CLASS[]        = "> Mass storage device connected.";
const uint8_t MSG_HID_CLASS[]        = "> HID device connected.";
const uint8_t MSG_DISK_SIZE[]        = "> Size of the disk in MBytes: ";
const uint8_t MSG_LUN[]              = "> LUN Available in the device:";
const uint8_t MSG_ROOT_CONT[]        = "> Exploring disk flash ...";
const uint8_t MSG_WR_PROTECT[]       = "> The disk is write protected.";
const uint8_t MSG_UNREC_ERROR[]      = "> UNRECOVERED ERROR STATE.";
const uint8_t MSG_FILE_NOTINIT[]     = "> Cannot initialize File System.";
const uint8_t MSG_FILE_INIT[]        = "> File System initialized.";
const uint8_t MSG_Write_File[]       = "> Writing File to disk flash ...";
const uint8_t MSG_Write_Protect[]    = "> Disk flash is write protected ";
const uint8_t MSG_NOT_WRITE[]        = "> GD32.TXT CANNOT be writen.";
const uint8_t MSG_CREATE_FILE[]      = "> GD32.TXT created in the disk.";

static uint8_t explore_disk     (char* path, uint8_t recu_level);
static void toggle_leds         (void);

/*!
    \brief      user operation for host-mode initialization
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_init(void)
{
    static uint8_t startup = 0U;

    if (0U == startup) {
        startup = 1U;

        /* configure the LEDs and KEYs*/
        gd_eval_led_init(LED2);
        gd_eval_led_init(LED3);
        gd_eval_key_init(KEY_CET, KEY_MODE_GPIO);
        gd_eval_key_init(KEY_B, KEY_MODE_GPIO);
        gd_eval_key_init(KEY_C, KEY_MODE_GPIO);

        exmc_lcd_init();

        usb_mdelay(50);

        lcd_init();

        lcd_log_init();

        lcd_log_header_set((uint8_t *)MSG_HOST_HEADER, 80);

        lcd_log_print((uint8_t *)MSG_LIB_START, sizeof(MSG_LIB_START) - 1, LCD_COLOR_WHITE);

        charform.char_color = LCD_COLOR_RED;

        lcd_log_footer_set((uint8_t *)MSG_HOST_FOOTER, 70);
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
    LINE = 190;

    lcd_log_text_zone_clear(30, 0, 210, 320);

    lcd_log_print((uint8_t *)MSG_DEV_DISCONNECTED, sizeof(MSG_DEV_DISCONNECTED) - 1, LCD_COLOR_WHITE);

    need_clear = 1;
}

/*!
    \brief      user operation for reset USB Device
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
    \param[in]  device_speed: device speed
    \param[out] none
    \retval     none
*/
void usbh_user_device_speed_detected(uint32_t device_speed)
{
    if (PORT_SPEED_HIGH == device_speed) {
        lcd_log_print((uint8_t *)MSG_DEV_HIGHSPEED, sizeof(MSG_DEV_HIGHSPEED) - 1, LCD_COLOR_WHITE);
    } else if(PORT_SPEED_FULL == device_speed) {
        lcd_log_print((uint8_t *)MSG_DEV_FULLSPEED, sizeof(MSG_DEV_FULLSPEED) - 1, LCD_COLOR_WHITE);
    } else if(PORT_SPEED_LOW == device_speed) {
        lcd_log_print((uint8_t *)MSG_DEV_LOWSPEED, sizeof(MSG_DEV_LOWSPEED) - 1, LCD_COLOR_WHITE);
    } else {
        lcd_log_print((uint8_t *)MSG_DEV_ERROR, sizeof(MSG_DEV_ERROR) - 1, LCD_COLOR_WHITE);
    }
}

/*!
    \brief      user operation when device descriptor is available
    \param[in]  device_desc: device descriptor
    \param[out] none
    \retval     none
*/
void usbh_user_device_desc_available(void *device_desc)
{
    uint8_t TempStr[64], str_len = 0;
    usb_desc_dev *pDevStr = device_desc;

    sprintf((char *)TempStr, "VID: %04Xh", (uint32_t)pDevStr->idVendor);
    str_len = strlen((const char *)TempStr);
    lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);

    sprintf((char *)TempStr, "PID: %04Xh", (uint32_t)pDevStr->idProduct);
    str_len = strlen((const char *)TempStr);
    lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);
}

/*!
    \brief      usb device is successfully assigned the Address 
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
    uint8_t Str1[] = "To see the disk information: ";
    uint8_t Str2[] = "Press CET Key...";
    
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
    \retval     user response for user key
*/
usbh_user_status usbh_user_userinput(void)
{
    usbh_user_status usbh_usr_status = USBH_USER_NO_RESP;

    /*key B3 is in polling mode to detect user action */
    if (RESET == gd_eval_key_state_get(KEY_CET)) {
        LINE = 190;

        lcd_log_text_zone_clear(30, 0, 210, 320);

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
    \brief      demo application for mass storage
    \param[in]  pudev: pointer to device
    \param[in]  id: no use here
    \param[out] none
    \retval     status
*/
int usbh_usr_msc_application(void)
{
    FRESULT res;
    uint16_t bytesWritten, bytesToWrite;

    uint8_t Str1[] = "> To see the root content of the disk ";
    uint8_t Str2[] = "> Press C Key to write file";
    uint8_t Str3[] = "> The MSC host demo is end.";
    uint8_t Str4[] = "> Press CET key...";
    uint8_t TempStr[64], str_len = 0;

    switch(usbh_usr_application_state)
    {
        case USBH_USR_FS_INIT:
            /* initialises the file system*/
            if (FR_OK != f_mount(0, &fatfs)) {
                lcd_log_print((uint8_t *)MSG_FILE_NOTINIT, sizeof(MSG_FILE_NOTINIT) - 1, LCD_COLOR_WHITE);

                return(-1);
            }

            lcd_log_print((uint8_t *)MSG_FILE_INIT, sizeof(MSG_FILE_INIT) - 1, LCD_COLOR_WHITE);

            sprintf((char *)TempStr, "> Disk capacity: %ud Bytes.", \
                    usbh_msc_param.msc_capacity * usbh_msc_param.msc_page_len);

            str_len = strlen((const char *)TempStr);

            lcd_log_print((uint8_t *)TempStr, str_len, LCD_COLOR_WHITE);

            if (DISK_WRITE_PROTECTED == usbh_msc_param.msc_write_protect) {
                lcd_log_print((uint8_t *)MSG_WR_PROTECT, sizeof(MSG_WR_PROTECT) - 1, LCD_COLOR_WHITE);
            }

            usbh_usr_application_state = USBH_USR_FS_READLIST;
            break;

        case USBH_USR_FS_READLIST:
            lcd_log_print((uint8_t *)MSG_ROOT_CONT, sizeof(MSG_ROOT_CONT) - 1, LCD_COLOR_WHITE);
            lcd_log_print((uint8_t *)Str1, sizeof(Str1) - 1, LCD_COLOR_GREEN);
            lcd_log_print((uint8_t *)Str4, sizeof(Str4) - 1, LCD_COLOR_GREEN);

            /*Key B3 in polling*/
            while ((usbh_msc_core.host.connect_status)  && \
                (SET == gd_eval_key_state_get (KEY_CET))) {
                toggle_leds();
            }
  
            LINE = 190;

            lcd_log_text_zone_clear(30, 0, 210, 320);

            explore_disk("0:/", 1);
            line_idx = 0;
            usbh_usr_application_state = USBH_USR_FS_WRITEFILE;
            break;

        case USBH_USR_FS_WRITEFILE:
            usb_mdelay(100U);

            lcd_log_print((uint8_t *)Str2, sizeof(Str2) - 1, LCD_COLOR_GREEN);

            /*key b3 in polling*/
            while ((usbh_msc_core.host.connect_status) && \
                    (SET == gd_eval_key_state_get (KEY_C))) {
                toggle_leds();
            }

            lcd_log_print((uint8_t *)MSG_Write_File, sizeof(MSG_Write_File) - 1, LCD_COLOR_WHITE);

            if (DISK_WRITE_PROTECTED == usbh_msc_param.msc_write_protect) {
                lcd_log_print((uint8_t *)MSG_Write_Protect, sizeof(MSG_Write_Protect) - 1, LCD_COLOR_WHITE);

                usbh_usr_application_state = USBH_USR_FS_DEMOEND;
                break;
            }

            /* register work area for logical drives */
            f_mount(0, &fatfs);

            if (FR_OK == f_open(&file, "0:GD32.TXT", FA_CREATE_ALWAYS | FA_WRITE)) {
                /* write buffer to file */
                bytesToWrite = sizeof(WriteTextBuff); 
                res = f_write (&file, WriteTextBuff, bytesToWrite, (void *)&bytesWritten);
                /* EOF or error */
                if ((0U == bytesWritten) || (FR_OK != res)) {
                    lcd_log_print((uint8_t *)MSG_NOT_WRITE, sizeof(MSG_NOT_WRITE) - 1, LCD_COLOR_WHITE);
                } else {
                    lcd_log_print((uint8_t *)MSG_CREATE_FILE, sizeof(MSG_CREATE_FILE) - 1, LCD_COLOR_WHITE);
                }

                /* close file and filesystem */
                f_close(&file);
                f_mount(0, NULL); 
            } else {
                lcd_log_print((uint8_t *)MSG_CREATE_FILE, sizeof(MSG_CREATE_FILE) - 1, LCD_COLOR_WHITE);
            }

            usbh_usr_application_state = USBH_USR_FS_DEMOEND;
            lcd_log_print((uint8_t *)Str3, sizeof(Str3) - 1, LCD_COLOR_GREEN);
            break;

        case USBH_USR_FS_DEMOEND:
            break;

        default:
            break;
    }

    return(0);
}

/*!
    \brief      displays disk content
    \param[in]  path: pointer to root path
    \param[in]  recu_level: recursive level
    \param[out] none
    \retval     status
*/
static uint8_t explore_disk (char* path, uint8_t recu_level)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;

    uint8_t Str2[] = "Press B Key to continue";

    res = f_opendir(&dir, path);

    if (res == FR_OK) {
        while (usbh_msc_core.host.connect_status) {
            res = f_readdir(&dir, &fno);
            if (FR_OK != res || 0U == fno.fname[0]) {
                break;
            }

            if ('.' == fno.fname[0]) {
                continue;
            }

            fn = fno.fname;

            line_idx++;

            if (line_idx > 4) {
                line_idx = 0;

                lcd_log_print((uint8_t *)Str2, sizeof(Str2) - 1, LCD_COLOR_GREEN);

                /*key B3 in polling*/
                while ((usbh_msc_core.host.connect_status) && \
                     (SET == gd_eval_key_state_get (KEY_B))) {
                    toggle_leds();
                }
            } 

            if (1U == recu_level) {
                uint8_t temp[] = "   |__";
                lcd_log_print((uint8_t *)temp, strlen((const char *)temp), LCD_COLOR_WHITE);
            } else if(2U == recu_level) {
                uint8_t temp[] = "   |   |__";
                lcd_log_print((uint8_t *)temp, strlen((const char *)temp), LCD_COLOR_WHITE);
            }

            if (AM_DIR == (fno.fattrib & AM_MASK)) {
                lcd_log_print((uint8_t *)fno.fname, strlen(fno.fname), LCD_COLOR_RED);
            } else {
                lcd_log_print((uint8_t *)fno.fname, strlen(fno.fname), LCD_COLOR_WHITE);
            }

            if ((AM_DIR == (fno.fattrib & AM_MASK)) && (1U == recu_level)) {
                explore_disk(fn, 2);
            }
        }
    }

    return res;
}

/*!
    \brief      toggle leds to shows user input state
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void toggle_leds(void)
{
    static uint32_t i;

    if (0x10000U == i++) {
        gd_eval_led_toggle(LED2);
        gd_eval_led_toggle(LED3);
        gd_eval_led_toggle(LED4);
        i = 0;
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
    usbh_usr_application_state = USBH_USR_FS_INIT;
}
