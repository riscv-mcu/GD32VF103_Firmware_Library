/*!
    \file  gd32vf103_audio_codec.h
    \brief the audio codec low layer driver

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

#ifndef __GD32VF103_AUDIO_CODEC_H
#define __GD32VF103_AUDIO_CODEC_H

#include "gd32vf103.h"

/* CONFIGURATION: Audio Codec Driver Configuration parameters */
//#define AUDIO_USE_MACROS

/* Audio Transfer mode (DMA, Interrupt or Polling) */
#define AUDIO_MAL_MODE_NORMAL         /* Uncomment this line to enable the audio 
                                         Transfer using DMA */
/* #define AUDIO_MAL_MODE_CIRCULAR */ /* Uncomment this line to enable the audio 
                                         Transfer using DMA */

/* For the DMA modes select the interrupt that will be used */
/* #define AUDIO_MAL_DMA_IT_TC_EN */  /* Uncomment this line to enable DMA Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_HT_EN */  /* Uncomment this line to enable DMA Half Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_TE_EN */  /* Uncomment this line to enable DMA Transfer Error interrupt */

/* #define USE_DMA_PAUSE_FEATURE *//* Uncomment this line to enable the use of DMA Pause Feature
                                 When this define is enabled, the Pause function will
                                 just pause/resume the DMA without need to reinitialize the
                                 DMA structure. In this case, the buffer pointer should remain
                                 available after resume. */

/* Select the interrupt preemption priority and subpriority for the DMA interrupt */
#define EVAL_AUDIO_IRQ_PREPRIO           0   /* Select the preemption priority level(0 is the highest) */
#define EVAL_AUDIO_IRQ_SUBRIO            0   /* Select the sub-priority level (0 is the highest) */

/* Uncomment the following line to use the default Codec_TIMEOUT_UserCallback() 
   function implemented in gd3210c_audio_codec.c file.
   Codec_TIMEOUT_UserCallback() function is called whenever a timeout condition 
   occurs during communication (waiting on an event that doesn't occur, bus 
   errors, busy devices ...). */   
/* #define USE_DEFAULT_TIMEOUT_CALLBACK */


/* OPTIONAL Configuration defines parameters */

/* Uncomment defines below to select standard for audio communication between Codec and I2S peripheral */
/* #define I2S_STANDARD_PHILLIPS */
/* #define I2S_STANDARD_MSB */
#define I2S_STANDARD_LSB

/* Uncomment the defines below to select if the Master clock mode should be enabled or not */
#define CODEC_MCLK_ENABLED
/* #deine CODEC_MCLK_DISABLED */

/* Uncomment this line to enable verifying data sent to codec after each write operation */
/* #define VERIFY_WRITTENDATA */

/* Hardware Configuration defines parameters */

/* I2S peripheral configuration defines */
#define CODEC_I2S                      SPI1
#define CODEC_I2S_CLK                  RCU_SPI1
#define CODEC_I2S_ADDRESS              0x4000380C
#define CODEC_I2S_IRQ                  SPI1_IRQn
#define CODEC_I2S_GPIO1_CLOCK          RCU_GPIOB
#define CODEC_I2S_GPIO2_CLOCK          RCU_GPIOC
#define CODEC_I2S_WS_PIN               GPIO_PIN_12
#define CODEC_I2S_SCK_PIN              GPIO_PIN_13
#define CODEC_I2S_SD_PIN               GPIO_PIN_15
#define CODEC_I2S_MCK_PIN              GPIO_PIN_6
#define CODEC_I2S_GPIO                 GPIOB
#define CODEC_I2S_MCK_GPIO             GPIOC

/* I2S DMA Stream definitions */
#define AUDIO_MAL_DMA_CLOCK            RCU_DMA0
#define AUDIO_MAL_DMA_CHANNEL          DMA_CH4
#define AUDIO_MAL_DMA_IRQ              DMA0_Channel4_IRQn
#define AUDIO_MAL_DMA_FLAG_ALL         DMA_FLAG_G
#define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PERIPHERALDATASIZE_HALFWORD
#define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MEMORYDATASIZE_HALFWORD
#define DMA_MAX_SZE                    0xFFFF
#define AUDIO_MAL_DMA                  DMA0

