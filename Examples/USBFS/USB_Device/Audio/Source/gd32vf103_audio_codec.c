/*!
    \file  gd3210c_audio_codec.c
    \brief This file includes the low layer driver for CS43L22 Audio Codec

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
#include "gd32vf103_audio_codec.h"
/* Private define ------------------------------------------------------------*/

/* Mask for the bit EN of the I2S CFGR register */
#define I2S_ENABLE_MASK                 (0x0400)

/* Delay for the Codec to be correctly reset */
#define CODEC_RESET_DELAY               (0x4FFF)

/* Codec audio Standards */
#ifdef I2S_STANDARD_PHILLIPS
    #define CODEC_STANDARD                 0x04
    #define I2S_STANDARD                   I2S_STD_PHILLIPS
#elif defined(I2S_STANDARD_MSB)
    #define CODEC_STANDARD                 0x00
    #define I2S_STANDARD                   I2S_STD_MSB
#elif defined(I2S_STANDARD_LSB)
    #define CODEC_STANDARD                 0x08
    #define I2S_STANDARD                   I2S_STD_LSB
#else 
    #error "Error: No audio communication standard selected !"
#endif /* I2S_STANDARD */

/* The 7 bits Codec adress (sent through I2C interface) */
#define CODEC_ADDRESS                      0x94  /* b00100111 */

/* Private variables ---------------------------------------------------------*/
/* This structure is declared glabal because it is handled by two different functions */
static dma_parameter_struct DMA_InitStructure;
static uint8_t OutputDev = 0;

uint32_t AudioTotalSize = 0xFFFF; /* This variable holds the total size of the audio file */
uint32_t AudioRemSize   = 0xFFFF; /* This variable holds the remaining data in audio file */
uint16_t *CurrentPos;             /* This variable holds the current poisition of audio pointer */
uint32_t I2S_AudioFreq = 0;


/* Private functions ---------------------------------------------------------*/

/* Audio Codec functions */

/* Low layer codec functions */
static void     Codec_CtrlInterface_Init    (void);
static void     Codec_CtrlInterface_DeInit  (void);
static void     Codec_AudioInterface_Init   (uint32_t AudioFreq);
static void     Codec_AudioInterface_DeInit (void);
static void     Codec_Reset                 (void);
static uint32_t Codec_WriteRegister         (uint32_t RegisterAddr, uint32_t RegisterValue);
static void     Codec_GPIO_Init             (void);
static void     Codec_GPIO_DeInit           (void);
static void     Delay                       (__IO uint32_t nCount);


/**
  * @brief  Configure the audio peripherals.
  * @param  OutputDevice:
  *   This parameter can be any one of the following values:
  *     @arg OUTPUT_DEVICE_SPEAKER
  *     @arg OUTPUT_DEVICE_HEADPHONE
  *     @arg OUTPUT_DEVICE_BOTH
  *     @arg OUTPUT_DEVICE_AUTO
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to paly the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
    /* Perform low layer Codec initialization */
    if (Codec_Init(OutputDevice, VOLUME_CONVERT(Volume), AudioFreq) != 0)
    {
        return 1;
    }
    else
    {
        /* I2S data transfer preparation:
           Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
        Audio_MAL_Init();

        /* Return 0 when all operations are OK */
        return 0;
    }
}

/**
  * @brief  Dinitializes all the resources used by the codec (those initialized 
  *         by EVAL_AUDIO_Init() function) EXCEPT the I2C resources since they are 
  *         used by the IOExpander as well (and eventually other modules). 
  * @param  None.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_DeInit(void)
{
    /* DeInitialize the Media layer */
    Audio_MAL_DeInit();

    /* DeInitialize Codec */
    Codec_DeInit();

    return 0;
}

/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data BYTES.
  * @retval OK if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Play(uint16_t* pBuffer, uint32_t Size)
{
    /* Set the total number of data to be played (count in half-word) */
    AudioTotalSize = Size/2;

    /* Call the audio Codec Play function */
    Codec_Play();

    /* Update the Media layer and enable it for play */  
    Audio_MAL_Play((uint32_t)pBuffer, (uint32_t)(DMA_MAX(AudioTotalSize / 2)));

    /* Update the remaining number of data to be played */
    AudioRemSize = (Size/2) - DMA_MAX(AudioTotalSize);

    /* Update the current audio pointer position */
    CurrentPos = pBuffer + DMA_MAX(AudioTotalSize);

    return 0;
}

