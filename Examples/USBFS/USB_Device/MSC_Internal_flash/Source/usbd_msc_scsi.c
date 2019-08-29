/*!
    \file  usbd_msc_scsi.c
    \brief USB SCSI layer functions

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
#include "usbd_msc_scsi.h"
#include "usbd_msc_mem.h"
#include "usbd_msc_data.h"
#include "usbd_enum.h"

msc_scsi_sense scsi_sense[SENSE_LIST_DEEPTH];

uint8_t scsi_sense_head;
uint8_t scsi_sense_tail;

uint32_t scsi_blk_size[MEM_LUN_NUM];
uint32_t scsi_blk_nbr[MEM_LUN_NUM];

uint32_t scsi_blk_addr;
uint32_t scsi_blk_len;

usb_core_driver *cdev;

static int8_t scsi_test_unit_ready      (uint8_t lun, uint8_t *params);
static int8_t scsi_inquiry              (uint8_t lun, uint8_t *params);
static int8_t scsi_read_format_capacity (uint8_t lun, uint8_t *params);
static int8_t scsi_read_capacity10      (uint8_t lun, uint8_t *params);
static int8_t scsi_request_sense        (uint8_t lun, uint8_t *params);
static int8_t scsi_start_stop_unit      (uint8_t lun, uint8_t *params);
static int8_t scsi_allow_medium_removal (uint8_t lun, uint8_t *params);
static int8_t scsi_mode_sense6          (uint8_t lun, uint8_t *params);
static int8_t scsi_toc_cmd_read         (uint8_t lun, uint8_t *params);
static int8_t scsi_mode_sense10         (uint8_t lun, uint8_t *params);
static int8_t scsi_write10              (uint8_t lun, uint8_t *params);
static int8_t scsi_read10               (uint8_t lun, uint8_t *params);
static int8_t scsi_verify10             (uint8_t lun, uint8_t *params);
static int8_t scsi_check_address_range  (uint8_t lun, uint32_t blk_offset, uint16_t blk_nbr);
static int8_t scsi_process_read         (uint8_t lun);
static int8_t scsi_process_write        (uint8_t lun);
static int8_t scsi_format_cmd           (uint8_t lun);

/*!
    \brief      process SCSI commands
    \param[in]  udev: pointer to USB device instance
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
int8_t scsi_process_cmd(usb_core_driver *pudev, uint8_t lun, uint8_t *params)
{
    cdev = pudev;

    switch (params[0]) {
        case SCSI_TEST_UNIT_READY:
            return scsi_test_unit_ready (lun, params);

        case SCSI_REQUEST_SENSE:
            return scsi_request_sense (lun, params);

        case SCSI_INQUIRY:
            return scsi_inquiry (lun, params);

        case SCSI_START_STOP_UNIT:
            return scsi_start_stop_unit (lun, params);

        case SCSI_ALLOW_MEDIUM_REMOVAL:
            return scsi_allow_medium_removal (lun, params);

        case SCSI_MODE_SENSE6:
            return scsi_mode_sense6 (lun, params);

        case SCSI_MODE_SENSE10:
            return scsi_mode_sense10 (lun, params);

        case SCSI_READ_FORMAT_CAPACITIES:
            return scsi_read_format_capacity (lun, params);

        case SCSI_READ_CAPACITY10:
            return scsi_read_capacity10 (lun, params);

        case SCSI_READ10:
            return scsi_read10 (lun, params); 

        case SCSI_WRITE10:
            return scsi_write10 (lun, params);

        case SCSI_VERIFY10:
            return scsi_verify10 (lun, params);

        case SCSI_FORMAT_UNIT:
            return scsi_format_cmd (lun);

        case SCSI_READ_TOC_DATA:
            return scsi_toc_cmd_read (lun, params);

        default:
            scsi_sense_code (lun, ILLEGAL_REQUEST, INVALID_CDB);
            return -1;
    }
}

/*!
    \brief      process SCSI Test Unit Ready command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_test_unit_ready (uint8_t lun, uint8_t *params)
{
    /* case 9 : Hi > D0 */
    if (0U != bbb_cbw.dCBWDataTransferLength) {
        scsi_sense_code (bbb_cbw.bCBWLUN, ILLEGAL_REQUEST, INVALID_CDB);

        return -1;
    }

    if (0U != usbd_mem_fops->mem_ready(lun)) {
        scsi_sense_code(lun, NOT_READY, MEDIUM_NOT_PRESENT);

        return -1;
    }

    msc_bbb_datalen = 0;

    return 0;
}