#define Audio_MAL_IRQHandler           DMA0_Channel4_IRQHandler

/* SPI peripheral configuration defines (control interface of the audio codec) */
#define CODEC_SPI                      SPI0
#define CODEC_SPI_CLK                  RCU_SPI0
#define CODEC_SPI_GPIO_CLOCK           RCU_GPIOA
#define CODEC_SPI_GPIO_AF              GPIO_AF
#define CODEC_SPI_GPIO                 GPIOA
#define CODEC_SPI_SCL_PIN              GPIO_PIN_5
#define CODEC_SPI_SDA_PIN              GPIO_PIN_7
#define CODEC_SPI_SEL_PIN              GPIO_PIN_4

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...) */
#define CODEC_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))

/* Audio Codec User defines */

/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER         1
#define OUTPUT_DEVICE_HEADPHONE       2
#define OUTPUT_DEVICE_BOTH            3
#define OUTPUT_DEVICE_AUTO            4

/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04

#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

#define VOLUME_CONVERT(x)    ((Volume > 100)? 100:((uint8_t)((Volume * 255) / 100)))
#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)


/* Generic functions */
uint32_t EVAL_AUDIO_Init        (uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
uint32_t EVAL_AUDIO_DeInit      (void);
uint32_t EVAL_AUDIO_Play        (uint16_t* pBuffer, uint32_t Size);
uint32_t EVAL_AUDIO_PauseResume (uint32_t Cmd, uint32_t Addr, uint32_t Size);
uint32_t EVAL_AUDIO_Stop        (uint32_t CodecPowerDown_Mode);
uint32_t EVAL_AUDIO_VolumeCtl   (uint8_t Volume);
uint32_t EVAL_AUDIO_Mute        (uint32_t Command);

/* Audio Codec functions */

/* High Layer codec functions */
uint32_t Codec_Init             (uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
uint32_t Codec_DeInit           (void);
uint32_t Codec_Play             (void);
uint32_t Codec_PauseResume      (uint32_t Cmd);
uint32_t Codec_Stop             (uint32_t Cmd);
uint32_t Codec_VolumeCtrl       (uint8_t Volume);
uint32_t Codec_Mute             (uint32_t Cmd);
uint32_t Codec_SwitchOutput     (uint8_t Output);

/* MAL (Media Access Layer) functions */
void     Audio_MAL_Init         (void);
void     Audio_MAL_DeInit       (void);
void     Audio_MAL_Play         (uint32_t Addr, uint32_t Size);
void     Audio_MAL_PauseResume  (uint32_t Cmd, uint32_t Addr, uint32_t Size);
void     Audio_MAL_Stop         (void);

/* User Callbacks: user has to implement these functions in his code if they are needed. */

/* This function is called when the requested data has been completely transferred.
   In Normal mode (when  the define AUDIO_MAL_MODE_NORMAL is enabled) this function
   is called at the end of the whole audio file.
   In circular mode (when  the define AUDIO_MAL_MODE_CIRCULAR is enabled) this 
   function is called at the end of the current buffer transmission. */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size);

/* This function is called when half of the requested buffer has been transferred 
   This callback is useful in Circular mode only (when AUDIO_MAL_MODE_CIRCULAR 
   define is enabled)*/
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size);

/* This function is called when an Interrupt due to transfer error on or peripheral
   error occurs. */
void EVAL_AUDIO_Error_CallBack(void* pData);

/* Codec_TIMEOUT_UserCallback() function is called whenever a timeout condition 
   occurs during communication (waiting on an event that doesn't occur, bus 
   errors, busy devices ...) on the Codec control interface (I2C).
   You can use the default timeout callback implementation by uncommenting the 
   define USE_DEFAULT_TIMEOUT_CALLBACK in audio_codec.h file.
   Typically the user implementation of this callback should reset I2C peripheral
   and re-initialize communication or in worst case reset all the application. */
uint32_t Codec_TIMEOUT_UserCallback(void);
 
#endif /* __GD32VF103_USB_AUDIOCODEC_H */
