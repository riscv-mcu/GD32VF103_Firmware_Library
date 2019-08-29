/*!
    \file  iap_core.c
    \brief IAP driver

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

#include "iap_core.h"

#define USBD_VID                     0x28E9
#define USBD_PID                     0x028B

uint8_t report_buf[IAP_OUT_PACKET + 1];
uint8_t option_byte[IAP_IN_PACKET] = {0};

/* state machine variables */
uint8_t device_status[IAP_IN_PACKET] = {0};
uint8_t bin_address[IAP_IN_PACKET] = {0};

uint8_t usbd_customhid_report_id = 0;
uint8_t flag = 0;

static uint32_t usbd_customhid_altset = 0;
static uint32_t usbd_customhid_protocol = 0;
static uint32_t usbd_customhid_idlestate = 0;

static uint16_t transfer_times = 0;
static uint16_t page_count = 0;
static uint32_t file_length = 0;
static uint32_t base_address = APP_LOADED_ADDR;


/* Note:it should use the C99 standard when compiling the below codes */
/* USB standard device descriptor */
__ALIGN_BEGIN const usb_desc_dev device_descripter __ALIGN_END =
{
    .header = 
     {
         .bLength = USB_DEV_DESC_LEN, 
         .bDescriptorType = USB_DESCTYPE_DEV
     },
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = USB_FS_EP0_MAX_LEN,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = STR_IDX_MFC,
    .iProduct = STR_IDX_PRODUCT,
    .iSerialNumber = STR_IDX_SERIAL,
    .bNumberConfigurations = USBD_CFG_MAX_NUM
};

__ALIGN_BEGIN const usb_descriptor_configuration_set_struct configuration_descriptor __ALIGN_END = 
{
    .Config = 
    {
        .header = 
         {
             .bLength = USB_CFG_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_CONFIG 
         },
        .wTotalLength = IAP_CONFIG_SET_DESC_SIZE,
        .bNumInterfaces = 0x01,
        .bConfigurationValue = 0x01,
        .iConfiguration = 0x00,
        .bmAttributes = 0x80,
        .bMaxPower = 0x32
    },

    .HID_Interface = 
    {
        .header = 
         {
             .bLength = USB_ITF_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_ITF 
         },
        .bInterfaceNumber = 0x00,
        .bAlternateSetting = 0x00,
        .bNumEndpoints = 0x02,
        .bInterfaceClass = 0x03,
        .bInterfaceSubClass = 0x00,
        .bInterfaceProtocol = 0x00,
        .iInterface = 0x00
    },

    .HID_VendorHID = 
    {
        .header = 
         {
             .bLength = sizeof(usb_hid_descriptor_hid_struct), 
             .bDescriptorType = HID_DESCTYPE 
         },
        .bcdHID = 0x0111,
        .bCountryCode = 0x00,
        .bNumDescriptors = 0x01,
        .bDescriptorType = HID_REPORT_DESCTYPE,
        .wDescriptorLength = IAP_REPORT_DESC_SIZE,
    },

    .HID_ReportINEndpoint = 
    {
        .header = 
         {
             .bLength = USB_EP_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_EP 
         },
        .bEndpointAddress = IAP_IN_EP,
        .bmAttributes = 0x03,
        .wMaxPacketSize = IAP_IN_PACKET,
        .bInterval = 0x20
    },

    .HID_ReportOUTEndpoint = 
    {
        .header = 
         {
             .bLength = USB_EP_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_EP 
         },
        .bEndpointAddress = IAP_OUT_EP,
        .bmAttributes = 0x03,
        .wMaxPacketSize = IAP_OUT_PACKET,
        .bInterval = 0x20
    }
};

/* USB language ID Descriptor */
__ALIGN_BEGIN const usb_desc_LANGID usbd_language_id_desc __ALIGN_END = 
{
    .header = 
     {
         .bLength = sizeof(usb_desc_LANGID), 
         .bDescriptorType = USB_DESCTYPE_STR
     },
    .wLANGID = ENG_LANGID
};

/* USB serial string */
__ALIGN_BEGIN uint8_t usbd_serial_string[USB_SERIAL_STRING_SIZE] __ALIGN_END =
{
    USB_SERIAL_STRING_SIZE,       /* bLength */
    USB_DESCTYPE_STR,          /* bDescriptorType */
};

