/*!
    \file    gd32vf103_it.c
    \brief   interrupt service routines

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

#include "gd32vf103_it.h"
#include "gd32vf103v_eval.h"

extern uint32_t is_backup_register_clear(void);

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void TAMPER_IRQHandler(void)
{
    if(RESET != bkp_interrupt_flag_get()){
        /* a tamper detection event occurred */
        /* check if backup data registers are cleared */
        if(0 == is_backup_register_clear()){
            /* backup data registers are cleared */
            /* turn on LED3 */
            gd_eval_led_on(LED3);
        }else{
            /* backup data registers are not cleared */
            /* turn on LED4 */
            gd_eval_led_on(LED4);
        }
        /* clear the interrupt bit flag of tamper interrupt */
        bkp_interrupt_flag_clear();
        /* disable the tamper pin */
        bkp_interrupt_disable();
        /* enable the tamper pin */
        bkp_interrupt_enable();
        /* tamper pin active level set */
        bkp_tamper_active_level_set(TAMPER_PIN_ACTIVE_LOW);
    }
}
