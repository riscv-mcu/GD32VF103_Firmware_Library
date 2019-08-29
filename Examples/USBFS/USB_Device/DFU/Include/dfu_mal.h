/*!
    \file  usbd_dfu_mal.h
    \brief USB DFU device media access layer header file

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

#ifndef USBD_DFU_MAL_H
#define USBD_DFU_MAL_H

#include "usb_conf.h"
#include "usbd_conf.h"

typedef struct _DFU_MAL_PROPERTY
{
    const uint8_t* pStrDesc;
    uint8_t  (*pMAL_Init)      (void);
    uint8_t  (*pMAL_DeInit)    (void);
    uint8_t  (*pMAL_Erase)     (uint32_t Addr);
    uint8_t  (*pMAL_Write)     (uint32_t Addr, uint32_t Len);
    uint8_t* (*pMAL_Read)      (uint32_t Addr, uint32_t Len);
    uint8_t  (*pMAL_CheckAdd)  (uint32_t Addr);
    const uint32_t EraseTimeout;
    const uint32_t WriteTimeout;
}
DFU_MAL_Property_TypeDef;

typedef enum
{
    MAL_OK = 0,
    MAL_FAIL
} MAL_Status;

#define _1st_BYTE(x)              (uint8_t)((x) & 0xFF)               /*!< addressing cycle 1st byte */
#define _2nd_BYTE(x)              (uint8_t)(((x) & 0xFF00) >> 8)      /*!< addressing cycle 2nd byte */
#define _3rd_BYTE(x)              (uint8_t)(((x) & 0xFF0000) >> 16)   /*!< addressing cycle 3rd byte */

#define SET_POLLING_TIMEOUT(x)    buffer[1] = _1st_BYTE(x);\
                                  buffer[2] = _2nd_BYTE(x);\
                                  buffer[3] = _3rd_BYTE(x);

uint8_t  DFU_MAL_Init      (void);
uint8_t  DFU_MAL_DeInit    (void);
uint8_t  DFU_MAL_Erase     (uint32_t Addr);
uint8_t  DFU_MAL_Write     (uint32_t Addr, uint32_t Len);
uint8_t* DFU_MAL_Read      (uint32_t Addr, uint32_t Len);
uint8_t  DFU_MAL_GetStatus (uint32_t Addr, uint8_t Cmd, uint8_t *buffer);

extern uint8_t  MAL_Buffer[TRANSFER_SIZE];

#endif /* USBD_DFU_MAL_H */
