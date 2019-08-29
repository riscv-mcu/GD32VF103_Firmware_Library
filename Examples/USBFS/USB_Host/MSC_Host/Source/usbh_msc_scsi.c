/*!
    \file  usbh_msc_scsi.c
    \brief USB MSC SCSI commands implemention

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

#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bbb.h"

usbh_msc_parameter usbh_msc_param;

uint8_t usbh_databuf_in[512];
uint8_t usbh_databuf_out[512];

/*!
    \brief      send 'Test unit ready' command to the device
    \param[in]  pudev: pointer to usb device
    \param[out] none
    \retval     operation status
*/
uint8_t usbh_msc_test_unitready (usb_core_driver *pudev)
{
    uint8_t index;
    usbh_msc_status status = USBH_MSC_BUSY;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.cmd_state_machine) {
        case CMD_SEND_STATE:  
            /* prepare the CBW and relevent field */
            msc_cbw_data.field.dCBWDataTransferLength = 0;       /* no data transfer */
            msc_cbw_data.field.bmCBWFlags = USB_TRX_OUT;
            msc_cbw_data.field.bCBWCBLength = CBW_LENGTH_TEST_UNIT_READY;

            msc_botxfer_param.xfer_buf = msc_csw_data.CSWArray;
            msc_botxfer_param.data_len = USBH_MSC_CSW_MAX_LENGTH;
            msc_botxfer_param.msc_state_current = USBH_MSC_TEST_UNIT_READY;

            for (index = CBW_CB_LENGTH; index != 0; index--) {
                msc_cbw_data.field.CBWCB[index] = 0x00;
            }

            msc_cbw_data.field.CBWCB[0] = SCSI_TEST_UNIT_READY; 
            msc_botxfer_param.bot_state = USBH_MSC_SEND_CBW;

            /* start the transfer, then let the state machine magage the other transactions */
            msc_botxfer_param.msc_state = USBH_MSC_BOT_USB_TRANSFERS;
            msc_botxfer_param.bot_xfer_status = USBH_MSC_BUSY;
            msc_botxfer_param.cmd_state_machine = CMD_WAIT_STATUS;

            status = USBH_MSC_BUSY; 
            break;

        case CMD_WAIT_STATUS: 
            if (msc_botxfer_param.bot_xfer_status == USBH_MSC_OK) {
                /* commands successfully sent and response received */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_OK;
            } else if ( msc_botxfer_param.bot_xfer_status == USBH_MSC_FAIL) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_FAIL;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_PHASE_ERROR) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_PHASE_ERROR;    
            }
            break;

        default:
            break;
        }
    }

    return status;
}

/*!
    \brief      send the read capacity command to the device
    \param[in]  pudev: pointer to usb device
    \param[out] none
    \retval     operation status
*/
uint8_t usbh_msc_read_capacity10 (usb_core_driver *pudev)
{
    uint8_t index;
    usbh_msc_status status = USBH_MSC_BUSY;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.cmd_state_machine) {
        case CMD_SEND_STATE:
            /* prepare the CBW and relevent field */
            msc_cbw_data.field.dCBWDataTransferLength = READ_CAPACITY10_DATA_LEN;
            msc_cbw_data.field.bmCBWFlags = USB_TRX_IN;
            msc_cbw_data.field.bCBWCBLength = CBW_LENGTH;

            msc_botxfer_param.xfer_buf = usbh_databuf_in;
            msc_botxfer_param.msc_state_current = USBH_MSC_READ_CAPACITY10;

            for (index = CBW_CB_LENGTH; index != 0; index--) {
                msc_cbw_data.field.CBWCB[index] = 0x00;
            }

            msc_cbw_data.field.CBWCB[0] = SCSI_READ_CAPACITY10; 
            msc_botxfer_param.bot_state = USBH_MSC_SEND_CBW;

            /* start the transfer, then let the state machine manage the other transactions */
            msc_botxfer_param.msc_state = USBH_MSC_BOT_USB_TRANSFERS;
            msc_botxfer_param.bot_xfer_status = USBH_MSC_BUSY;
            msc_botxfer_param.cmd_state_machine = CMD_WAIT_STATUS;

            status = USBH_MSC_BUSY;
            break;

        case CMD_WAIT_STATUS:
            if (msc_botxfer_param.bot_xfer_status == USBH_MSC_OK) {
                /* assign the capacity */
                (((uint8_t*)&usbh_msc_param.msc_capacity )[3]) = usbh_databuf_in[0];
                (((uint8_t*)&usbh_msc_param.msc_capacity )[2]) = usbh_databuf_in[1];
                (((uint8_t*)&usbh_msc_param.msc_capacity )[1]) = usbh_databuf_in[2];
                (((uint8_t*)&usbh_msc_param.msc_capacity )[0]) = usbh_databuf_in[3];

                /* assign the page length */
                (((uint8_t*)&usbh_msc_param.msc_page_len )[1]) = usbh_databuf_in[6];
                (((uint8_t*)&usbh_msc_param.msc_page_len )[0]) = usbh_databuf_in[7];

                /* commands successfully sent and response received */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_OK;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_FAIL) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_FAIL;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_PHASE_ERROR) {
                /* Failure Mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_PHASE_ERROR;
            } else {
                /* wait for the commands to get completed */
                /* no change in state machine */
            }
            break;

        default:
            break;
        }
    }

    return status;
}

