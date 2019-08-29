/*!
    \file  usbh_msc_bbb.c
    \brief USB MSC BBB protocol related functions

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

#include "usbh_transc.h"
#include "drv_usbh_int.h"
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bbb.h"

usbh_cbw_pkt msc_cbw_data;
usbh_csw_pkt msc_csw_data;

static uint32_t bot_stall_error_count;   /* keeps count of stall error cases */
usbh_botxfer msc_botxfer_param;

/*!
    \brief      initialize the mass storage parameters
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     none
*/
void usbh_msc_init (usb_core_driver *pudev)
{
    if (pudev->host.connect_status) {
        msc_cbw_data.field.dCBWSignature = BBB_CBW_SIGNATURE;
        msc_cbw_data.field.dCBWTag = USBH_MSC_BOT_CBW_TAG;
        msc_cbw_data.field.bCBWLUN = 0;  /* only one lun is supported */
        msc_botxfer_param.cmd_state_machine = CMD_SEND_STATE;
    }

    bot_stall_error_count = 0;

    msc_error_count = 0;
}

/*!
    \brief      manage the different states of BOT transfer and updates the status to upper layer
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     none
*/
void usbh_msc_botxfer (usb_core_driver *pudev, usbh_host *puhost)
{
    uint8_t xfer_dir, index;
    static uint32_t remain_len;
    static uint8_t *data_pointer, *data_pointer_prev;
    static uint8_t error_dir;

    usbh_status status;

    usb_urb_state URB_Status = URB_IDLE;

    if (pudev->host.connect_status) {
        switch (msc_botxfer_param.bot_state) {
            case USBH_MSC_SEND_CBW:
                /* send CBW */
                usbh_data_send (pudev,
                                &msc_cbw_data.CBWArray[0], 
                                msc_machine.hc_num_out, 
                                BBB_CBW_LENGTH);

                msc_botxfer_param.bot_state_bkp = USBH_MSC_SEND_CBW;
                msc_botxfer_param.bot_state = USBH_MSC_SENT_CBW;
                break;

            case USBH_MSC_SENT_CBW:
                URB_Status = usbh_urbstate_get(pudev, msc_machine.hc_num_out);

                if (URB_Status == URB_DONE) {
                    bot_stall_error_count = 0;
                    msc_botxfer_param.bot_state_bkp = USBH_MSC_SENT_CBW; 

                    /* if the CBW packet is sent successful, then change the state */
                    xfer_dir = (msc_cbw_data.field.bmCBWFlags & USB_TRX_MASK);

                    if (msc_cbw_data.field.dCBWDataTransferLength != 0) {
                        remain_len = msc_cbw_data.field.dCBWDataTransferLength;
                        data_pointer = msc_botxfer_param.xfer_buf;
                        data_pointer_prev = data_pointer;

                        /* if there is data transfer stage */
                        if (xfer_dir == USB_TRX_IN) {
                            /* data direction is IN */
                            msc_botxfer_param.bot_state = USBH_MSC_BOT_DATAIN_STATE;
                        } else {
                            /* data direction is OUT */
                            msc_botxfer_param.bot_state = USBH_MSC_BOT_DATAOUT_STATE;
                        }
                    } else {
                        /* If there is NO Data Transfer Stage */
                        msc_botxfer_param.bot_state = USBH_MSC_RECEIVE_CSW_STATE;
                    }
                } else if (URB_Status == URB_NOTREADY) {
                    msc_botxfer_param.bot_state  = msc_botxfer_param.bot_state_bkp;    
                } else if (URB_Status == URB_STALL) {
                    error_dir = USBH_MSC_DIR_OUT;
                    msc_botxfer_param.bot_state = USBH_MSC_BOT_ERROR_OUT;
                }
                break;

            case USBH_MSC_BOT_DATAIN_STATE:
                URB_Status = usbh_urbstate_get(pudev, msc_machine.hc_num_in);

                /* BOT DATA IN stage */
                if ((URB_Status == URB_DONE) || 
                    (msc_botxfer_param.bot_state_bkp != USBH_MSC_BOT_DATAIN_STATE)) {
                    bot_stall_error_count = 0;
                    msc_botxfer_param.bot_state_bkp = USBH_MSC_BOT_DATAIN_STATE;    

                    if (remain_len > msc_machine.msc_bulk_epinsize) {
                        usbh_data_recev (pudev, 
                                         data_pointer, 
                                         msc_machine.hc_num_in, 
                                         msc_machine.msc_bulk_epinsize);

                        remain_len -= msc_machine.msc_bulk_epinsize;
                        data_pointer = data_pointer + msc_machine.msc_bulk_epinsize;
                    } else if (remain_len == 0) {
                        /* if value was 0, and successful transfer, then change the state */
                        msc_botxfer_param.bot_state = USBH_MSC_RECEIVE_CSW_STATE;
                    } else {
                        usbh_data_recev (pudev,
                                         data_pointer, 
                                         msc_machine.hc_num_in, 
                                         remain_len);

                        remain_len = 0; /* reset this value and keep in same state */
                    }
                } else if(URB_Status == URB_STALL) {
                    /* this is data stage stall condition */

                    error_dir = USBH_MSC_DIR_IN;
                    msc_botxfer_param.bot_state  = USBH_MSC_BOT_ERROR_IN;

                    /* Refer to USB Mass-Storage Class : BOT (www.usb.org) 
                    6.7.2 Host expects to receive data from the device
                    3. On a STALL condition receiving data, then:
                    The host shall accept the data received.
                    The host shall clear the Bulk-In pipe.
                    4. The host shall attempt to receive a CSW.

                    msc_botxfer_param.bot_state_bkp is used to switch to the Original 
                    state after the ClearFeature Command is issued.
                    */
                    msc_botxfer_param.bot_state_bkp = USBH_MSC_RECEIVE_CSW_STATE;
                }
                break;

            case USBH_MSC_BOT_DATAOUT_STATE:
                /* BOT DATA OUT stage */
                URB_Status = usbh_urbstate_get(pudev, msc_machine.hc_num_out);
                if (URB_Status == URB_DONE) {
                    bot_stall_error_count = 0;
                    msc_botxfer_param.bot_state_bkp = USBH_MSC_BOT_DATAOUT_STATE;

                    if (remain_len > msc_machine.msc_bulk_epoutsize) {
                        usbh_data_send (pudev,
                                        data_pointer, 
                                        msc_machine.hc_num_out, 
                                        msc_machine.msc_bulk_epoutsize);

                        data_pointer_prev = data_pointer;
                        data_pointer = data_pointer + msc_machine.msc_bulk_epoutsize;

                        remain_len = remain_len - msc_machine.msc_bulk_epoutsize;
                    } else if (remain_len == 0) {
                        /* if value was 0, and successful transfer, then change the state */
                        msc_botxfer_param.bot_state = USBH_MSC_RECEIVE_CSW_STATE;
                    } else {
                        usbh_data_send (pudev,
                                        data_pointer, 
                                        msc_machine.hc_num_out, 
                                        remain_len);

                        remain_len = 0; /* reset this value and keep in same state */
                    }
                } else if (URB_Status == URB_NOTREADY) {
                    if (data_pointer != data_pointer_prev) {
                        usbh_data_send (pudev,
                                        (data_pointer - msc_machine.msc_bulk_epoutsize), 
                                        msc_machine.hc_num_out, 
                                        msc_machine.msc_bulk_epoutsize);
                    } else {
                        usbh_data_send (pudev,
                                        data_pointer,
                                        msc_machine.hc_num_out, 
                                        msc_machine.msc_bulk_epoutsize);
                    }
                } else if (URB_Status == URB_STALL) {
                    error_dir = USBH_MSC_DIR_OUT;
                    msc_botxfer_param.bot_state  = USBH_MSC_BOT_ERROR_OUT;

                    /* Refer to USB Mass-Storage Class : BOT (www.usb.org) 
                    6.7.3 Ho - Host expects to send data to the device
                    3. On a STALL condition sending data, then:
                    " The host shall clear the Bulk-Out pipe.
                    4. The host shall attempt to receive a CSW.

                    The Above statement will do the clear the Bulk-Out pipe.
                    The Below statement will help in Getting the CSW.  

                    msc_botxfer_param.bot_state_bkp is used to switch to the Original 
                    state after the ClearFeature Command is issued.
                    */

                    msc_botxfer_param.bot_state_bkp = USBH_MSC_RECEIVE_CSW_STATE;
                }
                break;

            case USBH_MSC_RECEIVE_CSW_STATE:
                /* BOT CSW stage */     
                /* NOTE: We cannot reset the BOTStallErrorCount here as it may come from the clearFeature from previous command */
                msc_botxfer_param.bot_state_bkp = USBH_MSC_RECEIVE_CSW_STATE;
                msc_botxfer_param.xfer_buf = msc_csw_data.CSWArray;
                msc_botxfer_param.data_len = USBH_MSC_CSW_MAX_LENGTH;

                for (index = BBB_CSW_LENGTH; index != 0; index--) {
                    msc_csw_data.CSWArray[index] = 0;
                }

                msc_csw_data.CSWArray[0] = 0;

                usbh_data_recev (pudev,
                                 msc_botxfer_param.xfer_buf, 
                                 msc_machine.hc_num_in, 
                                 USBH_MSC_CSW_MAX_LENGTH);

                msc_botxfer_param.bot_state = USBH_MSC_DECODE_CSW;
                break;

            case USBH_MSC_DECODE_CSW:
                URB_Status = usbh_urbstate_get(pudev, msc_machine.hc_num_in);

                /* decode CSW */
                if (URB_Status == URB_DONE) {
                    bot_stall_error_count = 0;
                    msc_botxfer_param.bot_state_bkp = USBH_MSC_RECEIVE_CSW_STATE;
                    msc_botxfer_param.msc_state = msc_botxfer_param.msc_state_current ;
                    msc_botxfer_param.bot_xfer_status = usbh_msc_csw_decode (pudev , puhost);
                } else if (URB_Status == URB_STALL) {
                    error_dir = USBH_MSC_DIR_IN;
                    msc_botxfer_param.bot_state  = USBH_MSC_BOT_ERROR_IN;
                }
                break;

            case USBH_MSC_BOT_ERROR_IN: 
                status = usbh_msc_bot_abort(pudev, puhost, USBH_MSC_DIR_IN);

                if (status == USBH_OK) {
                    /* check if the error was due in both the directions */
                    if (error_dir == USBH_MSC_BOTH_DIR) {
                        /* if both directions are needed, switch to OUT direction */
                        msc_botxfer_param.bot_state = USBH_MSC_BOT_ERROR_OUT;
                    } else {
                        /* switch back to the original state, in many cases this will be USBH_MSC_RECEIVE_CSW_STATE state */
                        msc_botxfer_param.bot_state = msc_botxfer_param.bot_state_bkp;
                    }
                } else if (status == USBH_UNRECOVERED_ERROR) {
                    /* this means that there is a stall error limit, do reset recovery */
                    msc_botxfer_param.bot_xfer_status = USBH_MSC_PHASE_ERROR;
                }
                break;

            case USBH_MSC_BOT_ERROR_OUT: 
                status = usbh_msc_bot_abort (pudev, puhost, USBH_MSC_DIR_OUT);

                if (status == USBH_OK) {
                    /* switch back to the original state */
                    msc_botxfer_param.bot_state = msc_botxfer_param.bot_state_bkp;        
                } else if (status == USBH_UNRECOVERED_ERROR) {
                    /* this means that there is a stall error limit, do reset recovery */
                    msc_botxfer_param.bot_xfer_status = USBH_MSC_PHASE_ERROR;
                }
                break;

            default:
                break;
        }
    }
}