/*!
    \brief      process Inquiry command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_inquiry (uint8_t lun, uint8_t *params)
{
    uint8_t *page = NULL;

    uint16_t len = 0U;

    if (params[1] & 0x01U) {
        /* Evpd is set */
        page = (uint8_t *)msc_page00_inquiry_data;

        len = INQUIRY_PAGE00_LENGTH;
    } else {
        page = (uint8_t *)usbd_mem_fops->mem_inquiry_data[lun];

        len = page[4] + 5;

        if (params[4] <= len) {
            len = params[4];
        }
    }

    msc_bbb_datalen = len;

    while (len) {
        len--;
        msc_bbb_data[len] = page[len];
    }

    return 0;
}

/*!
    \brief      process Read Capacity 10 command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_read_capacity10 (uint8_t lun, uint8_t *params)
{
    uint32_t blk_num = usbd_mem_fops->mem_block_len[lun] - 1;

    scsi_blk_nbr[lun] = usbd_mem_fops->mem_block_len[lun];
    scsi_blk_size[lun] = usbd_mem_fops->mem_block_size[lun];

    msc_bbb_data[0] = (uint8_t)(blk_num >> 24);
    msc_bbb_data[1] = (uint8_t)(blk_num >> 16);
    msc_bbb_data[2] = (uint8_t)(blk_num >> 8);
    msc_bbb_data[3] = (uint8_t)(blk_num);

    msc_bbb_data[4] = (uint8_t)(scsi_blk_size[lun] >> 24);
    msc_bbb_data[5] = (uint8_t)(scsi_blk_size[lun] >> 16);
    msc_bbb_data[6] = (uint8_t)(scsi_blk_size[lun] >> 8);
    msc_bbb_data[7] = (uint8_t)(scsi_blk_size[lun]);

    msc_bbb_datalen = 8;

    return 0;
}

/*!
    \brief      process Read Format Capacity command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_read_format_capacity (uint8_t lun, uint8_t *params)
{
    uint32_t blk_size = usbd_mem_fops->mem_block_size[lun];
    uint32_t blk_num = usbd_mem_fops->mem_block_len[lun];

    uint16_t i = 0U;

    for (i = 0U; i < 12U; i++) {
        msc_bbb_data[i] = 0;
    }

    uint32_t blk_nbr = blk_num - 1;

    msc_bbb_data[3] = 0x08U;
    msc_bbb_data[4] = (uint8_t)(blk_nbr >> 24);
    msc_bbb_data[5] = (uint8_t)(blk_nbr >> 16);
    msc_bbb_data[6] = (uint8_t)(blk_nbr >>  8);
    msc_bbb_data[7] = (uint8_t)(blk_nbr);

    msc_bbb_data[8] = 0x02U;
    msc_bbb_data[9] = (uint8_t)(blk_size >> 16);
    msc_bbb_data[10] = (uint8_t)(blk_size >> 8);
    msc_bbb_data[11] = (uint8_t)(blk_size);

    msc_bbb_datalen = 12U;

    return 0;
}

/*!
    \brief      process Mode Sense6 command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_mode_sense6 (uint8_t lun, uint8_t *params)
{
    uint16_t len = 8U;

    msc_bbb_datalen = len;

    while (len) {
        len--;
        msc_bbb_data[len] = msc_mode_sense6_data[len];
    }

    return 0;
}

/*!
    \brief      process Mode Sense10 command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_mode_sense10 (uint8_t lun, uint8_t *params)
{
    uint16_t len = 8;

    msc_bbb_datalen = len;

    while (len) {
        len--;
        msc_bbb_data[len] = msc_mode_sense10_data[len];
    }

    return 0;
}

/*!
    \brief      process Request Sense command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_request_sense (uint8_t lun, uint8_t *params)
{
    uint8_t i = 0U;

    for (i = 0U; i < REQUEST_SENSE_DATA_LEN; i++) {
        msc_bbb_data[i] = 0U;
    }

    msc_bbb_data[0] = 0x70U;
    msc_bbb_data[7] = REQUEST_SENSE_DATA_LEN - 6U;

    if ((scsi_sense_head != scsi_sense_tail)) {
        msc_bbb_data[2] = scsi_sense[scsi_sense_head].SenseKey;
        msc_bbb_data[12] = scsi_sense[scsi_sense_head].ASCQ;
        msc_bbb_data[13] = scsi_sense[scsi_sense_head].ASC;
        scsi_sense_head++;

        if (scsi_sense_head == SENSE_LIST_DEEPTH) {
            scsi_sense_head = 0U;
        }
    }

    msc_bbb_datalen = USB_MIN(REQUEST_SENSE_DATA_LEN, params[4]);

    return 0;
}

/*!
    \brief      load the last error code in the error list
    \param[in]  lun: logical unit number
    \param[in]  skey: sense key
    \param[in]  asc: additional aense key
    \param[out] none
    \retval     none
*/
void scsi_sense_code (uint8_t lun, uint8_t skey, uint8_t asc)
{
    scsi_sense[scsi_sense_tail].SenseKey = skey;
    scsi_sense[scsi_sense_tail].ASC = asc << 8;
    scsi_sense_tail++;

    if (SENSE_LIST_DEEPTH == scsi_sense_tail) {
        scsi_sense_tail = 0;
    }
}