__ALIGN_BEGIN void* const usbd_strings[] __ALIGN_END = 
{
    [STR_IDX_LANGID] = (uint8_t *)&usbd_language_id_desc,
    [STR_IDX_MFC] = USBD_STRING_DESC("GigaDevice"),
    [STR_IDX_PRODUCT] = USBD_STRING_DESC("GD32 USB IAP in FS Mode"),
    [STR_IDX_SERIAL] = usbd_serial_string
};

/* USB custom HID device report descriptor */
__ALIGN_BEGIN const uint8_t iap_report_descriptor[IAP_REPORT_DESC_SIZE] __ALIGN_END =
{
    0x05, 0x01,     /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x00,     /* USAGE (Custom Device)        */
    0xa1, 0x01,     /* COLLECTION (Application)     */

    /* IAP command and data */
    0x85, 0x01,     /* REPORT_ID (0x01)          */
    0x09, 0x01,     /* USAGE (IAP command)       */
    0x15, 0x00,     /* LOGICAL_MINIMUM (0)       */
    0x25, 0xff,     /* LOGICAL_MAXIMUM (255)     */
    0x75, 0x08,     /* REPORT_SIZE (8)           */
    0x95, 0x3f,     /* REPORT_COUNT(63) */
    0x91, 0x82,     /* OUTPUT (Data,Var,Abs,Vol) */

    /* device status and option byte */  
    0x85, 0x02,     /* REPORT_ID (0x02)               */
    0x09, 0x02,     /* USAGE (Status and option byte) */
    0x15, 0x00,     /* LOGICAL_MINIMUM (0)            */
    0x25, 0xff,     /* LOGICAL_MAXIMUM (255)          */
    0x75, 0x08,     /* REPORT_SIZE (8)                */
    0x95, 0x17,     /* REPORT_COUNT (23)              */
    0x81, 0x82,     /* INPUT (Data,Var,Abs,Vol)       */

    0xc0            /* END_COLLECTION            */
};

/* IAP requests management functions */
static void  iap_req_erase     (usb_dev *pudev);
static void  iap_req_dnload    (usb_dev *pudev);
static void  iap_req_optionbyte(usb_dev *pudev, uint8_t option_ID);
static void  iap_req_leave     (usb_dev *pudev);
static void  iap_address_send  (usb_dev *pudev);

static void  iap_data_write (uint8_t *data, uint32_t addr, uint32_t len);

/*!
    \brief      initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
uint8_t iap_init (usb_dev *pudev, uint8_t config_index)
{
    /* initialize Tx endpoint */
    usbd_ep_setup(pudev, &(configuration_descriptor.HID_ReportINEndpoint));

    /* initialize Rx endpoint */
    usbd_ep_setup(pudev, &(configuration_descriptor.HID_ReportOUTEndpoint));

    /* unlock the internal flash */
    fmc_unlock();

    /* prepare receive Data */
    usbd_ep_recev(pudev, IAP_OUT_EP, report_buf, IAP_OUT_PACKET);

    return USBD_OK;
}

/*!
    \brief      de-initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  iap_deinit (usb_dev *pudev, uint8_t config_index)
{
    /* deinitialize HID endpoints */
    usbd_ep_clear (pudev, IAP_IN_EP);
    usbd_ep_clear (pudev, IAP_OUT_EP);

    /* lock the internal flash */
    fmc_lock();

    return USBD_OK;
}