/**
  * @brief  This function Pauses or Resumes the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used. In all cases the I2S 
  *         peripheral is disabled. 
  * @WARNING When calling EVAL_AUDIO_PauseResume() function for pause, only
  *         this function should be called for resume (use of EVAL_AUDIO_Play() 
  *         function for resume could lead to unexpected behaviour).
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @param  Addr: Address from/at which the audio stream should resume/pause.
  * @param  Size: Number of data to be configured for next resume.
  * @retval o if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_PauseResume(uint32_t Cmd, uint32_t Addr, uint32_t Size)
{
    if (Cmd != AUDIO_PAUSE)
    {
        /* Call the Media layer pause/resume function */
        Audio_MAL_PauseResume(Cmd, Addr, Size);

        /* Call the Audio Codec Pause/Resume function */
        if (Codec_PauseResume(Cmd) != 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        /* Call the Audio Codec Pause/Resume function */
        if (Codec_PauseResume(Cmd) != 0)
        {
            return 1;
        }
        else
        {
            /* Call the Media layer pause/resume function */
            Audio_MAL_PauseResume(Cmd, Addr, Size);

            /* Return 0 if all operations are OK */
            return 0;
        }
    }
}

/**
  * @brief  Stops audio playing and Power down the Audio Codec.
  * @param  Option:
  *   This parameter can be any one of the following values:
  *     @arg CODEC_PDWN_SW for software power off (by writing registers)
             Then no need to reconfigure the Codec after power on.
  *     @arg CODEC_PDWN_HW completely shut down the codec (physically). Then need 
  *          to reconfigure the Codec after power on.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Stop(uint32_t Option)
{
    /* Call Audio Codec Stop function */
    if (Codec_Stop(Option) != 0)
    {
        return 1;
    }
    else
    {
        /* Call Media layer Stop function */
        Audio_MAL_Stop();

        /* Update the remaining data number */
        AudioRemSize = AudioTotalSize;    

        /* Return 0 when all operations are correctly done */
        return 0;
    }
}

/**
  * @brief Controls the current audio volume level. 
  * @param Volume: Volume level to be set in percentage from 0% to 100% (0 for 
  *        Mute and 100 for Max volume level).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_VolumeCtl(uint8_t Volume)
{
    /* Call the codec volume control function with converted volume value */
    return (Codec_VolumeCtrl(VOLUME_CONVERT(Volume)));
}

/**
  * @brief Enable or disable the MUTE mode by software 
  * @param Cmd: could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to 
  *        unmute the codec and restore previous volume level.
  * @retval o if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Mute(uint32_t Cmd)
{
    /* Call the Codec Mute function */
    return (Codec_Mute(Cmd));
}

/**
  * @brief  This function handles main Media layer interrupt. 
  * @param  None.
  * @retval 0 if correct communication, else wrong communication
  */
void Audio_MAL_IRQHandler(void)
{    
#ifndef AUDIO_MAL_MODE_NORMAL
    uint16_t *pAddr = (uint16_t *)CurrentPos;
    uint32_t Size = AudioRemSize;
#endif /* AUDIO_MAL_MODE_NORMAL */
}

/* CS43L22 Audio Codec Control Functions */

