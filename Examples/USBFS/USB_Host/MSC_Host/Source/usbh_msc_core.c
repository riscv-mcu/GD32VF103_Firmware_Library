/*!
    \file  usbh_msc_core.c
    \brief USB MSC(mass storage device) class driver

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
#include "usbh_transc.h"
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bbb.h"

#define USBH_MSC_ERROR_RETRY_LIMIT 10

uint8_t msc_error_count = 0;

usbh_msc_machine msc_machine;

static void usbh_msc_itf_deinit         (usb_core_driver *pudev, void *puhost);
static usbh_status usbh_msc_itf_init    (usb_core_driver *pudev, void *puhost);
static usbh_status usbh_msc_handle      (usb_core_driver *pudev, void *puhost);
static usbh_status usbh_msc_req         (usb_core_driver *pudev, void *puhost);
static usbh_status usbh_msc_maxlun_get  (usb_core_driver *pudev, usbh_host *puhost);

usbh_class_cb usbh_msc_cb = 
{
    usbh_msc_itf_init,
    usbh_msc_itf_deinit,
    usbh_msc_req,
    usbh_msc_handle,
};

void usbh_msc_error_handle (uint8_t status);

/*!
    \brief      interface initialization for MSC class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_msc_itf_init (usb_core_driver *pudev, void *puhost)
{
    usbh_host *pphost = puhost;

    usb_desc_itf *itf_desc = &pphost->dev_prop.itf_desc[0];

    if ((itf_desc->bInterfaceClass == MSC_CLASS) && 
        (itf_desc->bInterfaceProtocol == MSC_PROTOCOL)) {

        usb_desc_ep *ep_desc = &pphost->dev_prop.ep_desc[0][0];

        if (ep_desc->bEndpointAddress & 0x80) {
            msc_machine.msc_bulk_epin = ep_desc->bEndpointAddress;
            msc_machine.msc_bulk_epinsize = ep_desc->wMaxPacketSize;
        } else {
            msc_machine.msc_bulk_epout = ep_desc->bEndpointAddress;
            msc_machine.msc_bulk_epoutsize = ep_desc->wMaxPacketSize;
        }

        ep_desc = &pphost->dev_prop.ep_desc[0][1];

        if (ep_desc->bEndpointAddress & 0x80) {
            msc_machine.msc_bulk_epin = ep_desc->bEndpointAddress;
            msc_machine.msc_bulk_epinsize = ep_desc->wMaxPacketSize;
        } else {
            msc_machine.msc_bulk_epout = ep_desc->bEndpointAddress;
            msc_machine.msc_bulk_epoutsize = ep_desc->wMaxPacketSize;
        }

        msc_machine.hc_num_out = usbh_pipe_allocate(pudev, msc_machine.msc_bulk_epout);

        msc_machine.hc_num_in = usbh_pipe_allocate(pudev, msc_machine.msc_bulk_epin);


        /* open the new channels */
        usbh_pipe_create (pudev,
                          &pphost->dev_prop,
                          msc_machine.hc_num_out,
                          USB_EPTYPE_BULK,
                          msc_machine.msc_bulk_epoutsize);  

        usbh_pipe_create (pudev,
                          &pphost->dev_prop,
                          msc_machine.hc_num_in,
                          USB_EPTYPE_BULK,
                          msc_machine.msc_bulk_epinsize);

    } else {
        pphost->usr_cb->dev_not_supported();
    }

    return USBH_OK;
}