/*!
    \brief      send the mode sense6 command to the device
    \param[in]  pudev: pointer to usb device
    \param[out] none
    \retval     operation status
*/
uint8_t usbh_msc_mode_sense6 (usb_core_driver *pudev)
{
    uint8_t index;
    usbh_msc_status status = USBH_MSC_BUSY;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.cmd_state_machine) {
        case CMD_SEND_STATE:
            /* prepare the CBW and relevent field */
            msc_cbw_data.field.dCBWDataTransferLength = XFER_LEN_MODE_SENSE6;
            msc_cbw_data.field.bmCBWFlags = USB_TRX_IN;
            msc_cbw_data.field.bCBWCBLength = CBW_LENGTH;

            msc_botxfer_param.xfer_buf = usbh_databuf_in;
            msc_botxfer_param.msc_state_current = USBH_MSC_MODE_SENSE6;

            for (index = CBW_CB_LENGTH; index != 0; index--) {
                msc_cbw_data.field.CBWCB[index] = 0x00;
            }

            msc_cbw_data.field.CBWCB[0] = SCSI_MODE_SENSE6; 
            msc_cbw_data.field.CBWCB[2] = MODE_SENSE_PAGE_CONTROL_FIELD | MODE_SENSE_PAGE_CODE;
            msc_cbw_data.field.CBWCB[4] = XFER_LEN_MODE_SENSE6;
            msc_botxfer_param.bot_state = USBH_MSC_SEND_CBW;

            /* start the transfer, then let the state machine manage the other transactions */
            msc_botxfer_param.msc_state = USBH_MSC_BOT_USB_TRANSFERS;
            msc_botxfer_param.bot_xfer_status = USBH_MSC_BUSY;
            msc_botxfer_param.cmd_state_machine = CMD_WAIT_STATUS;

            status = USBH_MSC_BUSY;
            break;

        case CMD_WAIT_STATUS:
            if (msc_botxfer_param.bot_xfer_status == USBH_MSC_OK) {
                /* assign the write protect status */
                /* if writeprotect = 0, writing is allowed 
                   if writeprotect != 0, disk is write protected */
                if (usbh_databuf_in[2] & MASK_MODE_SENSE_WRITE_PROTECT) {
                    usbh_msc_param.msc_write_protect = DISK_WRITE_PROTECTED;
                } else {
                    usbh_msc_param.msc_write_protect = 0;
                }

                /* commands successfully sent and response received */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_OK;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_FAIL) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_FAIL;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_PHASE_ERROR) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_PHASE_ERROR;    
            } else {
                /* wait for the commands to get completed */
                /* no change in state machine */
            }
            break;

        default:
            break;
        }
    }

    return status;
}