/**
  * @brief  Initializes the audio codec and all related interfaces (control 
  *         interface: I2C and audio interface: I2S)
  * @param  OutputDevice:
  *   This parameter can be any one of the following values:
  *     @arg OUTPUT_DEVICE_SPEAKER
  *     @arg OUTPUT_DEVICE_HEADPHONE
  *     @arg OUTPUT_DEVICE_BOTH
  *     @arg OUTPUT_DEVICE_AUTO
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to paly the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
    uint32_t counter = 0;

    /* Configure the Codec related IOs */
    Codec_GPIO_Init();

    /* Reset the Codec Registers */
    Codec_Reset();

    /* Initialize the Control interface of the Audio Codec */
    Codec_CtrlInterface_Init();

    /* Keep Codec powered OFF */
    counter += Codec_WriteRegister(0x02, 0x01);

    switch (OutputDevice)
    {
        case OUTPUT_DEVICE_SPEAKER:
            counter += Codec_WriteRegister(0x04, 0xFA); /* SPK always ON & HP always OFF */
            OutputDev = 0xFA;
            break;

        case OUTPUT_DEVICE_HEADPHONE:
            counter += Codec_WriteRegister(0x04, 0xAF); /* SPK always OFF & HP always ON */
            OutputDev = 0xAF;
            break;

        case OUTPUT_DEVICE_BOTH:
            counter += Codec_WriteRegister(0x04, 0xAA); /* SPK always ON & HP always ON */
            OutputDev = 0xAA;
            break;

        case OUTPUT_DEVICE_AUTO:
            counter += Codec_WriteRegister(0x04, 0x05); /* Detect the HP or the SPK automatically */
            OutputDev = 0x05;
            break;

        default:
            counter += Codec_WriteRegister(0x04, 0x05); /* Detect the HP or the SPK automatically */
            OutputDev = 0x05;
            break;
    }

    /* Clock configuration: Auto detection */  
    counter += Codec_WriteRegister(0x05, 0x81);

    /* Set the Slave Mode and the audio Standard */  
    counter += Codec_WriteRegister(0x06, CODEC_STANDARD);

    /* Set the Master volume */
    Codec_VolumeCtrl(Volume);

    /* If the Speaker is enabled, set the Mono mode and volume attenuation level */
    if (OutputDevice != OUTPUT_DEVICE_HEADPHONE)
    {
        /* Set the Speaker Mono mode */
        counter += Codec_WriteRegister(0x0F , 0x06);

        /* Set the Speaker attenuation level */
        counter += Codec_WriteRegister(0x24, 0x00);
        counter += Codec_WriteRegister(0x25, 0x00);
    }

    /* Power on the Codec */
    counter += Codec_WriteRegister(0x02, 0x9E);

    /* Additional configuration for the CODEC. These configurations are done to reduce
       the time needed for the Codec to power off. If these configurations are removed,
       then a long delay should be added between powering off the Codec and switching
       off the I2S peripheral MCLK clock (which is the operating clock for Codec).
       If this delay is not inserted, then the codec will not shut down propoerly and
       it results in high noise after shut down. */

    /* Disable the analog soft ramp */
    counter += Codec_WriteRegister(0x0A, 0x00);

    /* Disable the digital soft ramp */
    counter += Codec_WriteRegister(0x0E, 0x04);

    /* Disable the limiter attack level */
    counter += Codec_WriteRegister(0x27, 0x00);

    /* Adjust Bass and Treble levels */
    counter += Codec_WriteRegister(0x1F, 0x0F);

    /* Adjust PCM volume level */
    counter += Codec_WriteRegister(0x1A, 0x0A);
    counter += Codec_WriteRegister(0x1B, 0x0A);

    /* Configure the I2S peripheral */
    Codec_AudioInterface_Init(AudioFreq);

    /* Return communication control value */
    return counter;
}

/**
  * @brief  Restore the audio codec state to default state and free all used 
  *         resources.
  * @param  None.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_DeInit(void)
{
    uint32_t counter = 0;

    /* Reset the Codec Registers */
    Codec_Reset();

    /* Keep Codec powered OFF */
    counter += Codec_WriteRegister(0x02, 0x01);    

    /* Deinitialize all use GPIOs */
    Codec_GPIO_DeInit();

    /* Disable the Codec control interface */
    Codec_CtrlInterface_DeInit();

    /* Deinitialize the Codec audio interface (I2S) */
    Codec_AudioInterface_DeInit(); 

    /* Return communication control value */
    return counter;  
}

/**
  * @brief  Start the audio Codec play feature.
  *         For this codec no Play options are required.
  * @param  None.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_Play(void)
{
    /* No actions required on Codec level for play command */

    /* Return communication control value */
    return 0;
}

