/*!
    \file  audio_core.c
    \brief USB audio device class core functions

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

#include "usbd_audio_out_if.h"
#include "audio_core.h"

#define USBD_VID                     0x0483
#define USBD_PID                     0x5730

/* Main Buffer for Audio Data Out transfers and its relative pointers */
uint8_t  IsocOutBuff [TOTAL_OUT_BUF_SIZE * 2];
uint8_t* IsocOutWrPtr = IsocOutBuff;
uint8_t* IsocOutRdPtr = IsocOutBuff;

/* AUDIO Requests management functions */
static void AUDIO_Req_GetCurrent   (usb_dev *pudev, usb_req *req);
static void AUDIO_Req_SetCurrent   (usb_dev *pudev, usb_req *req);
static uint8_t USBD_AUDIO_DataOut (usb_dev *pudev, uint8_t EpID);
static uint8_t USBD_AUDIO_EP0_RxReady (usb_dev *pudev);
static uint8_t USBD_AUDIO_SOF (usb_dev *pudev);
static void USBD_AUDIO_SetInterface(usb_dev *pudev, usb_req *req);
static void USBD_AUDIO_GetInterface(usb_dev *pudev, usb_req *req);

/* Main Buffer for Audio Control Rrequests transfers and its relative variables */
uint8_t  AudioCtl[64];
uint8_t  AudioCtlCmd = 0;
uint8_t  AudioCtlUnit = 0;
uint32_t AudioCtlLen = 0;

static uint32_t PlayFlag = 0;

static __IO uint32_t USBD_AUDIO_AltSet = 0;

/* note:it should use the c99 standard when compiling the below codes */
/* USB standard device descriptor */
const usb_desc_dev device_descripter =
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