/*!
    \brief      process Start Stop Unit command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_start_stop_unit (uint8_t lun, uint8_t *params)
{
    msc_bbb_datalen = 0;

    return 0;
}

/*!
    \brief      process Allow Medium Removal command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_allow_medium_removal (uint8_t lun, uint8_t *params)
{
    msc_bbb_datalen = 0;

    return 0;
}

/*!
    \brief      process Read10 command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_read10 (uint8_t lun, uint8_t *params)
{
    if (msc_bbb_state == BBB_IDLE) {
        /* direction is from device to host */
        if (0x80U != (bbb_cbw.bmCBWFlags & 0x80U)) {
            scsi_sense_code (bbb_cbw.bCBWLUN, ILLEGAL_REQUEST, INVALID_CDB);

            return -1;
        }

        if (0U != usbd_mem_fops->mem_ready(lun)) {
            scsi_sense_code (lun, NOT_READY, MEDIUM_NOT_PRESENT);

            return -1;
        }

        scsi_blk_addr = (params[2] << 24) | (params[3] << 16) | \
                        (params[4] <<  8) |  params[5];

        scsi_blk_len = (params[7] <<  8) | params[8];

        if (scsi_check_address_range (lun, scsi_blk_addr, scsi_blk_len) < 0) {
            return -1; /* error */
        }

        msc_bbb_state = BBB_DATA_IN;

        scsi_blk_addr *= scsi_blk_size[lun];
        scsi_blk_len  *= scsi_blk_size[lun];

        /* cases 4,5 : Hi <> Dn */
        if (bbb_cbw.dCBWDataTransferLength != scsi_blk_len) {
            scsi_sense_code (bbb_cbw.bCBWLUN, ILLEGAL_REQUEST, INVALID_CDB);

            return -1;
        }
    }

    msc_bbb_datalen = MSC_MEDIA_PACKET_SIZE;

    return scsi_process_read (lun);
}

/*!
    \brief      process Write10 command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_write10 (uint8_t lun, uint8_t *params)
{
    if (BBB_IDLE == msc_bbb_state) {
        /* case 8 : Hi <> Do */
        if (0x80U == (bbb_cbw.bmCBWFlags & 0x80U)) {
            scsi_sense_code (bbb_cbw.bCBWLUN, ILLEGAL_REQUEST, INVALID_CDB);

            return -1;
        }

        /* check whether media is ready */
        if (0U != usbd_mem_fops->mem_ready(lun)) {
            scsi_sense_code (lun, NOT_READY, MEDIUM_NOT_PRESENT);

            return -1;
        }

        /* check if media is write-protected */
        if (0U != usbd_mem_fops->mem_protected(lun)) {
            scsi_sense_code (lun, NOT_READY, WRITE_PROTECTED);

            return -1;
        }

        scsi_blk_addr = (params[2] << 24) | (params[3] << 16) | \
                        (params[4] <<  8) |  params[5];

        scsi_blk_len = (params[7] <<  8) | params[8];

        /* check if LBA address is in the right range */
        if (scsi_check_address_range (lun, scsi_blk_addr, scsi_blk_len) < 0) {
            return -1; /* error */
        }

        scsi_blk_addr *= scsi_blk_size[lun];
        scsi_blk_len  *= scsi_blk_size[lun];

        /* cases 3,11,13 : Hn,Ho <> D0 */
        if (bbb_cbw.dCBWDataTransferLength != scsi_blk_len) {
            scsi_sense_code (bbb_cbw.bCBWLUN, ILLEGAL_REQUEST, INVALID_CDB);

            return -1;
        }

        /* prepare endpoint to receive first data packet */
        msc_bbb_state = BBB_DATA_OUT;

        usbd_ep_recev (cdev, 
                       MSC_OUT_EP, 
                       msc_bbb_data, 
                       USB_MIN (scsi_blk_len, MSC_MEDIA_PACKET_SIZE));
    } else { /* write process ongoing */
        return scsi_process_write (lun);
    }

    return 0;
}

