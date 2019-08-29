/*!
    \file    main.c
    \brief   flash to ram demo

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
#include <string.h>
#include "gd32vf103v_eval.h"
#include <stdio.h>
#include "riscv_encoding.h"

#define TRANSFER_NUM                     0x400                     /* configuration value in bytes */
#define FMC_PAGE_SIZE                    ((uint16_t)0x800)
#define BANK0_WRITE_START_ADDR           ((uint32_t)0x08004000)

void rcu_config(void);
void config_clic_irqs (void);
void led_config(void);

__IO uint32_t g_dmacomplete_flag = 0;
uint8_t g_destbuf[TRANSFER_NUM];
const uint32_t transdata = 0x3210ABCD;
fmc_state_enum fmcstatus = FMC_READY;

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    uint32_t i, count;
    uint32_t *ptrd;
    uint32_t address = 0x00;
    ErrStatus access_flag = SUCCESS;
    dma_parameter_struct dma_init_struct;
    uint32_t wperror = 0;

    /* system clocks configuration */
    rcu_config();
    /* CLIC configuration */
    config_clic_irqs();
    /* LED configuration */
    led_config() ;
    /* unlock the flash bank1 program erase controller */
    fmc_unlock();

    /* define the number of page to be erased */
    count = TRANSFER_NUM / FMC_PAGE_SIZE;
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_PGERR | FMC_FLAG_WPERR | FMC_FLAG_END);

    /* erase the flash pages */
    for(i = 0; i <= count; i++){
        fmcstatus = fmc_page_erase(BANK0_WRITE_START_ADDR + (FMC_PAGE_SIZE * i));
        wperror += (fmcstatus == FMC_WPERR);
        fmc_flag_clear(FMC_FLAG_PGERR | FMC_FLAG_WPERR | FMC_FLAG_END);
    }

    if(wperror != 0){
        while(1);
    }

    /* unlock the flash bank1 program erase controller */
    fmc_lock();

    ptrd = (uint32_t*)BANK0_WRITE_START_ADDR;
    count = TRANSFER_NUM / sizeof(*ptrd);

    for(i = 0; i < count; i++){
        if(0xFFFFFFFF != *ptrd){
            access_flag = ERROR;
            break;
        }
        ptrd++;
    }

    if(ERROR == access_flag){
        while(1);
    }

    /* unlock the flash bank1 program erase controller */
    fmc_unlock();
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_PGERR | FMC_FLAG_WPERR | FMC_FLAG_END);

    /* program flash bank1 */
    address = BANK0_WRITE_START_ADDR;
    wperror = 0;
    count = BANK0_WRITE_START_ADDR + TRANSFER_NUM;

    while(address < count){
        fmcstatus = fmc_word_program(address, transdata);
        address = address + 4;
        wperror += (FMC_WPERR == fmcstatus);
        fmc_flag_clear(FMC_FLAG_PGERR | FMC_FLAG_WPERR | FMC_FLAG_END);
    }

    if(wperror != 0){
        while(1);
    }

    fmc_lock();

    memset(g_destbuf ,0 ,TRANSFER_NUM);

    /* DMA0 channel0 initialize */
    dma_deinit(DMA0, DMA_CH0);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)g_destbuf;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = TRANSFER_NUM;
    dma_init_struct.periph_addr = (uint32_t)BANK0_WRITE_START_ADDR;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_ENABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH0, &dma_init_struct);
    /* DMA0 channel0 mode configuration */
    dma_circulation_disable(DMA0, DMA_CH0);
    dma_memory_to_memory_enable(DMA0, DMA_CH0);
    /* DMA0 channel0 interrupt configuration */
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);
    /* enable DMA0 transfer */
    dma_channel_enable(DMA0, DMA_CH0);

    /* wait DMA interrupt */
    while(0 == g_dmacomplete_flag);

    /* compare destdata with transdata */
    ptrd = (uint32_t *)g_destbuf;
    count = TRANSFER_NUM / sizeof(*ptrd);

    for(i = 0; i < count; i++){
        if(transdata != *ptrd){
            access_flag = ERROR;
            break;
        }
        ptrd++;
    }

    /* transfer success */
    if(access_flag != ERROR){
        gd_eval_led_on(LED1);
        gd_eval_led_on(LED2);
        gd_eval_led_on(LED3);
        gd_eval_led_on(LED4);
     }else{
        gd_eval_led_on(LED1);
        gd_eval_led_on(LED3);
    }

    while(1){
    }
}

/*!
    \brief      configure LED
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_config(void)
{
    gd_eval_led_init(LED1);
    gd_eval_led_init(LED2);
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED4);

    /* LED off */
    gd_eval_led_off(LED1);
    gd_eval_led_off(LED2);
    gd_eval_led_off(LED3);
    gd_eval_led_off(LED4);
}

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA0);
}

/*!
    \brief      configure the CLIC
    \param[in]  none
    \param[out] none
    \retval     none
*/
void config_clic_irqs (void)
{
    eclic_global_interrupt_enable();
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);
    eclic_irq_enable(DMA0_Channel0_IRQn, 1, 1);

}