/**
  * @brief  Pauses and resumes playing on the audio codec.
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_PauseResume(uint32_t Cmd)
{
    uint32_t counter = 0;

    /* Pause the audio file playing */
    if (Cmd == AUDIO_PAUSE)
    {
        /* Mute the output first */
        counter += Codec_Mute(AUDIO_MUTE_ON);

        /* Put the Codec in Power save mode */
        counter += Codec_WriteRegister(0x02, 0x01);
    }
    else /* AUDIO_RESUME */
    {
        /* Unmute the output first */
        counter += Codec_Mute(AUDIO_MUTE_OFF);

        counter += Codec_WriteRegister(0x04, OutputDev);

        /* Exit the Power save mode */
        counter += Codec_WriteRegister(0x02, 0x9E); 
    }

    return counter;
}

/**
  * @brief  Stops audio Codec playing. It powers down the codec.
  * @param  CodecPdwnMode:
  *   This parameter can be any one of the following values:
  *     @arg CODEC_PDWN_SW: only mutes the audio codec. When resuming from this 
  *          mode the codec keeps the prvious initialization (no need to re-Initialize
  *          the codec registers)
  *     @arg CODEC_PDWN_HW: Physically power down the codec. When resuming from this
  *          mode, the codec is set to default configuration (user should re-Initialize
  *          the codec in order to play again the audio stream).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_Stop(uint32_t CodecPdwnMode)
{
    uint32_t counter = 0;   

    /* Mute the output first */
    Codec_Mute(AUDIO_MUTE_ON);

    if (CodecPdwnMode == CODEC_PDWN_SW)
    {
        /* Power down the DAC and the speaker (PMDAC and PMSPK bits)*/
        counter += Codec_WriteRegister(0x02, 0x9F);
    }
    else /* CODEC_PDWN_HW */
    {
        /* Power down the DAC components */
        counter += Codec_WriteRegister(0x02, 0x9F);

        /* Wait at least 100ms */
        Delay(0xFFF);

        /* Reset The pin */
        //IOE_WriteIOPin(AUDIO_RESET_PIN, BitReset);
    }

    return counter;
}

/**
  * @brief  Highers or Lowers the codec volume level.
  * @param  Volume: a byte value from 0 to 255 (refer to codec registers 
  *         description for more details).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_VolumeCtrl(uint8_t Volume)
{
    uint32_t counter = 0;

    if (Volume > 0xE6)
    {
        /* Set the Master volume */
        counter += Codec_WriteRegister(0x20, Volume - 0xE7);
        counter += Codec_WriteRegister(0x21, Volume - 0xE7);
    }
    else
    {
        /* Set the Master volume */
        counter += Codec_WriteRegister(0x20, Volume + 0x19);
        counter += Codec_WriteRegister(0x21, Volume + 0x19);
    }

    return counter;
}

/**
  * @brief  Enables or disables the mute feature on the audio codec.
  * @param  Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
  *              mute mode.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_Mute(uint32_t Cmd)
{
    uint32_t counter = 0;

    /* Set the Mute mode */
    if (Cmd == AUDIO_MUTE_ON)
    {
        counter += Codec_WriteRegister(0x04, 0xFF);

        //counter += Codec_WriteRegister(0x0D, 0x63);
        //counter += Codec_WriteRegister(0x0F, 0xF6);
    }
    else /* AUDIO_MUTE_OFF Disable the Mute */
    {
        counter += Codec_WriteRegister(0x04, OutputDev);

        //counter += Codec_WriteRegister(0x0D, 0x60);
        //counter += Codec_WriteRegister(0x0F, 0x06);
    }

    return counter;
}

/**
  * @brief  Resets the audio codec. It restores the default configuration of the 
  *         codec (this function shall be called before initializing the codec).
  * @note   This function calls an external driver function: The IO Expander driver.
  * @param  None.
  * @retval 0 if correct communication, else wrong communication
  */
