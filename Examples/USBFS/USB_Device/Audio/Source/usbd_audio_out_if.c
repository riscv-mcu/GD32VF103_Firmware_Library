/*!
    \file  usbd_audio_out_if.c
    \brief This file provides the Audio Out (palyback) interface API

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

/* Includes ------------------------------------------------------------------*/
#include "audio_core.h"
#include "usbd_audio_out_if.h"

/** @addtogroup GD32VF103_Firmware
  * @{
  */

/** @addtogroup USB_OTG_FS
  * @{
  */

/** @addtogroup USB_OTG_FS_Device_Class_Library
  * @{
  */

/** @defgroup USBD_AUDIO
  * @{
  */
static uint8_t  Init         (uint32_t AudioFreq, uint32_t Volume, uint32_t Options);
static uint8_t  DeInit       (uint32_t Options);
static uint8_t  AudioCmd     (uint8_t* pbuf, uint32_t Size, uint8_t Cmd);
static uint8_t  VolumeCtl    (uint8_t Vol);
static uint8_t  MuteCtl      (uint8_t Cmd);
static uint8_t  PeriodicTC   (uint8_t Cmd);
static uint8_t  GetState     (void);

/** @defgroup USBD_AUDIO_out_if_Private_Variables
  * @{
  */
AUDIO_FOPS_TypeDef  AUDIO_OUT_fops = 
{
    Init,
    DeInit,
    AudioCmd,
    VolumeCtl,
    MuteCtl,
    PeriodicTC,
    GetState
};

static uint8_t AudioState = AUDIO_STATE_INACTIVE;

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_out_if_Private_Functions
  * @{
  */

/**
  * @brief  Initialize and configures all required resources for audio play function.
  * @param  AudioFreq: Statrtup audio frequency. 
  * @param  Volume: Startup volume to be set.
  * @param  options: specific options passed to low layer function.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  Init (uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
    static uint32_t Initialized = 0;

    /* Check if the low layer has already been initialized */
    if (Initialized == 0)
    {
        /* Call low layer function */
        if (EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, Volume, AudioFreq) != 0)
        {
            AudioState = AUDIO_STATE_ERROR;
            return AUDIO_FAIL;
        }

        /* Set the Initialization flag to prevent reinitializing the interface again */
        Initialized = 1;
    }

    /* Update the Audio state machine */
    AudioState = AUDIO_STATE_ACTIVE;

    return AUDIO_OK;
}

/**
  * @brief  Free all resources used by low layer and stops audio-play function.
  * @param  Options: options passed to low layer function.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  DeInit (uint32_t Options)
{
    /* Update the Audio state machine */
    AudioState = AUDIO_STATE_INACTIVE;

    return AUDIO_OK;
}

/**
  * @brief  Play, Stop, Pause or Resume current file.
  * @param  pbuf: address from which file shoud be played.
  * @param  Size: size of the current buffer/file.
  * @param  Cmd: command to be executed, can be:
                 AUDIO_CMD_PLAY
                 AUDIO_CMD_PAUSE
                 AUDIO_CMD_RESUME
                 AUDIO_CMD_STOP.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  AudioCmd (uint8_t* pbuf, uint32_t Size, uint8_t Cmd)
{
    /* Check the current state */
    if ((AudioState == AUDIO_STATE_INACTIVE) || (AudioState == AUDIO_STATE_ERROR))
    {
        AudioState = AUDIO_STATE_ERROR;

        return AUDIO_FAIL;
    }

    switch (Cmd)
    {
        /* Process the PLAY command ----------------------------*/
        case AUDIO_CMD_PLAY:
            /* If current state is Active or Stopped */
            if ((AudioState == AUDIO_STATE_ACTIVE) || \
                (AudioState == AUDIO_STATE_STOPPED) || \
                (AudioState == AUDIO_STATE_PLAYING))
            {
                Audio_MAL_Play((uint32_t)pbuf, (Size/2));
                AudioState = AUDIO_STATE_PLAYING;

                return AUDIO_OK;
            }
            /* If current state is Paused */
            else if (AudioState == AUDIO_STATE_PAUSED)
            {
                if (EVAL_AUDIO_PauseResume(AUDIO_RESUME, (uint32_t)pbuf, (Size/2)) != 0)
                {
                    AudioState = AUDIO_STATE_ERROR;

                    return AUDIO_FAIL;
                }
                else
                {
                    AudioState = AUDIO_STATE_PLAYING;

                    return AUDIO_OK;
                }
            }
            else /* Not allowed command */
            {
                return AUDIO_FAIL;
            }

        /* Process the STOP command ----------------------------*/
        case AUDIO_CMD_STOP:
            if (AudioState != AUDIO_STATE_PLAYING)
            {
                /* Unsupported command */

                return AUDIO_FAIL;
            }
            else if (EVAL_AUDIO_Stop(CODEC_PDWN_SW) != 0)
            {
                AudioState = AUDIO_STATE_ERROR;

                return AUDIO_FAIL;
            }
            else
            {
                AudioState = AUDIO_STATE_STOPPED;

                return AUDIO_OK;
            }

        /* Process the PAUSE command ---------------------------*/
        case AUDIO_CMD_PAUSE:
            if (AudioState != AUDIO_STATE_PLAYING)
            {
                /* Unsupported command */
                return AUDIO_FAIL;
            }
            else if (EVAL_AUDIO_PauseResume(AUDIO_PAUSE, (uint32_t)pbuf, (Size/2)) != 0)
            {
                AudioState = AUDIO_STATE_ERROR;

                return AUDIO_FAIL;
            }
            else
            {
                AudioState = AUDIO_STATE_PAUSED;

                return AUDIO_OK;
            }

        /* Unsupported command ---------------------------------*/
        default:
            return AUDIO_FAIL;
    }
}

/**
  * @brief  Set the volume level in %
  * @param  Vol: volume level to be set in % (from 0% to 100%)
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  VolumeCtl (uint8_t Vol)
{
    /* Call low layer volume setting function */  
    if (EVAL_AUDIO_VolumeCtl(Vol) != 0)
    {
        AudioState = AUDIO_STATE_ERROR;

        return AUDIO_FAIL;
    }

    return AUDIO_OK;
}

/**
  * @brief  Mute or Unmute the audio current output
  * @param  Cmd: can be 0 to unmute, or 1 to mute.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  MuteCtl (uint8_t Cmd)
{
    /* Call low layer mute setting function */  
    if (EVAL_AUDIO_Mute(Cmd) != 0)
    {
        AudioState = AUDIO_STATE_ERROR;

        return AUDIO_FAIL;
    }

    return AUDIO_OK;
}

/**
  * @brief
  * @param  Cmd
  * @param
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  PeriodicTC (uint8_t Cmd)
{
    return AUDIO_OK;
}

/**
  * @brief  Return the current state of the audio machine
  * @param  None
  * @retval Current State.
  */
static uint8_t  GetState (void)
{
    return AudioState;
}
