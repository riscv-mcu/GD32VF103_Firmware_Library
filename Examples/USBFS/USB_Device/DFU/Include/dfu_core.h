/*!
    \file  usbd_hid_core.h
    \brief the header file of USB DFU device class core functions

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

#ifndef DFU_CORE_H
#define DFU_CORE_H

#include "usbd_enum.h"
#include "usb_ch9_std.h"
#include "usbd_transc.h"

/* manifestation State */
#define MANIFEST_COMPLETE       0x00
#define MANIFEST_IN_PROGRESS    0x01

#define USB_DFU_DESC_SIZE       9

/* special commands with download request */
#define GET_COMMANDS            0x00
#define SET_ADDRESS_POINTER     0x21
#define ERASE                   0x41
#define NO_CMD                  0xFF

/* memory operation command */
#define CMD_ERASE               0
#define CMD_WRITE               1

#define FLASH_ERASE_TIMEOUT     60
#define FLASH_WRITE_TIMEOUT     80

/* bit detach capable = bit 3 in bmAttributes field */
#define DFU_DETACH_MASK               (uint8_t)(0x10)

#define USB_SERIAL_STRING_SIZE        0x06
#define DEVICE_ID                     (0x40022100)

#define USB_DFU_CONFIG_DESC_SIZE      (18 + (9 * USBD_ITF_MAX_NUM))
#define DFU_DESC_TYPE                 0x21
#define FLASH_IF_STRING               "@Internal Flash   /0x08000000/16*002Ka,112*002Kg"

/* DFU device state defines */
typedef enum
{
    STATE_appIDLE = 0x00,
    STATE_appDETACH,
    STATE_dfuIDLE,
    STATE_dfuDNLOAD_SYNC,
    STATE_dfuDNBUSY,
    STATE_dfuDNLOAD_IDLE,
    STATE_dfuMANIFEST_SYNC,
    STATE_dfuMANIFEST,
    STATE_dfuMANIFEST_WAIT_RESET,
    STATE_dfuUPLOAD_IDLE,
    STATE_dfuERROR
}dfu_state_enum;

/* DFU device status defines */
typedef enum
{
    STATUS_OK = 0x00,
    STATUS_errTARGET,
    STATUS_errFILE,
    STATUS_errWRITE,
    STATUS_errERASE,
    STATUS_errCHECK_ERASED,
    STATUS_errPROG,
    STATUS_errVERIFY,
    STATUS_errADDRESS,
    STATUS_errNOTDONE,
    STATUS_errFIRMWARE,
    STATUS_errVENDOR,
    STATUS_errUSBR,
    STATUS_errPOR,
    STATUS_errUNKNOWN,
    STATUS_errSTALLEDPKT
}dfu_status_enum;

/* DFU class-specific requests */
typedef enum
{
    DFU_DETACH = 0,
    DFU_DNLOAD,
    DFU_UPLOAD,
    DFU_GETSTATUS,
    DFU_CLRSTATUS,
    DFU_GETSTATE,
    DFU_ABORT,
    DFU_REQ_MAX
}dfu_requests_enum;

#pragma pack(1)

/* USB dfu function descriptor struct */
typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */
    uint8_t bmAttributes;                 /*!< DFU attributes */
    uint16_t wDetachTimeOut;              /*!< time, in milliseconds, that the device will wait after receipt of the DFU_DETACH request. If */ 
    uint16_t wTransferSize;               /*!< maximum number of bytes that the device can accept per control-write transaction */
    uint16_t bcdDFUVersion;               /*!< numeric expression identifying the version of the DFU Specification release. */
} usb_dfu_function_descriptor_struct;

#pragma pack()

/* USB configuration descriptor struct */
typedef struct
{
    usb_desc_config                            Config;
    usb_desc_itf                               DFU_Interface;
    usb_dfu_function_descriptor_struct         DFU_Function_Desc;
} usb_descriptor_configuration_set_struct;

typedef  void  (*pAppFunction) (void);

extern void* const usbd_strings[USB_STRING_COUNT];
extern const usb_desc_dev device_descripter;
extern const usb_descriptor_configuration_set_struct configuration_descriptor;

extern usb_class_core usbd_dfu_cb;

/* function declarations */
/* initialize the MSC device */
uint8_t dfu_init (usb_dev *pudev, uint8_t config_index);
/* de-initialize the MSC device */
uint8_t dfu_deinit (usb_dev *pudev, uint8_t config_index);
/* handle the MSC class-specific requests */
uint8_t dfu_req_handler (usb_dev *pudev, usb_req *req);
/* handle data Stage */
uint8_t dfu_data_in_handler (usb_dev *pudev, uint8_t ep_num);
uint8_t dfu_data_out_handler (usb_dev *pudev, uint8_t ep_num);

#endif  /* DFU_CORE_H */
