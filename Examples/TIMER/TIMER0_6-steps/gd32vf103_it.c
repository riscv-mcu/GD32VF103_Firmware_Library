/*!
    \file  gd32vf103_it.c
    \brief interrupt service routines

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
#include "systick.h"

__IO uint32_t step = 1;


/*!
    \brief      this function handles TIMER0 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void TIMER0_TRG_CMT_IRQHandler(void)
{
    /* clear TIMER interrupt flag */
    timer_interrupt_flag_clear(TIMER0,TIMER_INT_CMT);

    switch(step){
    /* next step: step 2 configuration .A-C` breakover---------------------------- */
    case 1: 
        /*  channel0 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_ENABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCXN_DISABLE);

        /*  channel1 configuration */
        timer_channel_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCXN_DISABLE);

        /*  channel2 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCXN_ENABLE);

        step++;
        break; 

    /* next step: step 3 configuration .B-C` breakover---------------------------- */
    case 2:
        /*  channel0 configuration */
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCXN_DISABLE);

        /*  channel1 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCX_ENABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCXN_DISABLE);

        /*  channel2 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCXN_ENABLE);

        step++;
        break;

    /* next step: step 4 configuration .B-A` breakover---------------------------- */
    case 3:
        /*  channel0 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCXN_ENABLE);

        /*  channel1 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCX_ENABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCXN_DISABLE);

        /*  channel2 configuration */
        timer_channel_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCXN_DISABLE);

        step++;
        break;

    /* next step: step 5 configuration .C-A` breakover---------------------------- */
    case 4:
        /*  channel0 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCXN_ENABLE);

        /*  channel1 configuration */       
        timer_channel_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCXN_DISABLE);

        /*  channel2 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCX_ENABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCXN_DISABLE);

        step++;
        break;

    /* next step: step 6 configuration .C-B` breakover---------------------------- */
    case 5:
        /*  channel0 configuration */
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCXN_DISABLE);

        /*  channel1 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCXN_ENABLE);

        /*  channel2 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCX_ENABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCXN_DISABLE);

        step++;
        break;
  
    /* next step: step 1 configuration .A-B` breakover---------------------------- */        
    case 6:
        /*  channel0 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCX_ENABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_0, TIMER_CCXN_DISABLE);

        /*  channel1 configuration */
        timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);
        timer_channel_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_1, TIMER_CCXN_ENABLE);

        /*  channel2 configuration */
        timer_channel_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCX_DISABLE);
        timer_channel_complementary_output_state_config(TIMER0, TIMER_CH_2, TIMER_CCXN_DISABLE);

        step = 1;
        break;
    }
}
