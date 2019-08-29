/*!
    \file  usbd_msc_bbb.c
    \brief USB BBB(Bulk/Bulk/Bulk) protocol core functions
    \note  BBB means Bulk-only transport protocol for USB MSC

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

#include "usbd_msc_bbb.h"
#include "usbd_msc_mem.h"
#include "usbd_msc_scsi.h"
#include "usbd_transc.h"
#include "usbd_enum.h"

uint8_t msc_bbb_state;
uint8_t msc_bbb_status;

uint16_t msc_bbb_datalen;

uint8_t msc_bbb_data[MSC_MEDIA_PACKET_SIZE];

msc_bbb_cbw bbb_cbw;
msc_bbb_csw bbb_csw;

static void msc_bbb_cbw_decode (usb_core_driver *pudev);
static void msc_bbb_data_send (usb_core_driver *pudev, uint8_t *pbuf, uint16_t Len);
static void msc_bbb_abort (usb_core_driver *pudev);

/*!
    \brief      initialize the bbb process
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     none
*/
void msc_bbb_init (usb_core_driver *pudev)
{
    uint8_t lun_num;

    msc_bbb_state = BBB_IDLE;
    msc_bbb_status = BBB_STATUS_NORMAL;

    /* init the storage logic unit */
    for(lun_num = 0U; lun_num < MEM_LUN_NUM; lun_num++) {
        usbd_mem_fops->mem_init(lun_num);
    }

    /* flush the Rx FIFO */
    usbd_fifo_flush (pudev, MSC_OUT_EP);

    /* flush the Tx FIFO */
    usbd_fifo_flush (pudev, MSC_IN_EP);

    /* prepare endpoint to receive the first BBB CBW */
    usbd_ep_recev (pudev, MSC_OUT_EP, (uint8_t *)&bbb_cbw, BBB_CBW_LENGTH);
}

/*!
    \brief      reset the BBB machine
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     none
*/
void msc_bbb_reset (usb_core_driver *pudev)
{
    msc_bbb_state = BBB_IDLE;
    msc_bbb_status = BBB_STATUS_RECOVERY;

    /* prapare endpoint to receive the first BBB command */
    usbd_ep_recev (pudev, MSC_OUT_EP, (uint8_t *)&bbb_cbw, BBB_CBW_LENGTH);
}

/*!
    \brief      de-initialize the BBB machine
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     none
*/
void msc_bbb_deinit (usb_core_driver *pudev)
{
    msc_bbb_state = BBB_IDLE;
}

/*!
    \brief      handle BBB data IN stage
    \param[in]  pudev: pointer to USB device instance
    \param[in]  ep_num: endpoint number
    \param[out] none
    \retval     none
*/
void msc_bbb_data_in (usb_core_driver *pudev, uint8_t ep_num)
{
    usbd_fifo_flush (pudev, MSC_IN_EP);

    switch (msc_bbb_state) {
    case BBB_DATA_IN:
        if (scsi_process_cmd (pudev, bbb_cbw.bCBWLUN, &bbb_cbw.CBWCB[0]) < 0) {
            msc_bbb_csw_send (pudev, CSW_CMD_FAILED);
        }
        break;

    case BBB_SEND_DATA:
    case BBB_LAST_DATA_IN:
        msc_bbb_csw_send (pudev, CSW_CMD_PASSED);
        break;

    default:
        break;
    }
}

/*!
    \brief      handle BBB data OUT stage
    \param[in]  pudev: pointer to USB device instance
    \param[in]  ep_num: endpoint number
    \param[out] none
    \retval     none
*/
void msc_bbb_data_out (usb_core_driver *pudev, uint8_t ep_num)
{
    switch (msc_bbb_state) {
    case BBB_IDLE:
        msc_bbb_cbw_decode (pudev);
        break;

    case BBB_DATA_OUT:
        if (scsi_process_cmd (pudev, bbb_cbw.bCBWLUN, &bbb_cbw.CBWCB[0]) < 0) {
            msc_bbb_csw_send (pudev, CSW_CMD_FAILED);
        }
        break;

    default:
        break;
    }
}

