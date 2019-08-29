/*!
    \file  usbh_hid_core.c
    \brief this file implements the functions for the HID core state process

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

#include "usbh_pipe.h"
#include "drv_usb_hw.h"
#include "usbh_hid_core.h"
#include "usbh_hid_mouse.h"
#include "usbh_hid_keybd.h"
#include <stdio.h>

hid_machine_struct hid_machine;

usb_desc_hid hid_desc;
usb_desc_report hid_report;

__IO uint8_t start_toggle = 0;

static void usbh_hiddesc_parse (usb_desc_hid *hid_desc, uint8_t *buf);

static void usbh_hid_itf_deinit (usb_core_driver *pudev, void *phost);

static usbh_status usbh_hid_itf_init (usb_core_driver *pudev, void *phost);

static usbh_status usbh_hid_handle (usb_core_driver *pudev, void *phost);

static usbh_status usbh_hid_class_req (usb_core_driver *pudev, void *phost);

static usbh_status usbh_hid_reportdesc_get (usb_core_driver *pudev, 
                                            usbh_host *phost, 
                                            uint16_t Len);

static usbh_status usbh_hid_desc_get (usb_core_driver *pudev, usbh_host *phost, uint16_t Len);

static usbh_status usbh_set_idle (usb_core_driver *pudev, 
                                  usbh_host *phost, 
                                  uint8_t Duration,
                                  uint8_t ReportId);

static usbh_status usbh_set_protocol (usb_core_driver *pudev, 
                                      usbh_host *phost,
                                      uint8_t    Protocol);

usbh_class_cb usbh_hid_cb = 
{
    usbh_hid_itf_init,
    usbh_hid_itf_deinit,
    usbh_hid_class_req,
    usbh_hid_handle
};

/*!
    \brief      initialize the hid class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_hid_itf_init (usb_core_driver *pudev, void *phost)
{
    uint8_t num = 0;
    uint8_t ep_num = 0;

    usbh_host *pphost = phost;

    usbh_status status = USBH_BUSY;

    usb_desc_itf *itf_desc = &pphost->dev_prop.itf_desc[0];

    hid_machine.state = HID_ERROR;

    if (itf_desc->bInterfaceSubClass == USB_HID_SUBCLASS_BOOT_ITF) {
        /* decode bootclass protocl: mouse or keyboard */
        if (itf_desc->bInterfaceProtocol == USB_HID_PROTOCOL_KEYBOARD) {
            hid_machine.proc = &HID_KEYBRD_cb;
        } else if (itf_desc->bInterfaceProtocol == USB_HID_PROTOCOL_MOUSE) {
            hid_machine.proc = &hid_mouse_cb;
        } else {
            return status;
        }

        hid_machine.state = HID_IDLE;
        hid_machine.ctlstate = HID_REQ_IDLE;

        hid_machine.poll = pphost->dev_prop.ep_desc[0][0].bInterval;

        if (hid_machine.poll < HID_MIN_POLL) {
            hid_machine.poll = HID_MIN_POLL;
        }

        /* check fo available number of endpoints */
        /* find the number of endpoints in the interface descriptor */
        /* choose the lower number in order not to overrun the buffer allocated */
        ep_num = USB_MIN(itf_desc->bNumEndpoints, USBH_MAX_EP_NUM);

        /* decode endpoint IN and OUT address from interface descriptor */
        for (num = 0; num < ep_num; num++) {
            usb_desc_ep *ep_desc = &pphost->dev_prop.ep_desc[0][num];

            uint8_t ep_addr = ep_desc->bEndpointAddress;

            hid_machine.len = ep_desc->wMaxPacketSize;

            if (ep_addr & 0x80) {
                hid_machine.ep_in = ep_addr;
                hid_machine.pipe_in = usbh_pipe_allocate (pudev, ep_addr);

                /* open channel for IN endpoint */
                usbh_pipe_create (pudev,
                                  &pphost->dev_prop, 
                                  hid_machine.pipe_in,
                                  USB_EPTYPE_INTR,
                                  hid_machine.len); 
            } else {
                hid_machine.ep_out = ep_addr;
                hid_machine.pipe_out = usbh_pipe_allocate (pudev, ep_addr);

                /* open channel for OUT endpoint */
                usbh_pipe_create (pudev,
                                  &pphost->dev_prop,
                                  hid_machine.pipe_out,
                                  USB_EPTYPE_INTR,
                                  hid_machine.len);
            }
        }

        start_toggle = 0;
        status = USBH_OK; 
    } else {
        pphost->usr_cb->dev_not_supported();
    }

    return status;
}

