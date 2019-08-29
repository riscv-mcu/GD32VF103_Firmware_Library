/*!
    \file  usbh_hid_keybd.h
    \brief this file contains all the prototypes for the usbh_hid_keybd.c

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

#ifndef USBH_HID_KEYBD_H
#define USBH_HID_KEYBD_H

#include "usb_conf.h"
#include "usbh_hid_core.h"

//#define AZERTY_KEYBOARD
#define QWERTY_KEYBOARD

#define KBD_LEFT_CTRL              0x01U
#define KBD_LEFT_SHIFT             0x02U
#define KBD_LEFT_ALT               0x04U
#define KBD_LEFT_GUI               0x08U
#define KBD_RIGHT_CTRL             0x10U
#define KBD_RIGHT_SHIFT            0x20U
#define KBD_RIGHT_ALT              0x40U
#define KBD_RIGHT_GUI              0x80U

#define KBR_MAX_NBR_PRESSED        6U

extern hid_proc HID_KEYBRD_cb;

/* function declarations */
/* init keyboard window */
void  usr_keybrd_init (void);
/* process keyboard data */
void  usr_keybrd_process_data (uint8_t pbuf);

#endif /* USBH_HID_KEYBD_H */