/* USB device configuration descriptor */
const usb_descriptor_configuration_set_struct configuration_descriptor = 
{
    .Config = 
    {
        .header = 
         {
             .bLength = USB_CFG_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_CONFIG 
         },
        .wTotalLength = USB_SPEAKER_CONFIG_DESC_SIZE,
        .bNumInterfaces = 0x02,
        .bConfigurationValue = 0x01,
        .iConfiguration = 0x00,
        .bmAttributes = 0xC0,
        .bMaxPower = 0x32
    },

    .Speaker_Std_Interface = 
    {
        .header = 
         {
             .bLength = USB_ITF_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_ITF 
         },
         .bInterfaceNumber = 0x00,
         .bAlternateSetting = 0x00,
         .bNumEndpoints = 0x00,
         .bInterfaceClass = 0x01,
         .bInterfaceSubClass = 0x01,
         .bInterfaceProtocol = 0x00,
         .iInterface = 0x00
    },
    
    .Speaker_AC_Interface = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_AC_interface_struct), 
             .bDescriptorType = 0x24 
         },
         .bDescriptorSubtype = 0x01,
         .bcdADC = 0x0100,
         .wTotalLength = 0x0027,
         .bInCollection = 0x01,
         .baInterfaceNr = 0x01
    },
    
    .Speaker_IN_Terminal = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_input_terminal_struct), 
             .bDescriptorType = 0x24 
         },
         .bDescriptorSubtype = 0x02,
         .bTerminalID = 0x01,
         .wTerminalType = 0x0101,
         .bAssocTerminal = 0x00,
         .bNrChannels = 0x01,
         .wChannelConfig = 0x0000,
         .iChannelNames = 0x00,
         .iTerminal = 0x00
    },
    
    .Speaker_Feature_Unit = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_mono_feature_unit_struct), 
             .bDescriptorType = 0x24 
         },
         .bDescriptorSubtype = AUDIO_CONTROL_FEATURE_UNIT,
         .bUnitID = AUDIO_OUT_STREAMING_CTRL,
         .bSourceID = 0x01,
         .bControlSize = 0x01,
         .bmaControls0 = AUDIO_CONTROL_MUTE,
         .bmaControls1 = 0x00,
         .iFeature = 0x00
    },
    
    .Speaker_OUT_Terminal = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_output_terminal_struct), 
             .bDescriptorType = AUDIO_INTERFACE_DESCRIPTOR_TYPE 
         },
         .bDescriptorSubtype = AUDIO_CONTROL_OUTPUT_TERMINAL,
         .bTerminalID = 0x03,
         .wTerminalType = 0x0301,
         .bAssocTerminal = 0x00,
         .bSourceID = 0x02,
         .iTerminal = 0x00
    },

    .Speaker_Std_AS_Interface_ZeroBand = 
    {
        .header = 
         {
             .bLength = USB_ITF_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_ITF 
         },
         .bInterfaceNumber = 0x01,
         .bAlternateSetting = 0x00,
         .bNumEndpoints = 0x00,
         .bInterfaceClass = USB_DEVICE_CLASS_AUDIO,
         .bInterfaceSubClass = AUDIO_SUBCLASS_AUDIOSTREAMING,
         .bInterfaceProtocol = AUDIO_PROTOCOL_UNDEFINED,
         .iInterface = 0x00
    },
    
    .Speaker_Std_AS_Interface_Opera = 
    {
        .header = 
         {
             .bLength = USB_ITF_DESC_LEN, 
             .bDescriptorType = USB_DESCTYPE_ITF 
         },
         .bInterfaceNumber = 0x01,
         .bAlternateSetting = 0x01,
         .bNumEndpoints = 0x01,
         .bInterfaceClass = USB_DEVICE_CLASS_AUDIO,
         .bInterfaceSubClass = AUDIO_SUBCLASS_AUDIOSTREAMING,
         .bInterfaceProtocol = AUDIO_PROTOCOL_UNDEFINED,
         .iInterface = 0x00
    },
    
    .Speaker_AS_Interface = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_AS_interface_struct), 
             .bDescriptorType = AUDIO_INTERFACE_DESCRIPTOR_TYPE 
         },
         .bDescriptorSubtype = AUDIO_STREAMING_GENERAL,
         .bTerminalLink = 0x01,
         .bDelay = 0x01,
         .wFormatTag = 0x0001,
    },
    
    .Speaker_Format_TypeI = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_format_type_struct), 
             .bDescriptorType = AUDIO_INTERFACE_DESCRIPTOR_TYPE 
         },
         .bDescriptorSubtype = AUDIO_STREAMING_FORMAT_TYPE,
         .bFormatType = AUDIO_FORMAT_TYPE_III,
         .bNrChannels = 0x02,
         .bSubFrameSize = 0x02,
         .bBitResolution = 0x10,
         .bSamFreqType = 0x01,
         .bSamFreq[0]= (uint8_t)USBD_AUDIO_FREQ,
         .bSamFreq[1]= USBD_AUDIO_FREQ >> 8,
         .bSamFreq[2]= USBD_AUDIO_FREQ >> 16
    },
    
    .Speaker_Std_Endpoint = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_std_endpoint_struct), 
             .bDescriptorType = USB_DESCTYPE_EP 
         },
         .bEndpointAddress = AUDIO_OUT_EP,
         .bmAttributes = USB_ENDPOINT_TYPE_ISOCHRONOUS,
         .wMaxPacketSize = PACKET_SIZE(USBD_AUDIO_FREQ),
         .bInterval = 0x01,
         .bRefresh = 0x00,
         .bSynchAddress = 0x00
    },
    
    .Speaker_AS_Endpoint = 
    {
        .header = 
         {
             .bLength = sizeof(usb_descriptor_AS_endpoint_struct), 
             .bDescriptorType = AUDIO_ENDPOINT_DESCRIPTOR_TYPE 
         },
         .bDescriptorSubtype = AUDIO_ENDPOINT_GENERAL,
         .bmAttributes = 0x00,
         .bLockDelayUnits = 0x00,
         .wLockDelay = 0x0000,
    }
};