/*!
    \brief      de-initialize the host pipes used for the HID class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
void usbh_hid_itf_deinit (usb_core_driver *pudev, void *phost)
{
    if (0x00U != hid_machine.pipe_in) {
        usb_pipe_halt (pudev, hid_machine.pipe_in);

        usbh_pipe_free (pudev, hid_machine.pipe_in);

        hid_machine.pipe_in = 0;     /* reset the pipe as free */
    }

    if (0x00U != hid_machine.pipe_out) {
        usb_pipe_halt (pudev, hid_machine.pipe_out);

        usbh_pipe_free (pudev, hid_machine.pipe_out);

        hid_machine.pipe_out = 0; /* reset the channel as free */
    }

    start_toggle = 0;
}

/*!
    \brief      handle HID class requests for HID class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_hid_class_req (usb_core_driver *pudev, void *phost)
{
    usbh_host *pphost = phost;

    usbh_status status = USBH_BUSY;
    usbh_status class_req_status = USBH_BUSY;

    /* handle HID control state machine */
    switch (hid_machine.ctlstate) {
    case HID_REQ_IDLE:
    case HID_REQ_GET_HID_DESC:
        /* get HID descriptor */ 
        if (usbh_hid_desc_get (pudev, pphost, USB_HID_DESC_SIZE) == USBH_OK) {
            usbh_hiddesc_parse(&hid_desc, pudev->host.rx_buf);

            hid_machine.ctlstate = HID_REQ_GET_REPORT_DESC;
        }
        break;

    case HID_REQ_GET_REPORT_DESC:
        /* get report descriptor */ 
        if (usbh_hid_reportdesc_get(pudev, pphost, hid_desc.wDescriptorLength) == USBH_OK) {
            hid_machine.ctlstate = HID_REQ_SET_IDLE;
        }
        break;

    case HID_REQ_SET_IDLE:
        class_req_status = usbh_set_idle (pudev, pphost, 0, 0);

        /* set idle */
        if (class_req_status == USBH_OK) {
            hid_machine.ctlstate = HID_REQ_SET_PROTOCOL;
        } else if(class_req_status == USBH_NOT_SUPPORTED) {
            hid_machine.ctlstate = HID_REQ_SET_PROTOCOL;
        }
        break; 

    case HID_REQ_SET_PROTOCOL:
        /* set protocol */
        if (usbh_set_protocol (pudev, pphost, 0) == USBH_OK) {
            hid_machine.ctlstate = HID_REQ_IDLE;

            /* all requests performed */
            status = USBH_OK;
        }
        break;

    default:
        break;
    }

    return status;
}

/*!
    \brief      manage state machine for HID data transfers 
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_hid_handle (usb_core_driver *pudev, void *phost)
{
    usbh_host *pphost = phost;
    usbh_status status = USBH_OK;

    static uint32_t frame_count;

    switch (hid_machine.state) {
    case HID_IDLE:
        hid_machine.proc->Init();
        hid_machine.state = HID_SYNC;
        break;

    case HID_SYNC:
        /* sync with start of even frame */
        if (usb_frame_even(pudev) == TRUE) {
            hid_machine.state = HID_GET_DATA;
        }
        break;

    case HID_GET_DATA:

        usbh_data_recev (pudev, hid_machine.buf, hid_machine.pipe_in, hid_machine.len);

        start_toggle = 1;
        hid_machine.state = HID_POLL;
        hid_machine.timer = usb_curframe_get (pudev);
        break;

    case HID_POLL:
        frame_count = usb_curframe_get (pudev);

        if ((frame_count > hid_machine.timer) && ((frame_count - hid_machine.timer) >= hid_machine.poll)) {
            hid_machine.state = HID_GET_DATA;
        } else if ((frame_count < hid_machine.timer) && ((frame_count + 0x3FFFU - hid_machine.timer) >= hid_machine.poll)) {
            hid_machine.state = HID_GET_DATA;
        } else if (usbh_urbstate_get (pudev, hid_machine.pipe_in) == URB_DONE) {
            if (start_toggle == 1) { /* handle data once */
                start_toggle = 0;
                hid_machine.proc->Decode(hid_machine.buf);
            }
        } else if (usbh_urbstate_get (pudev, hid_machine.pipe_in) == URB_STALL) { /* IN endpoint stalled */
            /* issue clear feature on interrupt in endpoint */ 
            if ((usbh_clrfeature (pudev, pphost, hid_machine.ep_addr, hid_machine.pipe_in)) == USBH_OK) {
                /* change state to issue next in token */
                hid_machine.state = HID_GET_DATA;
            }
        }
        break;

    default:
        break;
    }

    return status;
}

