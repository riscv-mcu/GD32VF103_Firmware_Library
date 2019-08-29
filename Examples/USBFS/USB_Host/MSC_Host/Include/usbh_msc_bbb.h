/*!
    \file  usbh_msc_bbb.h
    \brief header file for usbh_msc_bbb.c

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

#ifndef __USBH_MSC_BBB_H
#define __USBH_MSC_BBB_H

#include "usbh_enum.h"
#include "msc_bbb.h"

typedef union {
    msc_bbb_cbw field;

    uint8_t CBWArray[31];
}usbh_cbw_pkt;

typedef union {
    msc_bbb_csw field;

    uint8_t CSWArray[13];
}usbh_csw_pkt;

enum usbh_msc_state {
    USBH_MSC_BOT_INIT_STATE = 0,
    USBH_MSC_BOT_RESET,
    USBH_MSC_GET_MAX_LUN,
    USBH_MSC_TEST_UNIT_READY,
    USBH_MSC_READ_CAPACITY10,
    USBH_MSC_MODE_SENSE6,
    USBH_MSC_REQUEST_SENSE,
    USBH_MSC_BOT_USB_TRANSFERS,
    USBH_MSC_DEFAULT_APPLI_STATE,
    USBH_MSC_CTRL_ERROR_STATE,
    USBH_MSC_UNRECOVERED_STATE
};

enum msc_bot_state {
    USBH_MSC_SEND_CBW = 1U,
    USBH_MSC_SENT_CBW,
    USBH_MSC_BOT_DATAIN_STATE,
    USBH_MSC_BOT_DATAOUT_STATE,
    USBH_MSC_RECEIVE_CSW_STATE,
    USBH_MSC_DECODE_CSW,
    USBH_MSC_BOT_ERROR_IN,
    USBH_MSC_BOT_ERROR_OUT
};

typedef struct {
    uint8_t msc_state;
    uint8_t msc_state_bkp;
    uint8_t msc_state_current;
    uint8_t cmd_state_machine;
    uint8_t bot_state;
    uint8_t bot_state_bkp;

    uint8_t *xfer_buf;
    uint16_t data_len;

    uint8_t bot_xfer_status;
} usbh_botxfer;


#define USBH_MSC_BOT_CBW_TAG                0x20304050

#define USBH_MSC_CSW_MAX_LENGTH             63

#define USBH_MSC_SEND_CSW_DISABLE           0
#define USBH_MSC_SEND_CSW_ENABLE            1

#define USBH_MSC_DIR_IN                     0
#define USBH_MSC_DIR_OUT                    1
#define USBH_MSC_BOTH_DIR                   2

#define USBH_MSC_PAGE_LENGTH                512

#define CBW_CB_LENGTH                       16
#define CBW_LENGTH                          10
#define CBW_LENGTH_TEST_UNIT_READY          6

#define MAX_BULK_STALL_COUNT_LIMIT          0x04   /* If STALL is seen on Bulk 
                                                      Endpoint continously, this means 
                                                      that device and Host has phase error
                                                      Hence a Reset is needed */

extern usbh_botxfer msc_botxfer_param;
extern usbh_cbw_pkt msc_cbw_data;
extern usbh_csw_pkt msc_csw_data;

void usbh_msc_init (usb_core_driver *pudev);

void usbh_msc_botxfer (usb_core_driver *pudev, usbh_host *puhost);

uint8_t usbh_msc_csw_decode (usb_core_driver *pudev, usbh_host *puhost);

usbh_status usbh_msc_bot_abort (usb_core_driver *pudev, usbh_host *puhost, uint8_t direction);

#endif  /* __USBH_MSC_BOT_H */
