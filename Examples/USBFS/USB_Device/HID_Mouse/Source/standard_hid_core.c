/*!
    \file  standard_hid_core.c
    \brief HID class driver

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

#include "standard_hid_core.h"

#define USBD_VID                     0x28e9
#define USBD_PID                     0x0380

static uint32_t usbd_hid_protocol = 0;
static uint32_t usbd_hid_idlestate  = 0;
__IO uint8_t prev_transfer_complete = 1;

/* Note:it should use the C99 standard when compiling the below codes */
/* USB standard device descriptor */
const usb_desc_dev hid_dev_desc =
{
    .header = 
     {
         .bLength          = USB_DEV_DESC_LEN, 
         .bDescriptorType  = USB_DESCTYPE_DEV
     },
    .bcdUSB                = 0x0200U,
    .bDeviceClass          = 0x00U,
    .bDeviceSubClass       = 0x00U,
    .bDeviceProtocol       = 0x00U,
    .bMaxPacketSize0       = USB_FS_EP0_MAX_LEN,
    .idVendor              = USBD_VID,
    .idProduct             = USBD_PID,
    .bcdDevice             = 0x0100U,
    .iManufacturer         = STR_IDX_MFC,
    .iProduct              = STR_IDX_PRODUCT,
    .iSerialNumber         = STR_IDX_SERIAL,
    .bNumberConfigurations = USBD_CFG_MAX_NUM
};

const usb_hid_desc_config_set hid_config_desc = 
{
    .config = 
    {
        .header = 
         {
             .bLength         = USB_CFG_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_CONFIG 
         },
        .wTotalLength         = USB_HID_CONFIG_DESC_LEN,
        .bNumInterfaces       = 0x01U,
        .bConfigurationValue  = 0x01U,
        .iConfiguration       = 0x00U,
        .bmAttributes         = 0xA0U,
        .bMaxPower            = 0x32U
    },

    .hid_itf = 
    {
        .header = 
         {
             .bLength         = USB_ITF_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber     = 0x00U,
        .bAlternateSetting    = 0x00U,
        .bNumEndpoints        = 0x01U,
        .bInterfaceClass      = USB_HID_CLASS,
        .bInterfaceSubClass   = USB_HID_SUBCLASS_BOOT_ITF,
        .bInterfaceProtocol   = USB_HID_PROTOCOL_MOUSE,
        .iInterface           = 0x00U
    },

    .hid_vendor = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_hid), 
             .bDescriptorType = USB_DESCTYPE_HID 
         },
        .bcdHID               = 0x0111U,
        .bCountryCode         = 0x00U,
        .bNumDescriptors      = 0x01U,
        .bDescriptorType      = USB_DESCTYPE_REPORT,
        .wDescriptorLength    = USB_HID_REPORT_DESC_LEN,
    },

    .hid_epin = 
    {
        .header = 
         {
             .bLength         = USB_EP_DESC_LEN,
             .bDescriptorType = USB_DESCTYPE_EP
         },
        .bEndpointAddress     = HID_IN_EP,
        .bmAttributes         = USB_EP_ATTR_INT,
        .wMaxPacketSize       = HID_IN_PACKET,
        .bInterval            = 0x01U,
    }
};

/* USB language ID Descriptor */
const usb_desc_LANGID usbd_language_id_desc = 
{
    .header = 
     {
         .bLength         = sizeof(usb_desc_LANGID), 
         .bDescriptorType = USB_DESCTYPE_STR
     },
    .wLANGID              = ENG_LANGID
};

void *const usbd_hid_strings[] = 
{
    [STR_IDX_LANGID]  = (uint8_t *)&usbd_language_id_desc,
    [STR_IDX_MFC]     = USBD_STRING_DESC("GigaDevice"),
    [STR_IDX_PRODUCT] = USBD_STRING_DESC("GD32 USB Mouse in FS Mode"),
    [STR_IDX_SERIAL]  = USBD_STRING_DESC("GD32VF103-V1.0.0-3a4b5ed")
};

