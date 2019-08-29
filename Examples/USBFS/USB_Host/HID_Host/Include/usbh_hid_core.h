/*!
    \file  usbh_hid_core.h
    \brief header file for usbh_hid_core.c

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

#ifndef __USBH_HID_CORE_H
#define __USBH_HID_CORE_H

#include "usb_hid.h"
#include "usbh_enum.h"
#include "usbh_transc.h"

#define HID_MIN_POLL                                    10U

#define USB_HID_DESC_SIZE                               9U

/* States for HID State Machine */
typedef enum {
    HID_IDLE = 0U,
    HID_SEND_DATA,
    HID_BUSY,
    HID_GET_DATA,
    HID_SYNC,
    HID_POLL,
    HID_ERROR,
}hid_state;

typedef enum {
    HID_REQ_IDLE = 0U,
    HID_REQ_GET_REPORT_DESC,
    HID_REQ_GET_HID_DESC,
    HID_REQ_SET_IDLE,
    HID_REQ_SET_PROTOCOL,
    HID_REQ_SET_REPORT,
}hid_ctlstate;

typedef struct
{
    void (*Init)   (void);
    void (*Decode) (uint8_t *data);
}hid_proc;

typedef struct
{
    uint8_t  ReportID;
    uint8_t  ReportType;
    uint16_t UsagePage;
    uint32_t Usage[2];
    uint32_t NbrUsage;
    uint32_t UsageMin;
    uint32_t UsageMax;
    int32_t  LogMin;
    int32_t  LogMax;
    int32_t  PhyMin;
    int32_t  PhyMax;
    int32_t  UnitExp;
    uint32_t Unit;
    uint32_t ReportSize;
    uint32_t ReportCnt;
    uint32_t Flag;
    uint32_t PhyUsage;
    uint32_t AppUsage;
    uint32_t LogUsage;
}usb_desc_report;

/* structure for HID process */
typedef struct
{
    uint8_t              buf[64];

    uint8_t              pipe_in;
    uint8_t              pipe_out;

    uint8_t              ep_addr;
    uint8_t              ep_out;
    uint8_t              ep_in;

    uint16_t             len;
    uint16_t             poll;

    __IO uint16_t        timer;

    hid_state            state; 
    hid_ctlstate         ctlstate;
    hid_proc            *proc;
}hid_machine_struct;

extern usbh_class_cb usbh_hid_cb;

usbh_status usbh_set_report (usb_core_driver *pudev,
                             usbh_host *phost,
                             uint8_t  report_type,
                             uint8_t  report_id,
                             uint8_t  report_len,
                             uint8_t *report_buff);

#endif /* __USBH_HID_CORE_H */