/*!
    \brief      handle the HID class-specific requests
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
uint8_t iap_req_handler (usb_dev *pudev, usb_req *req)
{
    uint16_t len = 0;
    uint8_t *pbuf = NULL;
    uint8_t USBD_CUSTOMHID_Report_LENGTH = 0;

    switch (req->bmRequestType & USB_REQTYPE_MASK) {
    case USB_REQTYPE_CLASS:
        switch (req->bRequest) {
        case GET_REPORT:
            /* no use for this driver */
            break;
        case GET_IDLE:
            pudev->dev.transc_in[0].xfer_buf = (uint8_t *)&usbd_customhid_idlestate;
            pudev->dev.transc_in[0].remain_len = 1;
            break;
        case GET_PROTOCOL:
            pudev->dev.transc_in[0].xfer_buf = (uint8_t *)&usbd_customhid_protocol;
            pudev->dev.transc_in[0].remain_len = 1;
            break;
        case SET_REPORT:
            flag = 1;
            usbd_customhid_report_id = (uint8_t)(req->wValue);
            USBD_CUSTOMHID_Report_LENGTH = (uint8_t)(req->wLength);
            pudev->dev.transc_out[0].xfer_buf = report_buf;
            pudev->dev.transc_out[0].remain_len = USBD_CUSTOMHID_Report_LENGTH;
            break;
        case SET_IDLE:
            usbd_customhid_idlestate = (uint8_t)(req->wValue >> 8);
            break;
        case SET_PROTOCOL:
            usbd_customhid_protocol = (uint8_t)(req->wValue);
            break;
        default:
            usbd_enum_error (pudev, req);
            return USBD_FAIL; 
            }
            break;
    case USB_REQTYPE_STRD:
        /* standard device request */
        switch(req->bRequest) {
        case USB_GET_DESCRIPTOR:
            switch(req->wValue >> 8) {
            case HID_REPORT_DESCTYPE:
                len = USB_MIN(IAP_REPORT_DESC_SIZE, req->wLength);
                pbuf = (uint8_t *)iap_report_descriptor;
                break;
            case HID_DESCTYPE:
                len = USB_MIN(IAP_CONFIG_DESC_SIZE, req->wLength);
                pbuf = (uint8_t *)(&(configuration_descriptor.HID_VendorHID));
                break;
            default:
                break;
            }
            pudev->dev.transc_in[0].xfer_buf = pbuf;
            pudev->dev.transc_in[0].remain_len = len;
            break;
        case USB_GET_INTERFACE:
            pudev->dev.transc_in[0].xfer_buf = (uint8_t *)&usbd_customhid_altset;
            pudev->dev.transc_in[0].remain_len = 1;
            break;
        case USB_SET_INTERFACE:
            usbd_customhid_altset = (uint8_t)(req->wValue);
            break;
        default:
            break;
        }
        break;
    }

    return USBD_OK;
}

/*!
    \brief      handle data stage
    \param[in]  pudev: pointer to USB device instance
    \param[in]  rx_tx: data transfer direction:
      \arg        USBD_TX
      \arg        USBD_RX
    \param[in]  ep_id: endpoint identifier
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  iap_data_in_handler (usb_dev *pudev, uint8_t ep_id)
{
    if((IAP_IN_EP & 0x7F) == ep_id) {
        return USBD_OK;
    } 
    return USBD_FAIL;
}

uint8_t  iap_data_out_handler (usb_dev *pudev, uint8_t ep_id)
{
    if (IAP_OUT_EP == ep_id) {
        switch (report_buf[0]) {
        case 0x01:
            switch(report_buf[1]) {
            case IAP_DNLOAD:
                iap_req_dnload(pudev);
                break;
            case IAP_ERASE:
                iap_req_erase(pudev);
                break;
            case IAP_OPTION_BYTE1:
                iap_req_optionbyte(pudev, 0x01);
                break;
            case IAP_LEAVE:
                iap_req_leave(pudev);
                break;
            case IAP_GETBIN_ADDRESS:
                iap_address_send(pudev);
                break;
            case IAP_OPTION_BYTE2:
                iap_req_optionbyte(pudev, 0x02);
                break;
            default:
                usbd_enum_error(pudev, NULL);
                return USBD_FAIL;
            }
            break;
        default:
            break;
        }

        usbd_ep_recev(pudev, IAP_OUT_EP, report_buf, IAP_OUT_PACKET);

        return USBD_OK;
    }

    return USBD_FAIL;
}

/*!
    \brief      send iap report
    \param[in]  pudev: pointer to USB device instance
    \param[in]  report: pointer to HID report
    \param[in]  len: data length
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  iap_report_send (usb_dev *pudev, uint8_t *report, uint16_t len)
{
    usbd_ep_send (pudev, IAP_IN_EP, report, len);

    return USBD_OK;
}

/*!
    \brief      handle the IAP_DNLOAD request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void iap_req_dnload(usb_dev *pudev)
{
    if (0 != transfer_times) {
        if (1 == transfer_times) {
            iap_data_write(&report_buf[2], base_address, file_length % TRANSFER_SIZE);

            device_status[0] = 0x02;
            device_status[1] = 0x02;
            iap_report_send (pudev, device_status, IAP_IN_PACKET);
        } else {
            iap_data_write(&report_buf[2], base_address, TRANSFER_SIZE);

            base_address += TRANSFER_SIZE;
        }

        transfer_times --;
    }
}

/*!
    \brief      handle the IAP_ERASE request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void iap_req_erase(usb_dev *pudev)
{
    uint32_t i = 0, addr = 0;

    /* get base address to erase */
    base_address  = report_buf[2];
    base_address |= report_buf[3] << 8;
    base_address |= report_buf[4] << 16;
    base_address |= report_buf[5] << 24;

    page_count = report_buf[6];

    /* get file length */
    file_length = report_buf[7];
    file_length |= report_buf[8] << 8;
    file_length |= report_buf[9] << 16;
    file_length |= report_buf[10] << 24;

    transfer_times = file_length / TRANSFER_SIZE + 1;

    /* check if the address is in protected area */
    if (IS_PROTECTED_AREA(base_address)) {
        return;
    }

    addr = base_address;

    /* unlock the flash program erase controller */
    fmc_unlock();

    for (i = 0; i < page_count; i ++) {
        /* call the standard flash erase-page function */
        fmc_page_erase(addr);

        addr += PAGE_SIZE;
    }

    fmc_lock();

    device_status[0] = 0x02;
    device_status[1] = 0x01;

    usbd_ep_send(pudev, IAP_IN_EP, device_status, IAP_IN_PACKET);
}