/*!
    \brief      process Verify10 command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t scsi_verify10 (uint8_t lun, uint8_t *params)
{
    if (0x02U == (params[1] & 0x02U)) {
        scsi_sense_code (lun, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);

        return -1; /* error, verify mode not supported*/
    }

    if (scsi_check_address_range (lun, scsi_blk_addr, scsi_blk_len) < 0) {
        return -1; /* error */
    }

    msc_bbb_datalen = 0U;

    return 0;
}

/*!
    \brief      check address range
    \param[in]  lun: logical unit number
    \param[in]  blk_offset: block offset
    \param[in]  blk_nbr: number of block to be processed
    \param[out] none
    \retval     none
*/
static int8_t scsi_check_address_range (uint8_t lun, uint32_t blk_offset, uint16_t blk_nbr)
{
    if ((blk_offset + blk_nbr) > scsi_blk_nbr[lun]) {
        scsi_sense_code (lun, ILLEGAL_REQUEST, ADDRESS_OUT_OF_RANGE);

        return -1;
    }

    return 0;
}

/*!
    \brief      handle read process
    \param[in]  lun: logical unit number
    \param[out] none
    \retval     none
*/
static int8_t scsi_process_read (uint8_t lun)
{
    uint32_t len = USB_MIN(scsi_blk_len, MSC_MEDIA_PACKET_SIZE);

    if (usbd_mem_fops->mem_read(lun,
                                msc_bbb_data, 
                                scsi_blk_addr, 
                                len / scsi_blk_size[lun]) < 0) {
        scsi_sense_code(lun, HARDWARE_ERROR, UNRECOVERED_READ_ERROR);

        return -1; 
    }

    usbd_ep_send (cdev, MSC_IN_EP, msc_bbb_data, len);

    scsi_blk_addr += len;
    scsi_blk_len  -= len;

    /* case 6 : Hi = Di */
    bbb_csw.dCSWDataResidue -= len;

    if (0U == scsi_blk_len) {
        msc_bbb_state = BBB_LAST_DATA_IN;
    }

    return 0;
}

/*!
    \brief      handle write process
    \param[in]  lun: logical unit number
    \param[out] none
    \retval     status
*/
static int8_t scsi_process_write (uint8_t lun)
{
    uint32_t len = USB_MIN(scsi_blk_len, MSC_MEDIA_PACKET_SIZE);

    if (usbd_mem_fops->mem_write (lun,
                                  msc_bbb_data, 
                                  scsi_blk_addr, 
                                  len / scsi_blk_size[lun]) < 0) {
        scsi_sense_code(lun, HARDWARE_ERROR, WRITE_FAULT);

        return -1;
    }

    scsi_blk_addr += len;
    scsi_blk_len  -= len;

    /* case 12 : Ho = Do */
    bbb_csw.dCSWDataResidue -= len;

    if (0U == scsi_blk_len) {
        msc_bbb_csw_send (cdev, CSW_CMD_PASSED);
    } else {
        /* prapare endpoint to receive next packet */
        usbd_ep_recev (cdev, 
                       MSC_OUT_EP, 
                       msc_bbb_data, 
                       USB_MIN (scsi_blk_len, MSC_MEDIA_PACKET_SIZE));
    }

    return 0;
}

/*!
    \brief      process Format Unit command
    \param[in]  lun: logical unit number
    \param[out] none
    \retval     status
*/
static int8_t scsi_format_cmd (uint8_t lun)
{
    return 0;
}

/*!
    \brief      process Read_Toc command
    \param[in]  lun: logical unit number
    \param[in]  params: command parameters
    \param[out] none
    \retval     none
*/
static int8_t  scsi_toc_cmd_read (uint8_t lun, uint8_t *params)
{
    uint8_t* pPage;
    uint16_t len;

    pPage = (uint8_t *)&usbd_mem_fops->mem_toc_data[lun * READ_TOC_CMD_LEN];
    len = pPage[1] + 2;

    msc_bbb_datalen = len;

    while (len) {
        len--;
        msc_bbb_data[len] = pPage[len];
    }

    return 0;
}