static void Codec_Reset(void)
{
    /* Configure the IO Expander (to use the Codec Reset pin mapped on the IOExpander) */
    //IOE_Config();

    /* Power Down the codec */
    //IOE_WriteIOPin(AUDIO_RESET_PIN, BitReset);

    /* wait for a delay to insure registers erasing */
    //Delay(CODEC_RESET_DELAY); 

    /* Power on the codec */
    //IOE_WriteIOPin(AUDIO_RESET_PIN, BitSet);
}

/**
  * @brief  Switch dynamically (while audio file is played) the output target (speaker or headphone).
  * @note   This function modifies a global variable of the audio codec driver: OutputDev.
  * @param  None.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t Codec_SwitchOutput(uint8_t Output)
{
    uint8_t counter = 0;

    switch (Output) 
    {
        case OUTPUT_DEVICE_SPEAKER:
            counter += Codec_WriteRegister(0x04, 0xFA); /* SPK always ON & HP always OFF */
            OutputDev = 0xFA;
            break;

        case OUTPUT_DEVICE_HEADPHONE:
            counter += Codec_WriteRegister(0x04, 0xAF); /* SPK always OFF & HP always ON */
            OutputDev = 0xAF;
            break;

        case OUTPUT_DEVICE_BOTH:
            counter += Codec_WriteRegister(0x04, 0xAA); /* SPK always ON & HP always ON */
            OutputDev = 0xAA;
            break;

        case OUTPUT_DEVICE_AUTO:
            counter += Codec_WriteRegister(0x04, 0x05); /* Detect the HP or the SPK automatically */
            OutputDev = 0x05;
            break;    

        default:
            counter += Codec_WriteRegister(0x04, 0x05); /* Detect the HP or the SPK automatically */
            OutputDev = 0x05;
            break;
    }

    return counter;
}

/**
  * @brief  Writes a Byte to a given register into the audio codec through the 
            control interface (I2C)
  * @param  RegisterAddr: The address (location) of the register to be written.
  * @param  RegisterValue: the Byte value to be written into destination register.
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_WriteRegister(uint32_t RegisterAddr, uint32_t RegisterValue)
{
    uint32_t result = 0;

    /* Return the verifying value: 0 (Passed) or 1 (Failed) */
    return result;
}

/**
  * @brief Initializes the Audio Codec control interface (I2C).
  * @param  None.
  * @retval None.
  */
static void Codec_CtrlInterface_Init(void)
{

}

/**
  * @brief  Restore the Audio Codec control interface to its default state.
  *         This function doesn't de-initialize the I2C because the I2C peripheral
  *         may be used by other modules.
  * @param  None.
  * @retval None.
  */
static void Codec_CtrlInterface_DeInit(void)
{
    /* Disable the I2C peripheral */ 
    /* This step is not done here because the I2C interface can be used by other modules */
    /* I2C_DeInit(CODEC_I2C); */
}

/**
  * @brief  Initializes the Audio Codec audio interface (I2S)
  * @note   This function assumes that the I2S input clock (through PLL_R in 
  *         Devices RevA/Z and through dedicated PLLI2S_R in Devices RevB/Y)
  *         is already configured and ready to be used.    
  * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral. 
  * @retval None.
  */
static void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
    I2S_AudioFreq = AudioFreq;

    /* Enable the CODEC_I2S peripheral clock */
    rcu_periph_clock_enable(CODEC_I2S_CLK);

    /* CODEC_I2S peripheral configuration */
    spi_i2s_deinit(CODEC_I2S);

    /* CODEC_I2S peripheral configuration */
    i2s_psc_config(CODEC_I2S, AudioFreq, I2S_FRAMEFORMAT_DT16B_CH16B, 
#ifdef CODEC_MCLK_ENABLED
    I2S_MCKOUT_ENABLE
#elif defined(CODEC_MCLK_DISABLED)
    I2S_MCKOUT_DISABLE
#endif
    );

    i2s_init(CODEC_I2S, I2S_MODE_MASTERTX, I2S_STD_LSB, I2S_CKPL_LOW);

    /* Enable the I2S DMA TX request */
    spi_dma_enable(CODEC_I2S, SPI_DMA_TRANSMIT);
    /* The I2S peripheral will be enabled only in the EVAL_AUDIO_Play() function 
       or by user functions if DMA mode not enabled */  
}

