/*!
    \file  usbh_msc_scsi.h
    \brief header file for usbh_msc_scsi.c

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

#ifndef __USBH_MSC_SCSI_H
#define __USBH_MSC_SCSI_H

#include "msc_scsi.h"
#include "usbh_enum.h"

typedef enum
{
    USBH_MSC_OK          = 0U,
    USBH_MSC_FAIL        = 1U,
    USBH_MSC_PHASE_ERROR = 2U,
    USBH_MSC_BUSY        = 3U
}usbh_msc_status;

enum cmd_states {
    CMD_UNINITIALIZED_STATE = 0,
    CMD_SEND_STATE,
    CMD_WAIT_STATUS
};

typedef struct
{
    uint32_t msc_capacity;
    uint32_t msc_sense_key;
    uint16_t msc_page_len;
    uint8_t msc_write_protect;
}usbh_msc_parameter;

#define DESC_REQUEST_SENSE                0x00U
#define ALLOCATION_LENGTH_REQUEST_SENSE   63U
#define XFER_LEN_MODE_SENSE6              63U

#define MASK_MODE_SENSE_WRITE_PROTECT     0x80U
#define MODE_SENSE_PAGE_CONTROL_FIELD     0x00U
#define MODE_SENSE_PAGE_CODE              0x3FU
#define DISK_WRITE_PROTECTED              0x01U

extern usbh_msc_parameter usbh_msc_param;

uint8_t usbh_msc_test_unitready  (usb_core_driver *pudev);
uint8_t usbh_msc_read_capacity10 (usb_core_driver *pudev);
uint8_t usbh_msc_mode_sense6     (usb_core_driver *pudev);
uint8_t usbh_msc_request_sense   (usb_core_driver *pudev);

uint8_t usbh_msc_write10 (usb_core_driver *pudev,
                          uint8_t *data_buf,
                          uint32_t addr,
                          uint32_t byte_num);

uint8_t usbh_msc_read10 (usb_core_driver *pudev,
                         uint8_t *data_buf,
                         uint32_t addr,
                         uint32_t byte_num);

void usbh_msc_state_machine (usb_core_driver *pudev);

#endif  /* __USBH_MSC_SCSI_H */