/*!
    \brief      manages the different error handling for stall
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[in]  direction: data IN or OUT
    \param[out] none
    \retval     none
*/
usbh_status usbh_msc_bot_abort (usb_core_driver *pudev, usbh_host *puhost, uint8_t direction)
{
    usbh_status status = USBH_BUSY;

    switch (direction) {
    case USBH_MSC_DIR_IN :
        /* send clrfeture command on bulk IN endpoint */
        status = usbh_clrfeature(pudev,
                                 puhost,
                                 msc_machine.msc_bulk_epin,
                                 msc_machine.hc_num_in);
        break;

    case USBH_MSC_DIR_OUT :
        /*send clrfeature command on bulk OUT endpoint */
        status = usbh_clrfeature(pudev, 
                                 puhost,
                                 msc_machine.msc_bulk_epout,
                                 msc_machine.hc_num_out);
        break;

    default:
        break;
    }

    bot_stall_error_count++; /* check continous number of times, stall has occured */

    if (bot_stall_error_count > MAX_BULK_STALL_COUNT_LIMIT) {
        status = USBH_UNRECOVERED_ERROR;
    }

    return status;
}

/*!
    \brief      decode the CSW received by the device and updates the same to upper layer
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     on success USBH_MSC_OK, on failure USBH_MSC_FAIL
    \notes
          Refer to USB Mass-Storage Class: BOT (www.usb.org)
          6.3.1 Valid CSW Conditions :
          The host shall consider the CSW valid when:
          1. dCSWSignature is equal to 53425355h
          2. the CSW is 13 (Dh) bytes in length,
          3. dCSWTag matches the dCBWTag from the corresponding CBW.
*/
uint8_t usbh_msc_csw_decode (usb_core_driver *pudev, usbh_host *puhost)
{
    uint8_t status;
    uint32_t data_xfercount = 0;

    status = USBH_MSC_FAIL;

    if (pudev->host.connect_status) {
        /* checking if the transfer length is diffrent than 13 */
        data_xfercount = usbh_xfercount_get (pudev, msc_machine.hc_num_in); 

        if (data_xfercount != BBB_CSW_LENGTH) {
            /* (4) Hi > Dn (Host expects to receive data from the device,
                   Device intends to transfer no data)
               (5) Hi > Di (Host expects to receive data from the device,
                   Device intends to send data to the host)
               (9) Ho > Dn (Host expects to send data to the device,
                   Device intends to transfer no data)
               (11) Ho > Do  (Host expects to send data to the device,
                    Device intends to receive data from the host)
            */

            status = USBH_MSC_PHASE_ERROR;
        } else {
            /* CSW length is correct */

            /* check validity of the CSW Signature and CSWStatus */
            if (msc_csw_data.field.dCSWSignature == BBB_CSW_SIGNATURE) {
                /* check condition 1. dCSWSignature is equal to 53425355h */
                if (msc_csw_data.field.dCSWTag == msc_cbw_data.field.dCBWTag) {
                    /* check condition 3. dCSWTag matches the dCBWTag from the corresponding CBW */
                    if (msc_csw_data.field.bCSWStatus == USBH_MSC_OK) {
                        /* refer to USB Mass-Storage Class : BOT (www.usb.org) 
                           Hn Host expects no data transfers
                           Hi Host expects to receive data from the device
                           Ho Host expects to send data to the device

                           Dn Device intends to transfer no data
                           Di Device intends to send data to the host
                           Do Device intends to receive data from the host

                           Section 6.7 
                           (1) Hn = Dn (Host expects no data transfers,
                               Device intends to transfer no data)
                           (6) Hi = Di (Host expects to receive data from the device,
                               Device intends to send data to the host)
                          (12) Ho = Do (Host expects to send data to the device, 
                               Device intends to receive data from the host)
                        */

                        status = USBH_MSC_OK;
                    } else if (msc_csw_data.field.bCSWStatus == USBH_MSC_FAIL) {
                        status = USBH_MSC_FAIL;
                    } else if(msc_csw_data.field.bCSWStatus == USBH_MSC_PHASE_ERROR) {
                        /* refer to USB Mass-Storage Class : BOT (www.usb.org) 
                           Section 6.7 
                           (2) Hn < Di ( Host expects no data transfers, 
                               Device intends to send data to the host)
                           (3) Hn < Do ( Host expects no data transfers, 
                               Device intends to receive data from the host)
                           (7) Hi < Di ( Host expects to receive data from the device, 
                               Device intends to send data to the host)
                           (8) Hi <> Do ( Host expects to receive data from the device, 
                               Device intends to receive data from the host)
                          (10) Ho <> Di (Host expects to send data to the device,
                               Di Device intends to send data to the host)
                          (13) Ho < Do (Host expects to send data to the device, 
                               Device intends to receive data from the host)
                        */

                        status = USBH_MSC_PHASE_ERROR;
                    }
                } /* CSW tag matching is checked */
            } /* CSW signature correct checking */else {
                /* If the CSW signature is not valid, we sall return the phase error to
                   upper layers for reset recovery */
                status = USBH_MSC_PHASE_ERROR;
            }
        } /* CSW length check */
    }

    msc_botxfer_param.bot_xfer_status = status;

    return status;
}