/**
  * @brief Restores the Audio Codec audio interface to its default state.
  * @param  None.
  * @retval None.
  */
static void Codec_AudioInterface_DeInit(void)
{
    /* Disable the CODEC_I2S peripheral (in case it hasn't already been disabled) */
    i2s_disable(CODEC_I2S);

    /* Deinitialize the CODEC_I2S peripheral */
    spi_i2s_deinit(CODEC_I2S);

    /* Disable the CODEC_I2S peripheral clock */
    rcu_periph_clock_disable(CODEC_I2S_CLK);
}

/**
  * @brief Initializes IOs used by the Audio Codec (on the control and audio 
  *        interfaces).
  * @param  None.
  * @retval None.
  */
static void Codec_GPIO_Init(void)
{
    /* enable the clock */
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(CODEC_I2S_GPIO1_CLOCK);
    rcu_periph_clock_enable(CODEC_I2S_GPIO2_CLOCK);
    rcu_periph_clock_enable(CODEC_SPI_CLK);

    /* CODEC_I2C SCL and SDA pins configuration */
    gpio_init(CODEC_SPI_GPIO, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, CODEC_SPI_SCL_PIN | CODEC_SPI_SDA_PIN);

    /* CODEC_I2S pins configuraiton: WS, SCK and SD pins */
    gpio_init(CODEC_I2S_GPIO, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, CODEC_I2S_WS_PIN | CODEC_I2S_SCK_PIN | CODEC_I2S_SD_PIN);

#ifdef CODEC_MCLK_ENABLED
    /* CODEC_I2S pins configuraiton: MCK pin */
    gpio_init(CODEC_I2S_MCK_GPIO, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, CODEC_I2S_MCK_PIN);
#endif /* CODEC_MCLK_ENABLED */
}

/**
  * @brief  Restores the IOs used by the Audio Codec interface to their default 
  *         state
  * @param  None.
  * @retval None.
  */
static void Codec_GPIO_DeInit(void)
{
    /* Deinitialize all the GPIOs used by the driver (EXCEPT the I2C IOs since 
     they are used by the IOExpander as well) */
    gpio_init(CODEC_I2S_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CODEC_I2S_WS_PIN | CODEC_I2S_SCK_PIN | CODEC_I2S_SD_PIN);

#ifdef CODEC_MCLK_ENABLED
    /* CODEC_I2S pins deinitialization: MCK pin */
    gpio_init(CODEC_I2S_MCK_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CODEC_I2S_MCK_PIN);
#endif /* CODEC_MCLK_ENABLED */
}

/**
  * @brief  Inserts a delay time (not accurate timing).
  * @param  nCount: specifies the delay time length.
  * @retval None.
  */
static void Delay( __IO uint32_t nCount)
{
    for (; nCount != 0; nCount--);
}

#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{
    /* Block communication and all processes */
    while (1)
    {
    }
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */

/* Audio MAL Interface Control Functions */

/**
  * @brief  Initializes and prepares the Media to perform audio data transfer 
  *         from Media to the I2S peripheral.
  * @param  None.
  * @retval None.
  */
void Audio_MAL_Init(void)  
{   
    /* Enable the DMA clock */
    rcu_periph_clock_enable(AUDIO_MAL_DMA_CLOCK);

    /* Configure the DMA Stream */
    dma_channel_enable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);
    dma_deinit(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

    /* Set the parameters to be configured */
    DMA_InitStructure.periph_addr = CODEC_I2S_ADDRESS;
    DMA_InitStructure.memory_addr = (uint32_t)0;/* This field will be configured in play function */
    DMA_InitStructure.direction = DMA_MEMORY_TO_PERIPHERAL;
    DMA_InitStructure.number = (uint32_t)0xFFFE;/* This field will be configured in play function */
    DMA_InitStructure.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    DMA_InitStructure.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    DMA_InitStructure.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    DMA_InitStructure.memory_width = DMA_MEMORY_WIDTH_16BIT;

#ifdef AUDIO_MAL_MODE_NORMAL
    dma_circulation_disable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
    dma_circulation_enable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);