/*!
    \brief      decode the CBW command and set the BBB state machine accordingtly
    \param[in]  udev: pointer to USB device instance
    \param[out] none
    \retval     none
*/
static void msc_bbb_cbw_decode (usb_core_driver *pudev)
{
    bbb_csw.dCSWTag = bbb_cbw.dCBWTag;
    bbb_csw.dCSWDataResidue = bbb_cbw.dCBWDataTransferLength;

    if ((BBB_CBW_LENGTH != usbd_rxcount_get (pudev, MSC_OUT_EP)) ||
            (BBB_CBW_SIGNATURE != bbb_cbw.dCBWSignature)||
                (bbb_cbw.bCBWLUN > 1U) || 
                    (bbb_cbw.bCBWCBLength < 1U) || 
                        (bbb_cbw.bCBWCBLength > 16U)) {
        /* illegal command handler */
        scsi_sense_code (bbb_cbw.bCBWLUN, ILLEGAL_REQUEST, INVALID_CDB);

        msc_bbb_status = BBB_STATUS_ERROR;

        msc_bbb_abort (pudev);
    } else {
        if (scsi_process_cmd (pudev, bbb_cbw.bCBWLUN, &bbb_cbw.CBWCB[0]) < 0) {
            msc_bbb_abort (pudev);
        } else if ((BBB_DATA_IN != msc_bbb_state) && 
                    (BBB_DATA_OUT != msc_bbb_state) &&
                      (BBB_LAST_DATA_IN != msc_bbb_state)) { /* burst xfer handled internally */
            if (msc_bbb_datalen > 0U) {
                msc_bbb_data_send (pudev, msc_bbb_data, msc_bbb_datalen);
            } else if (0U == msc_bbb_datalen) {
                msc_bbb_csw_send (pudev, CSW_CMD_PASSED);
            }
        }
    }
}

/*!
    \brief      send the requested data
    \param[in]  udev: pointer to USB device instance
    \param[in]  buf: pointer to data buffer
    \param[in]  len: data length
    \param[out] none
    \retval     none
*/
static void msc_bbb_data_send (usb_core_driver *pudev, uint8_t *buf, uint16_t len)
{
    len = USB_MIN (bbb_cbw.dCBWDataTransferLength, len);

    bbb_csw.dCSWDataResidue -= len;
    bbb_csw.bCSWStatus = CSW_CMD_PASSED;
    msc_bbb_state = BBB_SEND_DATA;

    usbd_ep_send (pudev, MSC_IN_EP, buf, len);
}

/*!
    \brief      send the CSW(command status wrapper)
    \param[in]  udev: pointer to USB device instance
    \param[in]  csw_status: CSW status
    \param[out] none
    \retval     none
*/
void msc_bbb_csw_send (usb_core_driver *pudev, uint8_t csw_status)
{
    bbb_csw.dCSWSignature = BBB_CSW_SIGNATURE;
    bbb_csw.bCSWStatus = csw_status;
    msc_bbb_state = BBB_IDLE;

    usbd_ep_send (pudev, MSC_IN_EP, (uint8_t *)&bbb_csw, BBB_CSW_LENGTH);

    /* prapare endpoint to receive next command */
    usbd_ep_recev (pudev, MSC_OUT_EP, (uint8_t *)&bbb_cbw, BBB_CBW_LENGTH);
}

/*!
    \brief      abort the current transfer
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     none
*/
static void msc_bbb_abort (usb_core_driver *pudev)
{
    if ((bbb_cbw.bmCBWFlags == 0) && 
         (bbb_cbw.dCBWDataTransferLength != 0) &&
          (msc_bbb_status == BBB_STATUS_NORMAL)) {
        usbd_ep_stall(pudev, MSC_OUT_EP);
    }

    usbd_ep_stall(pudev, MSC_IN_EP);

    if (msc_bbb_status == BBB_STATUS_ERROR) {
        usbd_ep_recev (pudev, MSC_OUT_EP, (uint8_t *)&bbb_cbw, BBB_CBW_LENGTH);
    }
}

/*!
    \brief      complete the clear feature request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  ep_num: endpoint number
    \param[out] none
    \retval     none
*/
void msc_bbb_clrfeature (usb_core_driver *pudev, uint8_t ep_num)
{
    if (msc_bbb_status == BBB_STATUS_ERROR)/* bad CBW signature */ {
        usbd_ep_stall(pudev, MSC_IN_EP);

        msc_bbb_status = BBB_STATUS_NORMAL;
    } else if(((ep_num & 0x80) == 0x80) && (msc_bbb_status != BBB_STATUS_RECOVERY)) {
        msc_bbb_csw_send (pudev, CSW_CMD_FAILED);
    }
}