/* USB language ID Descriptor */
const usb_desc_LANGID usbd_language_id_desc = 
{
    .header = 
     {
         .bLength = sizeof(usb_desc_LANGID), 
         .bDescriptorType = USB_DESCTYPE_STR
     },
    .wLANGID = ENG_LANGID
};

/* USB serial string */
uint8_t usbd_serial_string[USB_SERIAL_STRING_SIZE] =
{
    USB_SERIAL_STRING_SIZE,
    USB_DESCTYPE_STR,
};

__ALIGN_BEGIN void* const usbd_strings[] __ALIGN_END = 
{
    [STR_IDX_LANGID] = (uint8_t *)&usbd_language_id_desc,
    [STR_IDX_MFC] = USBD_STRING_DESC("GD32_Microelectronics"),
    [STR_IDX_PRODUCT] = USBD_STRING_DESC("GD32 Audio in FS Mode"),
    [STR_IDX_SERIAL] = USBD_STRING_DESC("GD32VF103-1.0.0-6f7e8dma"),
};

/*!
    \brief      initialize the AUDIO device
    \param[in]  pudev: pointer to usb device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     usb device operation status
*/
uint8_t audio_init (usb_dev *pudev, uint8_t config_index)
{
    /* initialize Rx endpoint */
    usbd_ep_setup(pudev, (const usb_desc_ep *)&(configuration_descriptor.Speaker_Std_Endpoint));

    /* Initialize the Audio output Hardware layer */
    if (AUDIO_OUT_fops.Init(USBD_AUDIO_FREQ, DEFAULT_VOLUME, 0) != USBD_OK)
    {
        return USBD_FAIL;
    }

    /* Prepare Out endpoint to receive audio data */
    usbd_ep_recev (pudev, AUDIO_OUT_EP, (uint8_t*)IsocOutBuff, AUDIO_OUT_PACKET);

    return USBD_OK;
}

/*!
    \brief      de-initialize the AUDIO device
    \param[in]  pudev: pointer to usb device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     usb device operation status
*/
uint8_t audio_deinit (usb_dev *pudev, uint8_t config_index)
{
    /* deinitialize AUDIO endpoints */
    usbd_ep_clear(pudev, AUDIO_OUT_EP);

    /* DeInitialize the Audio output Hardware layer */
    if (AUDIO_OUT_fops.DeInit(0) != USBD_OK)
    {
        return USBD_FAIL;
    }

    return USBD_OK;
}

/*!
    \brief      handle the AUDIO class-specific requests
    \param[in]  pudev: pointer to usb device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     usb device operation status
*/
uint8_t audio_req_handler (usb_dev *pudev, usb_req *req)
{
    switch (req->bmRequestType & USB_REQTYPE_MASK) {
    case USB_REQTYPE_CLASS:
        switch (req->bRequest) {
            case AUDIO_REQ_GET_CUR:
                AUDIO_Req_GetCurrent(pudev, req);
                break;

            case AUDIO_REQ_SET_CUR:
                AUDIO_Req_SetCurrent(pudev, req);
                break;

            default:
                usbd_enum_error (pudev, req);
                return USBD_FAIL; 
        }
        
    case USB_REQTYPE_STRD:
        /* standard device request */
        switch(req->bRequest) {
            case USB_GET_INTERFACE:
                USBD_AUDIO_GetInterface(pudev, req);
                break;

            case USB_SET_INTERFACE:
                USBD_AUDIO_SetInterface(pudev, req);
                break;

            default:
                break;
        }
        break;

    default:
        break;
    }

    return USBD_OK;
}

/*!
    \brief      handle data stage
    \param[in]  pudev: pointer to usb device instance
    \param[in]  rx_tx: the flag of Rx or Tx
    \param[in]  ep_id: the endpoint ID
    \param[out] none
    \retval     usb device operation status
*/
uint8_t audio_data_in_handler (usb_dev *pudev, uint8_t ep_id)
{
    if ((AUDIO_IN_EP & 0x7F) == ep_id) {     
        return USBD_OK;
    } 
    return USBD_FAIL;
}