/*!
    \brief      handle the IAP_OPTION_BYTE request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  iap_req_optionbyte(usb_dev *pudev, uint8_t option_ID)
{
    uint8_t i = 0;
    uint32_t address = 0;

    option_byte[0] = 0x02;

    if (option_ID == 0x01) {
        address = 0x1FFFF800;
    } else {
        return;
    }

    for (i = 1; i < 17; i++) {
        option_byte[i] = *(uint8_t *)address;
        address++;
    }

    iap_report_send (pudev, option_byte, IAP_IN_PACKET);
}

/*!
    \brief      handle the IAP_LEAVE request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  iap_req_leave(usb_dev *pudev)
{
    /* lock the internal flash */
    fmc_lock();

    /* generate system reset to allow jumping to the user code */
    eclic_system_reset();
}

/*!
    \brief      handle the IAP_SEND_ADDRESS request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  iap_address_send(usb_dev *pudev)
{
    bin_address[0] = 0x02;

    bin_address[1] = (uint8_t)(APP_LOADED_ADDR);
    bin_address[2] = (uint8_t)(APP_LOADED_ADDR >> 8);
    bin_address[3] = (uint8_t)(APP_LOADED_ADDR >> 16);
    bin_address[4] = (uint8_t)(APP_LOADED_ADDR >> 24);

    iap_report_send (pudev, bin_address, IAP_IN_PACKET);
}

/*!
    \brief      write data to sectors of memory
    \param[in]  data: data to be written
    \param[in]  addr: sector address/code
    \param[in]  len: length of data to be written (in bytes)
    \param[out] none
    \retval     MAL_OK if all operations are OK, MAL_FAIL else
*/
static void  iap_data_write (uint8_t *data, uint32_t addr, uint32_t len)
{
    uint32_t idx = 0;

    /* check if the address is in protected area */
    if (IS_PROTECTED_AREA(addr)) {
        return;
    }

   /* unlock the flash program erase controller */
    fmc_unlock();

    /* data received are word multiple */
    for (idx = 0; idx < len; idx += 2) {
        if (FMC_READY == fmc_halfword_program(addr, *(uint16_t *)(data + idx))) {
            addr += 2;
        } else {
            while(1);
        }
    }

    fmc_lock();
}

usb_class_core usbd_hid_cb = {
    .command         = NO_CMD,
    .alter_set       = 0,

    .init            = iap_init,
    .deinit          = iap_deinit,
    .req_proc        = iap_req_handler,
    .data_in         = iap_data_in_handler,
    .data_out        = iap_data_out_handler
};
