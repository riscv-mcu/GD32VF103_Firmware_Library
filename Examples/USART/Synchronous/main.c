/*!
    \file  main.c
    \brief USART synchronous

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

#include "gd32vf103.h"
#include "gd32vf103v_eval.h"
#include <stdio.h>

#define txbuffer_size1   (countof(txbuffer1) - 1)
#define txbuffer_size2   (countof(txbuffer2) - 1)
#define DYMMY_BYTE       0x00000000
#define countof(a)       (sizeof(a) / sizeof(*(a)))

uint8_t txbuffer1[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
                       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                       0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
                       0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
                       0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
                       0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
                       0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
                       0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F};
uint8_t txbuffer2[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
                       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                       0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
                       0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
                       0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
                       0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
                       0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
                       0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F};
uint8_t rxbuffer1[txbuffer_size2];
uint8_t rxbuffer2[txbuffer_size1];
__IO uint8_t data_read1 = txbuffer_size2;
__IO uint8_t data_read2 = txbuffer_size1;
__IO uint8_t tx_counter1 = 0, rx_counter1 = 0;
__IO uint8_t tx_counter2 = 0, rx_counter2 = 0;
__IO ErrStatus state1 = ERROR;
__IO ErrStatus state2 = ERROR;

void usart_config(void);
void spi_config(void);
void led_init(void);
ErrStatus memory_compare(uint8_t* src, uint8_t* dst, uint16_t length);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* initialize leds */
    led_init();
    /* turn off LED1~4 */
    gd_eval_led_off(LED1);
    gd_eval_led_off(LED2);
    gd_eval_led_off(LED3);
    gd_eval_led_off(LED4);

    /* configure USART */
    usart_config();

    /* configure SPI */
    spi_config();
    
    while(data_read2--){
        while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)){
        }
        /* write one byte in the USART0 data register */
        usart_data_transmit(USART0, txbuffer1[tx_counter1++]);
        /* wait until end of transmit */
        while(RESET == usart_flag_get(USART0, USART_FLAG_TC)){
        }
        /* wait the byte is entirely received by SPI0 */  
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE)){
        }
        /* store the received byte in the rxbuffer2 */
        rxbuffer2[rx_counter2++] = spi_i2s_data_receive(SPI0);
    }

    /* clear the USART0 data register */
    usart_data_receive(USART0);

    while(data_read1--){
        /* wait until end of transmit */
        while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE)){
        }
        /* write one byte in the SPI0 transmit data register */
        spi_i2s_data_transmit(SPI0, txbuffer2[tx_counter2++]);

        /* send a dummy byte to generate clock to slave */ 
        usart_data_transmit(USART0, DYMMY_BYTE);
        /* wait until end of transmit */
        while(RESET == usart_flag_get(USART0, USART_FLAG_TC)){
        }
        /* wait the byte is entirely received by USART0 */
        while(RESET == usart_flag_get(USART0, USART_FLAG_RBNE)){
        }
        /* store the received byte in the rxbuffer1 */
        rxbuffer1[rx_counter1++] = usart_data_receive(USART0);
    }
  
    /* check the received data with the send ones */
    state1 = memory_compare(txbuffer1, rxbuffer2, txbuffer_size1);
    state2 = memory_compare(txbuffer2, rxbuffer1, txbuffer_size2);
    
    if(SUCCESS == state1){
        /* if the data transmitted from USART0 and received by SPI0 are the same */
        gd_eval_led_on(LED1);
        gd_eval_led_on(LED3);
    }else{
        /* if the data transmitted from USART0 and received by SPI0 are not the same */
        gd_eval_led_off(LED1);
        gd_eval_led_off(LED3);
    }
    if(SUCCESS == state2){
        /* if the data transmitted from SPI0 and received by USART0 are the same */
        gd_eval_led_on(LED2);
        gd_eval_led_on(LED4);
    }else{
        /* if the data transmitted from SPI0 and received by USART0 are not the same */
        gd_eval_led_off(LED2);
        gd_eval_led_off(LED4);
    }
    while(1){
    }
}
/*!
    \brief      configure USART
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usart_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_AF);
    
    /* configure USART Tx as alternate function push-pull */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9|GPIO_PIN_8);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    
    /* configure USART synchronous mode */
    usart_synchronous_clock_enable(USART0);
    usart_synchronous_clock_config(USART0, USART_CLEN_EN, USART_CPH_2CK, USART_CPL_HIGH);
    
    usart_baudrate_set(USART0, 115200);
    /* configure USART transmitter */
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    /* configure USART receiver */
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    /* enable USART */
    usart_enable(USART0);
}

/*!
    \brief      configure SPI
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_config(void)
{
    spi_parameter_struct spi_init_parameter;
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);
    rcu_periph_clock_enable(RCU_AF);
    
    spi_i2s_deinit(SPI0);
    
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_5 | GPIO_PIN_6);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_7);

    /* configure SPI0 */
    spi_init_parameter.device_mode = SPI_SLAVE;
    spi_init_parameter.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_parameter.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_parameter.nss = SPI_NSS_SOFT;
    spi_init_parameter.endian = SPI_ENDIAN_LSB;
    spi_init_parameter.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_parameter.prescale = SPI_PSC_32;
    spi_init(SPI0, &spi_init_parameter);

    /* SPI0 enable */
    spi_enable(SPI0);
}

/*!
    \brief      initialize leds
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_init(void)
{
    gd_eval_led_init(LED1);
    gd_eval_led_init(LED2);
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED4);

}

/*!
    \brief      memory compare function
    \param[in]  src: source data
    \param[in]  dst: destination data
    \param[in]  length: the compare data length
    \param[out] none
    \retval     ErrStatus: ERROR or SUCCESS
*/
ErrStatus memory_compare(uint8_t* src, uint8_t* dst, uint16_t length) 
{
    while(length--){
        if(*src++ != *dst++){
            return ERROR;
        }
    }
    return SUCCESS;
}