#else
    #error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */  

    DMA_InitStructure.priority = DMA_PRIORITY_HIGH;
    dma_init(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL, &DMA_InitStructure);

   /* Enable the I2S DMA request */
   spi_dma_enable(CODEC_I2S, SPI_DMA_TRANSMIT);
}

/**
  * @brief  Restore default state of the used Media.
  * @param  None.
  * @retval None.
  */
void Audio_MAL_DeInit(void)  
{
    /* Disable the DMA Channel before the deinit */
    dma_channel_disable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

    /* Dinitialize the DMA Channel */
    dma_deinit(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

    /* The DMA clock is not disabled, since it can be used by other streams */
}

/**
  * @brief  Starts playing audio stream from the audio Media.
  * @param  Addr: Pointer to the audio stream buffer
  * @param  Size: Number of data in the audio stream buffer
  * @retval None.
  */
void Audio_MAL_Play(uint32_t Addr, uint32_t Size)
{
    /* Enable the I2S DMA Stream*/
    dma_channel_disable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

    /* Clear the Interrupt flag */
    dma_interrupt_flag_clear(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL, DMA_INT_FLAG_FTF);

    /* Configure the buffer address and size */
    DMA_InitStructure.memory_addr = (uint32_t)Addr;
    DMA_InitStructure.number = (uint32_t)(Size*2);

    /* Configure the DMA Stream with the new parameters */
    dma_init(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL, &DMA_InitStructure);

    /* Enable the I2S DMA Stream*/
    dma_channel_enable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

    /* If the I2S peripheral is still not enabled, enable it */
    if ((SPI_I2SCTL(CODEC_I2S) & I2S_ENABLE_MASK) == 0)
    {
        i2s_enable(CODEC_I2S);
    }
}

/**
  * @brief  Pauses or Resumes the audio stream playing from the Media.
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @param  Addr: Address from/at which the audio stream should resume/pause.
  * @param  Size: Number of data to be configured for next resume.
  * @retval None.
  */
void Audio_MAL_PauseResume(uint32_t Cmd, uint32_t Addr, uint32_t Size)
{
    /* Pause the audio file playing */
    if (Cmd == AUDIO_PAUSE)
    {
        /* Stop the current DMA request by resetting the I2S cell */
        Codec_AudioInterface_DeInit();

        /* Re-configure the I2S interface for the next resume operation */
        Codec_AudioInterface_Init(I2S_AudioFreq);

        /* Disable the DMA Stream */
        dma_channel_disable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

        /* Clear the Interrupt flag */
        dma_flag_clear(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL, AUDIO_MAL_DMA_FLAG_ALL);
    }
    else /* AUDIO_RESUME */
    {
        /* Configure the buffer address and size */
        DMA_InitStructure.memory_addr = (uint32_t)Addr;
        DMA_InitStructure.number = (uint32_t)(Size*2);

        /* Configure the DMA Stream with the new parameters */
        dma_init(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL, &DMA_InitStructure);

        /* Enable the I2S DMA Stream*/
        dma_channel_enable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

        /* If the I2S peripheral is still not enabled, enable it */
        if ((SPI_I2SCTL(CODEC_I2S) & I2S_ENABLE_MASK) == 0)
        {
            i2s_enable(CODEC_I2S);
        }
    }
}

/**
  * @brief  Stops audio stream playing on the used Media.
  * @param  None.
  * @retval None.
  */
void Audio_MAL_Stop(void)
{
    /* Stop the Transfer on the I2S side: Stop and disable the DMA stream */
    dma_channel_disable(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL);

    /* Clear all the DMA flags for the next transfer */
    dma_flag_clear(AUDIO_MAL_DMA, AUDIO_MAL_DMA_CHANNEL, AUDIO_MAL_DMA_FLAG_ALL);

    /* Stop the current DMA request by resetting the I2S cell */
    Codec_AudioInterface_DeInit();

    /* Re-configure the I2S interface for the next paly operation */
    Codec_AudioInterface_Init(I2S_AudioFreq);
}
