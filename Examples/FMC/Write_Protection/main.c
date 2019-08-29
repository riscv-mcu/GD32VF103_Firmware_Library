/*!
    \file  main.c
    \brief main flash program, write_protection

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

#define FLASH_PAGE_PROGRAM
//#define WRITE_PROTECTION_ENABLE
#define WRITE_PROTECTION_DISABLE

typedef enum {FAILED = 0, PASSED = !FAILED} test_state;
#define FLASH_PAGE_SIZE ((uint16_t)0x400)
#define FMC_PAGES_PROTECTED (OB_WP_4 | OB_WP_5)

#define WRITE_START_ADDR  ((uint32_t)0x08004000)
#define WRITE_END_ADDR    ((uint32_t)0x08010000)

uint32_t erase_counter = 0x0, address = 0x0;
uint32_t data = 0x5a5a5a5a;
uint32_t wp_value = 0xFFFFFFFF, protected_pages = 0x0;
uint32_t page_number;
__IO fmc_state_enum fmc_state = FMC_READY;
__IO test_state program_state = PASSED;

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* initialize led on the board */
    gd_eval_led_init(LED2);
    gd_eval_led_init(LED3);

    /* unlock the flash program/erase controller */
    fmc_unlock();
    ob_unlock();

    fmc_flag_clear(FMC_FLAG_END);
    fmc_flag_clear(FMC_FLAG_WPERR);
    fmc_flag_clear(FMC_FLAG_PGERR);

    /* get pages write protection status */
    wp_value = ob_write_protection_get();

#ifdef WRITE_PROTECTION_DISABLE
    /* get pages already write protected */
    protected_pages = ~(wp_value | FMC_PAGES_PROTECTED);
    /* check if desired pages are already write protected */
    if((wp_value | (~FMC_PAGES_PROTECTED)) != 0xFFFFFFFF ){
        /* erase all the option Bytes */
        fmc_state = ob_erase();

        /* check if there is write protected pages */
        if(protected_pages != 0x0){
            /* Restore write protected pages */
            fmc_state = ob_write_protection_enable(protected_pages);
        }
        /* generate system reset to load the new option byte values */
         eclic_system_reset();
    }
#elif defined WRITE_PROTECTION_ENABLE
    /* get current write protected pages and the new pages to be protected */
    protected_pages = (~wp_value) | FMC_PAGES_PROTECTED;

    /* check if desired pages are not yet write protected */
    if(((~wp_value) & FMC_PAGES_PROTECTED )!= FMC_PAGES_PROTECTED){

        /* erase all the option bytes because if a program operation is
        performed on a protected page, the flash memory returns a
        protection error */
        fmc_state = ob_erase();

        /* enable the pages write protection */
        fmc_state = ob_write_protection_enable(protected_pages);

        /* generate system reset to load the new option byte values */
         eclic_system_reset();
    }
#endif /* WRITE_PROTECTION_DISABLE */

#ifdef FLASH_PAGE_PROGRAM
    /* get the number of pages to be erased */
    page_number = (WRITE_END_ADDR - WRITE_START_ADDR) / FLASH_PAGE_SIZE;

    /* the selected pages are not write protected */
    if(0x00 != (wp_value & FMC_PAGES_PROTECTED)){
        /* clear all pending flags */
        fmc_flag_clear(FMC_FLAG_END);
        fmc_flag_clear(FMC_FLAG_WPERR);
        fmc_flag_clear(FMC_FLAG_PGERR);

        /* erase the flash pages */
        for(erase_counter = 0; (erase_counter < page_number) && (FMC_READY == fmc_state); erase_counter++){
            fmc_state = fmc_page_erase(WRITE_START_ADDR + (FLASH_PAGE_SIZE * erase_counter));
        }

        /* flash word program of data 0x5a5a5a5a at addresses defined by  WRITE_START_ADDR and WRITE_END_ADDR */
        address = WRITE_START_ADDR;

        while((address < WRITE_END_ADDR) && (FMC_READY == fmc_state)){
            fmc_state = fmc_word_program(address, data);
            address = address + 4;
        }
        /* check the correctness of written data */
        address = WRITE_START_ADDR;

        while((address < WRITE_END_ADDR) && (FAILED != program_state)){
            if(REG32(address) != data){
                program_state = FAILED;
            }
            address += 4;
        }
        gd_eval_led_on(LED2);
    }
    else{
        /* error to program the flash: the desired pages are write protected */
        program_state = FAILED;
        gd_eval_led_on(LED3);
    }
#endif /* FLASH_PAGE_PROGRAM */
    while(1){
    }
}
