/*!
    \file  usbd_storage_msd.c
    \brief disk operations functions

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
//#include "sram_msd.h"
#include "flash_msd.h"
#include "usbd_msc_mem.h"

/* USB Mass storage Standard Inquiry Data */

const int8_t STORAGE_InquiryData[] = 
{
#ifdef SRAM_STORAGE
    /* LUN 0 */
    0x00,
    0x80,
    0x00,
    0x01,
    (USBD_STD_INQUIRY_LENGTH - 5),
    0x00,
    0x00,
    0x00,
    'G', 'D', '3', '2', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
    'I', 'n', 't', 'e', 'r', 'n', 'a', 'l', /* Product      : 16 Bytes */
    ' ', 's', 'r', 'a', 'm', ' ', ' ', ' ',
    '1', '.', '0' ,'0',                     /* Version      : 4 Bytes */
#else
    /* LUN 0 */
    0x00,
    0x80,
    0x00,
    0x01,
    (USBD_STD_INQUIRY_LENGTH - 5),
    0x00,
    0x00,
    0x00,
    'G', 'D', '3', '2', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
    'I', 'n', 't', 'e', 'r', 'n', 'a', 'l', /* Product      : 16 Bytes */
    ' ', 'f', 'l', 'a', 's', 'h', ' ', ' ',
    '1', '.', '0' ,'0',                     /* Version      : 4 Bytes */
#endif
};

static int8_t  STORAGE_Init             (uint8_t Lun);
static int8_t  STORAGE_IsReady          (uint8_t Lun);
static int8_t  STORAGE_IsWriteProtected (uint8_t Lun);
static int8_t  STORAGE_GetMaxLun        (void);

static int8_t  STORAGE_Read             (uint8_t Lun,
                                        uint8_t *buf,
                                        uint32_t BlkAddr,
                                        uint16_t BlkLen);

static int8_t  STORAGE_Write            (uint8_t Lun,
                                        uint8_t *buf,
                                        uint32_t BlkAddr,
                                        uint16_t BlkLen);


usbd_mem_cb USBD_Internal_Storage_fops =
{
    .mem_init      = STORAGE_Init,
    .mem_ready     = STORAGE_IsReady,
    .mem_protected = STORAGE_IsWriteProtected,
    .mem_read      = STORAGE_Read,
    .mem_write     = STORAGE_Write,
    .mem_maxlun    = STORAGE_GetMaxLun,

    .mem_inquiry_data = {(uint8_t *)STORAGE_InquiryData},

#ifdef SRAM_STORAGE
    .mem_block_size   = {ISRAM_BLOCK_SIZE},
    .mem_block_len    = {ISRAM_BLOCK_NUM}
#else
    .mem_block_size   = {ISFLASH_BLOCK_SIZE},
    .mem_block_len    = {ISFLASH_BLOCK_NUM}
#endif /* SRAM_STORAGE */
};

usbd_mem_cb *usbd_mem_fops = &USBD_Internal_Storage_fops;


/**
  * @brief  Initialize the storage medium
  * @param  Lun: logical unit number
  * @retval Status
  */
static int8_t  STORAGE_Init (uint8_t Lun)
{
#ifdef SRAM_STORAGE

#else
    flash_init ();
#endif
    return 0;
}

/**
  * @brief  Check whether the medium is ready
  * @param  Lun: Logical unit number
  * @retval Status
  */
static int8_t  STORAGE_IsReady (uint8_t Lun)
{
    return 0;
}

/**
  * @brief  Check whether the medium is write-protected
  * @param  Lun: logical unit number
  * @retval Status
  */
static int8_t  STORAGE_IsWriteProtected (uint8_t Lun)
{
    return 0;
}

/**
  * @brief  Read data from the medium
  * @param  Lun: logical unit number
  * @param  buf: pointer to the buffer to save data
  * @param  BlkAddr: address of 1st block to be read
  * @param  BlkLen: number of blocks to be read
  * @retval Status
  */
static int8_t  STORAGE_Read (uint8_t Lun,
                            uint8_t *buf,
                            uint32_t BlkAddr,
                            uint16_t BlkLen)
{
#ifdef SRAM_STORAGE
    if(SRAM_ReadMultiBlocks(buf,
                            BlkAddr,
                            ISRAM_BLOCK_SIZE,
                            BlkLen) != 0)
    {
        return 5;
    }
#else
    if(flash_multi_blocks_read (buf, 
                               BlkAddr, 
                              ISFLASH_BLOCK_SIZE,
                            BlkLen) != 0)
    {
            return 5;
    }
#endif
    return 0;
}

/**
  * @brief  Write data to the medium
  * @param  Lun: logical unit number
  * @param  buf: pointer to the buffer to write
  * @param  BlkAddr: address of blocks to be written
  * @param  BlkLen: number of blocks to be read
  * @retval Status
  */
static int8_t  STORAGE_Write (uint8_t Lun,
                             uint8_t *buf,
                             uint32_t BlkAddr,
                             uint16_t BlkLen)
{
#ifdef SRAM_STORAGE
    if(SRAM_WriteMultiBlocks(buf,
                             BlkAddr,
                             ISRAM_BLOCK_SIZE,
                             BlkLen) != 0)
    {
        return 5;
    }
#else
    if(flash_multi_blocks_write (buf,
                                 BlkAddr,
                           ISFLASH_BLOCK_SIZE,
                                 BlkLen) != 0)
    {
            return 5;
    }

#endif
    return (0);
}

/**
  * @brief  Get number of supported logical unit
  * @param  None
  * @retval Number of logical unit
  */
static int8_t STORAGE_GetMaxLun (void)
{
    return (MEM_LUN_NUM - 1);
}
