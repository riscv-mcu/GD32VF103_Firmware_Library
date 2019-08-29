/*!
    \file    main.c
    \brief   deepsleep wakeup through exti interrupt

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
#include "systick.h"
#include "main.h"

#define BKP_DATA_REG_NUM              42

void led_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* configure systick */
    systick_config();
    
    /* led config */
    led_config();
    /* clock enable */
    rcu_periph_clock_enable(RCU_PMU);
    /* wakeup key config */
    gd_eval_key_init(KEY_A, KEY_MODE_GPIO);
    /* tamper key EXTI config */
    gd_eval_key_init(KEY_B, KEY_MODE_EXTI);
    /* press wakeup key to enter deepsleep mode and use tamper key generate a exti interrupt to wakeup mcu */
    while(1){
        if(RESET == gpio_input_bit_get(KEY_A_GPIO_PORT, KEY_A_PIN))
            pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
    }
}

/*!
    \brief      toggle the led
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_spark(void)
{
    static __IO uint32_t timingdelaylocal = 0;

    if (timingdelaylocal != 0x00){
        /* all the leds on */
        if(timingdelaylocal < 200){
            gd_eval_led_on(LED1);
            gd_eval_led_on(LED2);
            gd_eval_led_on(LED3);
            gd_eval_led_on(LED4);
        }else{
            /* all the leds off */
            gd_eval_led_off(LED1);
            gd_eval_led_off(LED2);
            gd_eval_led_off(LED3);
            gd_eval_led_off(LED4);
        }
        timingdelaylocal--;
    }else{
        timingdelaylocal = 400;
    }
}

/*!
    \brief      check if the backup data registers are clear or not
    \param[in]  none
    \param[out] none
    \retval     the number of data register
*/
uint32_t is_backup_register_clear(void)
{
    uint32_t temp = 0;

    for(temp = 0; temp < BKP_DATA_REG_NUM; temp++){
        if(temp < 10){
            /* check if the data of data register 0-9 is 0x0000 */
            if(0x0000 != BKP_DATA_GET(BKP_DATA0_9(temp))){
                return temp+1;
            }
        }else{
            /* check if the data of data register 10-41 is 0x0000 */
            if(0x0000 != BKP_DATA_GET(BKP_DATA10_41(temp))){
                return temp+1;
            }
        }
    }
    return 0;
}

/*!
    \brief      led config
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
}