/*!
    \brief      send the Request Sense command to the device
    \param[in]  pudev: pointer to usb device
    \param[out] none
    \retval     operation status
*/
uint8_t usbh_msc_request_sense (usb_core_driver *pudev)
{
    usbh_msc_status status = USBH_MSC_BUSY;
    uint8_t index;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.cmd_state_machine) {
        case CMD_SEND_STATE:
            /* prepare the cbw and relevent field */
            msc_cbw_data.field.dCBWDataTransferLength = ALLOCATION_LENGTH_REQUEST_SENSE;
            msc_cbw_data.field.bmCBWFlags = USB_TRX_IN;
            msc_cbw_data.field.bCBWCBLength = CBW_LENGTH;

            msc_botxfer_param.xfer_buf = usbh_databuf_in;
            msc_botxfer_param.msc_state_bkp = msc_botxfer_param.msc_state_current;
            msc_botxfer_param.msc_state_current = USBH_MSC_REQUEST_SENSE;

            for (index = CBW_CB_LENGTH; index != 0; index--) {
                msc_cbw_data.field.CBWCB[index] = 0x00;
            }

            msc_cbw_data.field.CBWCB[0] = SCSI_REQUEST_SENSE; 
            msc_cbw_data.field.CBWCB[1] = DESC_REQUEST_SENSE;
            msc_cbw_data.field.CBWCB[4] = ALLOCATION_LENGTH_REQUEST_SENSE;

            msc_botxfer_param.bot_state = USBH_MSC_SEND_CBW;

            /* start the transfer, then let the state machine magage the other transactions */
            msc_botxfer_param.msc_state = USBH_MSC_BOT_USB_TRANSFERS;
            msc_botxfer_param.bot_xfer_status = USBH_MSC_BUSY;
            msc_botxfer_param.cmd_state_machine = CMD_WAIT_STATUS;

            status = USBH_MSC_BUSY;
            break;

        case CMD_WAIT_STATUS:
            if (msc_botxfer_param.bot_xfer_status == USBH_MSC_OK) {
                /* get sense data */
                (((uint8_t*)&usbh_msc_param.msc_sense_key )[3]) = usbh_databuf_in[0];
                (((uint8_t*)&usbh_msc_param.msc_sense_key )[2]) = usbh_databuf_in[1];
                (((uint8_t*)&usbh_msc_param.msc_sense_key )[1]) = usbh_databuf_in[2];
                (((uint8_t*)&usbh_msc_param.msc_sense_key )[0]) = usbh_databuf_in[3];

                /* commands successfully sent and response received  */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_OK;      
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_FAIL) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_FAIL;
            } else if ( msc_botxfer_param.bot_xfer_status == USBH_MSC_PHASE_ERROR ) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_PHASE_ERROR;    
            } else {
                /* wait for the commands to get completed */
                /* no change in state machine */
            }
            break;

        default:
            break;
        }
    }

    return status;
}

/*!
    \brief      send the write10 command to the device
    \param[in]  pudev: pointer to usb device
    \param[in]  data_buf: data buffer contains the data to write
    \param[in]  addr: address to which the data will be written
    \param[in]  byte_num: number of bytes to be written
    \param[out] none
    \retval     operation status
*/
uint8_t usbh_msc_write10 (usb_core_driver *pudev, uint8_t *data_buf, uint32_t addr, uint32_t byte_num)
{
    uint8_t index;
    uint16_t nbOfPages;
    usbh_msc_status status = USBH_MSC_BUSY;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.cmd_state_machine) {
        case CMD_SEND_STATE:
            msc_cbw_data.field.dCBWDataTransferLength = byte_num;
            msc_cbw_data.field.bmCBWFlags = USB_TRX_OUT;
            msc_cbw_data.field.bCBWCBLength = CBW_LENGTH;
            msc_botxfer_param.xfer_buf = data_buf;

            for (index = CBW_CB_LENGTH; index != 0; index--) {
                msc_cbw_data.field.CBWCB[index] = 0x00;
            }

            msc_cbw_data.field.CBWCB[0] = SCSI_WRITE10; 

            /* logical block address */
            msc_cbw_data.field.CBWCB[2] = (((uint8_t*)&addr)[3]);
            msc_cbw_data.field.CBWCB[3] = (((uint8_t*)&addr)[2]);
            msc_cbw_data.field.CBWCB[4] = (((uint8_t*)&addr)[1]);
            msc_cbw_data.field.CBWCB[5] = (((uint8_t*)&addr)[0]);

            /* USBH_MSC_PAGE_LENGTH = 512*/
            nbOfPages = byte_num / USBH_MSC_PAGE_LENGTH;

            /* tranfer length */
            msc_cbw_data.field.CBWCB[7] = (((uint8_t *)&nbOfPages)[1]);
            msc_cbw_data.field.CBWCB[8] = (((uint8_t *)&nbOfPages)[0]);

            msc_botxfer_param.bot_state = USBH_MSC_SEND_CBW;

            /* start the transfer, then let the state machine magage the other transactions */
            msc_botxfer_param.msc_state = USBH_MSC_BOT_USB_TRANSFERS;
            msc_botxfer_param.bot_xfer_status = USBH_MSC_BUSY;
            msc_botxfer_param.cmd_state_machine = CMD_WAIT_STATUS;

            status = USBH_MSC_BUSY;
            break;

        case CMD_WAIT_STATUS:
            if (msc_botxfer_param.bot_xfer_status == USBH_MSC_OK) {
                /* commands successfully sent and response received */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_OK;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_FAIL) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_PHASE_ERROR) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_PHASE_ERROR;    
            }
            break;

        default:
            break;
        }
    }

    return status;
}