uint8_t audio_data_out_handler (usb_dev *pudev, uint8_t ep_id)
{
    if ((EP0_OUT & 0x7F) == ep_id) {
    
        USBD_AUDIO_EP0_RxReady(pudev);
        return USBD_OK;
    } else if ((AUDIO_OUT_EP & 0x7F) == ep_id) {
      
        USBD_AUDIO_DataOut(pudev, ep_id);
        return USBD_OK;
    }
    return USBD_FAIL;
}

/**
  * @brief  Handles the Audio Out data stage.
  * @param  pudev: pointer to usb device instance
  * @param  EpID: endpoint identifer
  * @retval usb device operation status
  */
static uint8_t  USBD_AUDIO_DataOut (usb_dev *pudev, uint8_t EpID)
{
    if (EpID == AUDIO_OUT_EP)
    {
        /* Increment the Buffer pointer or roll it back when all buffers are full */
        if (IsocOutWrPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
        {
            /* All buffers are full: roll back */
            IsocOutWrPtr = IsocOutBuff;
        }
        else
        {
            /* Increment the buffer pointer */
            IsocOutWrPtr += AUDIO_OUT_PACKET;
        }

        /* Toggle the frame index */  
        pudev->dev.transc_out[EpID].frame_num =
        (pudev->dev.transc_out[EpID].frame_num)? 0:1;

        /* Prepare Out endpoint to receive next audio packet */
        usbd_ep_recev (pudev, AUDIO_OUT_EP, (uint8_t*)(IsocOutWrPtr), AUDIO_OUT_PACKET);

        /* Trigger the start of streaming only when half buffer is full */
        if ((PlayFlag == 0) && (IsocOutWrPtr >= (IsocOutBuff + ((AUDIO_OUT_PACKET * OUT_PACKET_NUM) / 2))))
        {
            /* Enable start of Streaming */
            PlayFlag = 1;
        }
    }

    return USBD_OK;
}

/**
  * @brief  Handles audio control requests data.
  * @param  pudev: pointer to usb device instance
  * @retval usb device operation status
  */
static uint8_t  USBD_AUDIO_EP0_RxReady (usb_dev *pudev)
{
    /* Check if an AudioControl request has been issued */
    if (AudioCtlCmd == AUDIO_REQ_SET_CUR)
    {/* In this driver, to simplify code, only SET_CUR request is managed */

        /* Check for which addressed unit the AudioControl request has been issued */
        if (AudioCtlUnit == AUDIO_OUT_STREAMING_CTRL)
        {/* In this driver, to simplify code, only one unit is manage */

            /* Call the audio interface mute function */
            AUDIO_OUT_fops.MuteCtl(AudioCtl[0]);

            /* Reset the AudioCtlCmd variable to prevent re-entering this function */
            AudioCtlCmd = 0;
            AudioCtlLen = 0;
        }
    }

    return USBD_OK;
}

/**
  * @brief  Handles the SOF event (data buffer update and synchronization).
  * @param  pudev: pointer to usb device instance
  * @retval usb device operation status
  */
static uint8_t  USBD_AUDIO_SOF (usb_dev *pudev)
{
    /* Check if there are available data in stream buffer.
       In this function, a single variable (PlayFlag) is used to avoid software delays.
       The play operation must be executed as soon as possible after the SOF detection. */
    if (PlayFlag)
    {
        /* Start playing received packet */
        AUDIO_OUT_fops.AudioCmd((uint8_t*)(IsocOutRdPtr),  /* Samples buffer pointer */
                                AUDIO_OUT_PACKET,          /* Number of samples in Bytes */
                                AUDIO_CMD_PLAY);           /* Command to be processed */

        /* Increment the Buffer pointer or roll it back when all buffers all full */  
        if (IsocOutRdPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
        {
            /* Roll back to the start of buffer */
            IsocOutRdPtr = IsocOutBuff;
        }
        else
        {
            /* Increment to the next sub-buffer */
            IsocOutRdPtr += AUDIO_OUT_PACKET;
        }

        /* If all available buffers have been consumed, stop playing */
        if (IsocOutRdPtr == IsocOutWrPtr)
        {
            /* Pause the audio stream */
            AUDIO_OUT_fops.AudioCmd((uint8_t*)(IsocOutBuff),   /* Samples buffer pointer */
                                    AUDIO_OUT_PACKET,          /* Number of samples in Bytes */
                                    AUDIO_CMD_PAUSE);          /* Command to be processed */

            /* Stop entering play loop */
            PlayFlag = 0;

            /* Reset buffer pointers */
            IsocOutRdPtr = IsocOutBuff;
            IsocOutWrPtr = IsocOutBuff;
        }
    }

    return USBD_OK;
}

/**
  * @brief  Handle standard device request--Get Interface
  * @param  pudev: pointer to usb device instance
  * @param  req: standard device request
  * @retval usb device operation status
  */
static void USBD_AUDIO_GetInterface (usb_dev *pudev, usb_req *req)
{
    pudev->dev.transc_in[0].xfer_buf = (uint8_t *)&USBD_AUDIO_AltSet;
    pudev->dev.transc_in[0].remain_len = 1;
    usbd_ctl_send(pudev);
}

/**
  * @brief  Handle standard device request--Set Interface
  * @param  pudev: pointer to usb device instance
  * @param  req: standard device request
  * @retval usb device operation status
  */
static void USBD_AUDIO_SetInterface (usb_dev *pudev, usb_req *req)
{
    if ((uint8_t)(req->wValue) < AUDIO_TOTAL_IF_NUM)
    {
        USBD_AUDIO_AltSet = (uint8_t)(req->wValue);
    }
    else
    {
        usbd_ep_stall(pudev,0x80);
        usbd_ep_stall(pudev,0x00);
        usb_ctlep_startout(pudev);
    }
}

/**
  * @brief  Handles the GET_CUR Audio control request.
  * @param  pudev: pointer to usb device instance
  * @param  req: setup class request
  * @retval usb device operation status
  */
static void AUDIO_Req_GetCurrent(usb_dev *pudev, usb_req *req)
{
    /* Send the current mute state */
    pudev->dev.transc_in[0].xfer_buf = AudioCtl;
    pudev->dev.transc_in[0].remain_len = req->wLength;
    usbd_ctl_send(pudev);
}

/**
  * @brief  Handles the SET_CUR Audio control request.
  * @param  pudev: pointer to usb device instance
  * @param  req: setup class request
  * @retval usb device operation status
  */
static void AUDIO_Req_SetCurrent(usb_dev *pudev, usb_req *req)
{
    if (req->wLength)
    {
        /* Prepare the reception of the buffer over EP0 */
        pudev->dev.transc_out[0].xfer_buf = AudioCtl;
        pudev->dev.transc_out[0].remain_len = req->wLength;
        usbd_ctl_recev(pudev);
        
        /* Set the global variables indicating current request and its length 
        to the function usbd_audio_EP0_RxReady() which will process the request */
        AudioCtlCmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
        AudioCtlLen = req->wLength;          /* Set the request data length */
        AudioCtlUnit = BYTE_HIGH(req->wIndex);  /* Set the request target unit */
    }
}

usb_class_core usbd_audio_cb = {
    .command         = NO_CMD,
    .alter_set       = 0,

    .init            = audio_init,
    .deinit          = audio_deinit,
    .req_proc        = audio_req_handler,
    .data_in         = audio_data_in_handler,
    .data_out        = audio_data_out_handler,
    .SOF             = USBD_AUDIO_SOF
};