const uint8_t hid_report_desc[USB_HID_REPORT_DESC_LEN] =
{
    0x05, 0x01,  /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x02,  /* USAGE (Mouse) */
    0xa1, 0x01,  /* COLLECTION (Application) */
    0x09, 0x01,  /* USAGE (Pointer) */

    0xa1, 0x00,  /* COLLECTION (Physical) */
    0x05, 0x09,  /* USAGE_PAGE (Button) */
    0x19, 0x01,  /* USAGE_MINIMUM (1) */
    0x29, 0x03,  /* USAGE_MAXIMUM (3) */

    0x15, 0x00,  /* LOGICAL_MINIMUM (0) */
    0x25, 0x01,  /* LOGICAL_MAXIMUM (1) */
    0x95, 0x03,  /* REPORT_COUNT (3) */
    0x75, 0x01,  /* REPORT_SIZE (1) */
    0x81, 0x02,  /* INPUT (Data,Var,Abs) */

    0x95, 0x01,  /* REPORT_COUNT (1) */
    0x75, 0x05,  /* REPORT_SIZE (5) */
    0x81, 0x01,  /* INPUT (Cnst,Var,Abs) */

    0x05, 0x01,  /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x30,  /* USAGE (X) */
    0x09, 0x31,  /* USAGE (Y) */
    0x09, 0x38,  /* USAGE (Wheel) */

    0x15, 0x81,  /* LOGICAL_MINIMUM (81) */
    0x25, 0x7F,  /* LOGICAL_MAXIMUM (7F) */
    0x75, 0x08,  /* REPORT_SIZE (8) */
    0x95, 0x03,  /* REPORT_COUNT (3) */
    0x81, 0x06,  /* INPUT (Data,Ary,Abs) */
    0xc0,        /* END_COLLECTION */

    0x09, 0x3c,  /* USAGE (Motion Wakeup) */
    0x05, 0xff,  /* USAGE PAGE (vendor defined) */
    0x09, 0x01,  /* USAGE(01) */
    0x15, 0x00,  /* LOGICAL_MINIMUM (0) */
    0x25, 0x01,  /* LOGICAL_MAXIMUM (1) */
    0x75, 0x01,  /* REPORT_SIZE (1) */
    0x95, 0x02,  /* REPORT_COUNT (2) */
    0xb1, 0x22,  /* Feature (var) */
    0x75, 0x06,  /* REPORT_SIZE (6) */
    0x95, 0x01,  /* REPORT_COUNT (1) */
    0xb1, 0x01,  /* Feature (cnst) */
    0xc0
};

/* local function prototypes ('static') */
static uint8_t hid_init    (usb_dev *udev, uint8_t config_index);
static uint8_t hid_deinit  (usb_dev *udev, uint8_t config_index);
static uint8_t hid_req     (usb_dev *udev, usb_req *req);
static uint8_t hid_data_in (usb_dev *udev, uint8_t ep_num);

usb_class_core usbd_hid_cb = {
    .command         = NO_CMD,
    .alter_set       = 0,

    .init            = hid_init,
    .deinit          = hid_deinit,
    .req_proc        = hid_req,
    .data_in         = hid_data_in
};


/*!
    \brief      initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t hid_init (usb_dev *udev, uint8_t config_index)
{
    /* Initialize the data Tx endpoint */
    usbd_ep_setup (udev, &(hid_config_desc.hid_epin));

    return USBD_OK;
}

/*!
    \brief      de-initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t hid_deinit (usb_dev *pudev, uint8_t config_index)
{
    /* deinitialize HID endpoints */
    usbd_ep_clear(pudev, HID_IN_EP);

    return USBD_OK;
}

/*!
    \brief      handle the HID class-specific requests
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t hid_req (usb_dev *pudev, usb_req *req)
{
    usb_transc *transc = &pudev->dev.transc_in[0];

    switch (req->bRequest) {
        case GET_REPORT:
            /* no use for this driver */
            break;

        case GET_IDLE:
            transc->xfer_buf = (uint8_t *)&usbd_hid_idlestate;
            transc->remain_len = 1;
            break;

        case GET_PROTOCOL:
            transc->xfer_buf = (uint8_t *)&usbd_hid_protocol;
            transc->remain_len = 1;
            break;

        case SET_REPORT:
            /* no use for this driver */
            break;

        case SET_IDLE:
            usbd_hid_idlestate = (uint8_t)(req->wValue >> 8);
            break;

        case SET_PROTOCOL:
            usbd_hid_protocol = (uint8_t)(req->wValue);
            break;

        case USB_GET_DESCRIPTOR:
            if (USB_DESCTYPE_REPORT == (req->wValue >> 8)) {
                transc->remain_len = USB_MIN(USB_HID_REPORT_DESC_LEN, req->wLength);
                transc->xfer_buf = (uint8_t *)hid_report_desc;
                return REQ_SUPP;
            }
            break;

        default:
            break;
    }

    return USBD_OK;
}

/*!
    \brief      handle data stage
    \param[in]  pudev: pointer to USB device instance
    \param[in]  ep_num: endpoint identifier
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t hid_data_in (usb_dev *pudev, uint8_t ep_num)
{
    prev_transfer_complete = 1;
    return USBD_OK;
}

/*!
    \brief      send keyboard report
    \param[in]  pudev: pointer to USB device instance
    \param[in]  report: pointer to HID report
    \param[in]  len: data length
    \param[out] none
    \retval     USB device operation status
*/
uint8_t hid_report_send (usb_dev *pudev, uint8_t *report, uint16_t len)
{
    if (pudev->dev.cur_status == USBD_CONFIGURED) {
        if(1 == prev_transfer_complete){
            prev_transfer_complete = 0;
            usbd_ep_send(pudev, HID_IN_EP, report, len);
        }
    }

    return USBD_OK;
}