/*!
    \brief      de-initialize interface by freeing host channels allocated to interface
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
void usbh_msc_itf_deinit (usb_core_driver *pudev, void *puhost)
{
    if (msc_machine.hc_num_out) {
        usb_pipe_halt (pudev, msc_machine.hc_num_out);
        usbh_pipe_free (pudev, msc_machine.hc_num_out);

        msc_machine.hc_num_out = 0;
    }

    if (msc_machine.hc_num_in) {
        usb_pipe_halt (pudev, msc_machine.hc_num_in);
        usbh_pipe_free (pudev, msc_machine.hc_num_in);

        msc_machine.hc_num_in = 0;
    }
}

/*!
    \brief      initialize the MSC state machine
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_msc_req (usb_core_driver *pudev, void *puhost)
{
    usbh_status status = USBH_OK;
    msc_botxfer_param.msc_state = USBH_MSC_BOT_INIT_STATE;

    return status; 
}

/*!
    \brief      MSC state machine handler
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_msc_handle (usb_core_driver *pudev, void *puhost)
{
    usbh_host *pphost = puhost;
    usbh_status status = USBH_BUSY;
    uint8_t msc_status = USBH_MSC_BUSY;
    uint8_t app_status = 0;

    static uint8_t max_lun_exceed = FALSE;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.msc_state) {
        case USBH_MSC_BOT_INIT_STATE:
            usbh_msc_init(pudev);
            msc_botxfer_param.msc_state = USBH_MSC_BOT_RESET;
            break;

        case USBH_MSC_BOT_RESET:
            status = USBH_OK;

            msc_botxfer_param.msc_state = USBH_MSC_GET_MAX_LUN;
            break;

        case USBH_MSC_GET_MAX_LUN:
            /* issue Get_MaxLun request */
            status = usbh_msc_maxlun_get (pudev, puhost);

            if (status == USBH_OK) {
                msc_machine.max_lun = *(msc_machine.buf);

                /* if device has more that one logical unit then it is not supported */
                if ((msc_machine.max_lun > 0) && (max_lun_exceed == FALSE)) {
                    max_lun_exceed = TRUE;
                    //pphost->usr_cb->dev_not_supported();

                    break;
                }

                msc_botxfer_param.msc_state = USBH_MSC_TEST_UNIT_READY;
            }

            if (status == USBH_NOT_SUPPORTED) {
                /* if the command has failed, then we need to move to next state, after
                   STALL condition is cleared by Control-Transfer */
                msc_botxfer_param.msc_state_bkp = USBH_MSC_TEST_UNIT_READY; 

                /* a clear feature should be issued here */
                msc_botxfer_param.msc_state = USBH_MSC_CTRL_ERROR_STATE;
            }
            break;

        case USBH_MSC_CTRL_ERROR_STATE:
            /* issue clearfeature request */
            status = usbh_clrfeature(pudev,
                                     puhost,
                                     0x00,
                                     pphost->control.pipe_out_num);

            if (status == USBH_OK) {
                /* if GetMaxLun request not support, assume single LUN configuration */
                msc_machine.max_lun = 0;  

                msc_botxfer_param.msc_state = msc_botxfer_param.msc_state_bkp;     
            }
            break;

        case USBH_MSC_TEST_UNIT_READY:
            /* issue SCSI command TestUnitReady */ 
            msc_status = usbh_msc_test_unitready(pudev);

            if (msc_status == USBH_MSC_OK) {
                msc_botxfer_param.msc_state = USBH_MSC_READ_CAPACITY10;
                msc_error_count = 0;
                status = USBH_OK;
            } else {
                usbh_msc_error_handle (msc_status);
            }
            break;

        case USBH_MSC_READ_CAPACITY10:
            /* issue READ_CAPACITY10 SCSI command */
            msc_status = usbh_msc_read_capacity10(pudev);

            if (msc_status == USBH_MSC_OK) {
                msc_botxfer_param.msc_state = USBH_MSC_MODE_SENSE6;
                msc_error_count = 0;
                status = USBH_OK;
            } else {
                usbh_msc_error_handle (msc_status);
            }
            break;

        case USBH_MSC_MODE_SENSE6:
            /* issue ModeSense6 SCSI command for detecting if device is write-protected */
            msc_status = usbh_msc_mode_sense6 (pudev);

            if (msc_status == USBH_MSC_OK) {
                msc_botxfer_param.msc_state = USBH_MSC_DEFAULT_APPLI_STATE;
                msc_error_count = 0;
                status = USBH_OK;
            } else {
                usbh_msc_error_handle (msc_status);
            }
            break;

        case USBH_MSC_REQUEST_SENSE:
            /* issue RequestSense SCSI command for retreiving error code */
            msc_status = usbh_msc_request_sense (pudev);
            if (msc_status == USBH_MSC_OK) {
                msc_botxfer_param.msc_state = msc_botxfer_param.msc_state_bkp;
                status = USBH_OK;
            } else {
                usbh_msc_error_handle (msc_status);
            }
            break;

        case USBH_MSC_BOT_USB_TRANSFERS:
            /* process the BOT state machine */
            usbh_msc_botxfer(pudev , puhost);
            break;

        case USBH_MSC_DEFAULT_APPLI_STATE:
            /* process application callback for MSC */
            app_status = pphost->usr_cb->dev_user_app();

            if (app_status == 0) {
                msc_botxfer_param.msc_state = USBH_MSC_DEFAULT_APPLI_STATE;
            } else if (app_status == 1) {
                /* de-init requested from application layer */
                status = USBH_APPLY_DEINIT;
            }
            break;

        case USBH_MSC_UNRECOVERED_STATE:
            status = USBH_UNRECOVERED_ERROR;
            break;

        default:
            break;
        }
    }

    return status;
}

/*!
    \brief      get max lun of the mass storage device
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status
*/
static usbh_status usbh_msc_maxlun_get (usb_core_driver *pudev, usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_REQTYPE_CLASS | USB_RECPTYPE_ITF,
            .bRequest      = BBB_GET_MAX_LUN,
            .wValue        = 0,
            .wIndex        = 0,
            .wLength       = 1
        };

        usbh_ctlstate_config (puhost, msc_machine.buf, 1);
    } 

    status = usbh_ctl_handler (pudev, puhost);

    return status;
}

/*!
    \brief      handling errors occuring during the MSC state machine
    \param[in]  status: error status
    \param[out] none
    \retval     operation status
*/
void usbh_msc_error_handle (uint8_t status)
{
    if (status == USBH_MSC_FAIL) {
        msc_error_count++;

        if (msc_error_count < USBH_MSC_ERROR_RETRY_LIMIT) {
            /* try msc level error recovery, issue the request sense to get drive error reason  */
            msc_botxfer_param.msc_state = USBH_MSC_REQUEST_SENSE;
            msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
        } else {
            /* error trials exceeded the limit, go to unrecovered state */
            msc_botxfer_param.msc_state = USBH_MSC_UNRECOVERED_STATE;
        }
    } else if (status == USBH_MSC_PHASE_ERROR) {
        /* phase error, go to unrecoovered state */
        msc_botxfer_param.msc_state = USBH_MSC_UNRECOVERED_STATE;
    } else if (status == USBH_MSC_BUSY) {
        /* no change in state */
    }
}
