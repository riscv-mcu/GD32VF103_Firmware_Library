/*!
    \file  usbh_msc_fatfs.c
    \brief USB MSC host FATFS related functions

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

#include "usb_conf.h"
#include "diskio.h"
#include "usbh_msc_core.h"

static volatile DSTATUS state = STA_NOINIT; /* disk status */

extern usb_core_driver usbh_msc_core;
extern usbh_host usb_host;

/*!
    \brief      initialize the disk drive
    \param[in]  drv: physical drive number (0)
    \param[out] none
    \retval     operation status
*/
DSTATUS disk_initialize (BYTE drv)
{
    if (usbh_msc_core.host.connect_status) {
        state &= ~STA_NOINIT;
    }

    return state;
}

/*!
    \brief      get disk status
    \param[in]  drv: physical drive number (0)
    \param[out] none
    \retval     operation status
*/
DSTATUS disk_status (BYTE drv)
{
    if (drv) {
        return STA_NOINIT; /* supports only single drive */
    }

    return state;
}

/*!
    \brief      read sectors
    \param[in]  drv: physical drive number (0)
    \param[in]  buff: pointer to the data buffer to store read data
    \param[in]  sector: start sector number (LBA)
    \param[in]  count: sector count (1..255)
    \param[out] none
    \retval     operation status
*/
DRESULT disk_read (BYTE drv, 
                   BYTE *buff, 
                   DWORD sector, 
                   BYTE count)
{
    BYTE status = USBH_MSC_OK;

    if (drv || !count) {
        return RES_PARERR;
    }

    if (state & STA_NOINIT) {
        return RES_NOTRDY;
    }

    if (usbh_msc_core.host.connect_status) {
        do {
            status = usbh_msc_read10 (&usbh_msc_core, buff,sector, 512 * count);
            usbh_msc_botxfer(&usbh_msc_core, &usb_host);

            if (!usbh_msc_core.host.connect_status) {
                return RES_ERROR;
            }
        } while(status == USBH_MSC_BUSY);
    }

    if (status == USBH_MSC_OK) {
        return RES_OK;
    }

    return RES_ERROR;
}

#if _READONLY == 0

/*!
    \brief      write sectors
    \param[in]  drv: physical drive number (0)
    \param[in]  buff: pointer to the data buffer to store read data
    \param[in]  sector: start sector number (LBA)
    \param[in]  count: sector count (1..255)
    \param[out] none
    \retval     operation status
*/
DRESULT disk_write (BYTE drv, 
                    const BYTE *buff, 
                    DWORD sector, 
                    BYTE count)
{
    BYTE status = USBH_MSC_OK;

    if (drv || !count) {
        return RES_PARERR;
    }

    if (state & STA_NOINIT) {
        return RES_NOTRDY;
    }

    if (state & STA_PROTECT) {
        return RES_WRPRT;
    }

    if (usbh_msc_core.host.connect_status) {
        do {
            status = usbh_msc_write10 (&usbh_msc_core, (BYTE*)buff,sector, 512 * count);
            usbh_msc_botxfer(&usbh_msc_core, &usb_host);

            if (!usbh_msc_core.host.connect_status) {
                return RES_ERROR;
            }
        } while(status == USBH_MSC_BUSY);
    }

    if (status == USBH_MSC_OK) {
        return RES_OK;
    }

    return RES_ERROR;
}

#endif /* _READONLY == 0 */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0

/*!
    \brief      I/O control function
    \param[in]  drv: physical drive number (0)
    \param[in]  ctrl: control code
    \param[in]  buff: pointer to the data buffer to store read data
    \param[out] none
    \retval     operation status
*/
DRESULT disk_ioctl (BYTE drv, 
                    BYTE ctrl, 
                    void *buff)
{
    DRESULT res = RES_OK;

    if (drv) {
        return RES_PARERR;
    }

    res = RES_ERROR;

    if (state & STA_NOINIT) {
        return RES_NOTRDY;
    }

    switch (ctrl) {
    case CTRL_SYNC:         /* make sure that no pending write process */
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:  /* get number of sectors on the disk (dword) */
        *(DWORD*)buff = (DWORD) usbh_msc_param.msc_capacity;
        res = RES_OK;
        break;

    case GET_SECTOR_SIZE:  /* get r/w sector size (word) */
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

    case GET_BLOCK_SIZE:   /* get erase block size in unit of sector (dword) */
        *(DWORD*)buff = 512;
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
}
#endif /* _USE_IOCTL != 0 */