/*!
    \brief      send get report descriptor command to the device
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[in]  len: HID report descriptor length
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_hid_reportdesc_get (usb_core_driver *pudev, usbh_host *puhost, uint16_t len)
{
    usbh_status status = USBH_BUSY;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_STRD,
            .bRequest      = USB_GET_DESCRIPTOR,
            .wValue        = USBH_DESC(USB_DESCTYPE_REPORT),
            .wIndex        = 0,
            .wLength       = len
        };

        usbh_ctlstate_config (puhost, pudev->host.rx_buf, len);
    }

    status = usbh_ctl_handler (pudev, puhost);

    return status;
}

/*!
    \brief      send get HID descriptor command to the device
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[in]  len: HID descriptor length
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_hid_desc_get (usb_core_driver *pudev, usbh_host *puhost, uint16_t len)
{
    usbh_status status = USBH_BUSY;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_STRD,
            .bRequest      = USB_GET_DESCRIPTOR,
            .wValue        = USBH_DESC(USB_DESCTYPE_HID),
            .wIndex        = 0,
            .wLength       = len
        };

        usbh_ctlstate_config (puhost, pudev->host.rx_buf, len);
    }

    status = usbh_ctl_handler (pudev, puhost);

    return status;
}

/*!
    \brief      set idle state
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[in]  duration: duration for HID set idle request
    \param[in]  report_ID: targetted report ID for HID set idle request
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_set_idle (usb_core_driver *pudev,
                                  usbh_host *puhost,
                                  uint8_t duration,
                                  uint8_t report_ID)
{
    usbh_status status = USBH_BUSY;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_OUT | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = SET_IDLE,
            .wValue        = (duration << 8) | report_ID,
            .wIndex        = 0,
            .wLength       = 0
        };

        usbh_ctlstate_config (puhost, NULL, 0);
    }

    status = usbh_ctl_handler (pudev, puhost);

    return status;
}

/*!
    \brief      set report
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[in]  report_type: duration for HID set idle request
    \param[in]  report_ID: targetted report ID for HID set idle request
    \param[in]  report_len: length of data report to be send
    \param[in]  report_buf: report buffer
    \param[out] none
    \retval     operation status
*/
usbh_status usbh_set_report (usb_core_driver *pudev, 
                             usbh_host *puhost,
                             uint8_t  report_type,
                             uint8_t  report_ID,
                             uint8_t  report_len,
                             uint8_t *report_buf)
{
    usbh_status status = USBH_BUSY;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_OUT | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = SET_REPORT,
            .wValue        = (report_type << 8) | report_ID,
            .wIndex        = 0,
            .wLength       = report_len
        };

        usbh_ctlstate_config (puhost, report_buf, report_len);
    }

    status = usbh_ctl_handler (pudev, puhost);

    return status;
}

/*!
    \brief      set protocol state
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[in]  protocol: boot/report protocol
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_set_protocol (usb_core_driver *pudev, usbh_host *puhost, uint8_t protocol)
{
    usbh_status status = USBH_BUSY;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_OUT | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = SET_PROTOCOL,
            .wValue        = !protocol,
            .wIndex        = 0,
            .wLength       = 0
        };

        usbh_ctlstate_config (puhost, NULL, 0);
    }

    status = usbh_ctl_handler (pudev, puhost);

    return status;
}

/*!
    \brief      parse the HID descriptor
    \param[in]  hid_desc: pointer to HID descriptor
    \param[in]  buf: pointer to buffer where the source descriptor is available
    \param[out] none
    \retval     operation status
*/
static void  usbh_hiddesc_parse (usb_desc_hid *hid_desc, uint8_t *buf)
{
    hid_desc->header.bLength         = *(uint8_t *)(buf + 0);
    hid_desc->header.bDescriptorType = *(uint8_t *)(buf + 1);
    hid_desc->bcdHID                 = BYTE_SWAP(buf + 2);
    hid_desc->bCountryCode           = *(uint8_t *)(buf + 4);
    hid_desc->bNumDescriptors        = *(uint8_t *)(buf + 5);
    hid_desc->bDescriptorType        = *(uint8_t *)(buf + 6);
    hid_desc->wDescriptorLength      = BYTE_SWAP(buf + 7);
}
