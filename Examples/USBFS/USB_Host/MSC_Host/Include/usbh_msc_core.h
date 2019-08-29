/*!
    \file  usbh_msc_core.h
    \brief header file for the usbh_msc_core.c

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

#ifndef __USBH_MSC_CORE_H
#define __USBH_MSC_CORE_H

#include "usb_msc.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bbb.h"

/* structure for msc process */
typedef struct {
    uint8_t         hc_num_in;
    uint8_t         hc_num_out;
    uint8_t         msc_bulk_epin;
    uint8_t         msc_bulk_epout;
    uint16_t        msc_bulk_epinsize;
    uint16_t        msc_bulk_epoutsize;
    uint8_t         buf[USBH_MSC_MPS_SIZE];
    uint8_t         max_lun;
} usbh_msc_machine;

extern usbh_class_cb usbh_msc_cb;
extern usbh_msc_machine msc_machine;
extern uint8_t msc_error_count;

#endif  /* __USBH_MSC_CORE_H */