/*!
    \brief      send the read10 command to the device
    \param[in]  pudev: pointer to usb device
    \param[in]  data_buf: data buffer contains the data to write
    \param[in]  addr: address to which the data will be written
    \param[in]  byte_num: number of bytes to be written
    \param[out] none
    \retval     operation status
*/
uint8_t usbh_msc_read10 (usb_core_driver *pudev, uint8_t *data_buf, uint32_t addr, uint32_t byte_num)
{
    uint8_t index;
    uint16_t nbOfPages;

    static usbh_msc_status status = USBH_MSC_BUSY;

    status = USBH_MSC_BUSY;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.cmd_state_machine) {
        case CMD_SEND_STATE:
            /* prepare the CBW and relevent field */
            msc_cbw_data.field.dCBWDataTransferLength = byte_num;
            msc_cbw_data.field.bmCBWFlags = USB_TRX_IN;
            msc_cbw_data.field.bCBWCBLength = CBW_LENGTH;

            msc_botxfer_param.xfer_buf = data_buf;

            for(index = CBW_CB_LENGTH; index != 0; index--)
            {
                msc_cbw_data.field.CBWCB[index] = 0x00;
            }

            msc_cbw_data.field.CBWCB[0] = SCSI_READ10; 

            /* logical block address */
            msc_cbw_data.field.CBWCB[2] = (((uint8_t*)&addr)[3]);
            msc_cbw_data.field.CBWCB[3] = (((uint8_t*)&addr)[2]);
            msc_cbw_data.field.CBWCB[4] = (((uint8_t*)&addr)[1]);
            msc_cbw_data.field.CBWCB[5] = (((uint8_t*)&addr)[0]);

            /* USBH_MSC_PAGE_LENGTH = 512 */
            nbOfPages = byte_num / USBH_MSC_PAGE_LENGTH;

            /* tranfer length */
            msc_cbw_data.field.CBWCB[7] = (((uint8_t *)&nbOfPages)[1]);
            msc_cbw_data.field.CBWCB[8] = (((uint8_t *)&nbOfPages)[0]);

            msc_botxfer_param.bot_state = USBH_MSC_SEND_CBW;

            /* start the transfer, then let the state machine magage the other transactions */
            msc_botxfer_param.msc_state = USBH_MSC_BOT_USB_TRANSFERS;
            msc_botxfer_param.bot_xfer_status = USBH_MSC_BUSY;
            msc_botxfer_param.cmd_state_machine = CMD_WAIT_STATUS;

            status = USBH_MSC_BUSY;
            break;

        case CMD_WAIT_STATUS:
            if ((msc_botxfer_param.bot_xfer_status == USBH_MSC_OK) && \
                (pudev->host.connect_status)) {
                /* commands successfully sent and response received  */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_OK;
            } else if ((msc_botxfer_param.bot_xfer_status == USBH_MSC_FAIL) && \
                    (pudev->host.connect_status)) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
            } else if (msc_botxfer_param.bot_xfer_status == USBH_MSC_PHASE_ERROR) {
                /* failure mode */
                msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
                status = USBH_MSC_PHASE_ERROR;
            } else {
                /* wait for the commands to get completed */
                /* no change in state machine */
            }
            break;

        default:
            break;
        }
    }

    return status;
}
