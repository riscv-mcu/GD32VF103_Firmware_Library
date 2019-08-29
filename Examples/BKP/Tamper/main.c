/*!
    \file    main.c
    \brief   tamper demo

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

#define BKP_DATA_REG_NUM              42

void led_config(void);
void write_backup_register(uint16_t data);
uint32_t check_backup_register(uint16_t data);
    
/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* led configuration and turn on all led */
    led_config();
    /* ECLIC configuration */
    eclic_global_interrupt_enable();
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);
    eclic_irq_enable(TAMPER_IRQn, 1, 0);
    /* PMU lock enable */
    rcu_periph_clock_enable(RCU_PMU);
    /* BKP clock enable */
    rcu_periph_clock_enable(RCU_BKPI);
    
    /* enable write access to the registers in backup domain */
    pmu_backup_write_enable();
    /* tamper pin active level set */
    bkp_tamper_active_level_set(TAMPER_PIN_ACTIVE_LOW);
    /* tamper detection disable  */
    bkp_tamper_detection_disable();
    /* disable the tamper interrupt */
    bkp_interrupt_disable();
    /* clear the bit flag of tamper event */
    bkp_flag_clear();
    /* configure the tamper pin active on low level, and enable the tamper pin */
    bkp_interrupt_enable();
    /* tamper detection enable  */
    bkp_tamper_detection_enable();
    /* write data to backup DATAx registers */
    write_backup_register(0x1226);
  
    /* check if the written data are correct */
    if(0x00 == check_backup_register(0x1226)){
        /* turn on LED1 */
        gd_eval_led_on(LED1);
    }else{
        /* turn on LED2 */
        gd_eval_led_on(LED2);
    }
    while(1){
    }
}

/*!
    \brief      write data to backup DATAx registers
    \param[in]  data: the data to be written to backup data registers
      \arg        0x0000-0xFFFF
    \param[out] none
    \retval     none
*/
void write_backup_register(uint16_t data)
{
    uint32_t temp = 0;
    /* write data to backup data registers */
    for (temp = 0; temp < BKP_DATA_REG_NUM; temp++){
        if(temp < 10){
            BKP_DATA0_9(temp) = data + (temp * 0x50);
        }else{
            BKP_DATA10_41(temp) = data + (temp * 0x50);
        }
    }
}

/*!
    \brief      check if the backup DATAx registers values are correct or not
    \param[in]  data: the data to be written to backup data registers
      \arg        0x0000-0xFFFF
    \param[out] none
    \retval     the number of data register
*/
uint32_t check_backup_register(uint16_t data)
{
    uint32_t temp = 0;
    /* check the data of backup data registers */
    for(temp = 0; temp < BKP_DATA_REG_NUM; temp++){
        if(temp < 10){
            /* get data from data register 0-9 */
            if(data + (temp * 0x50) != BKP_DATA_GET(BKP_DATA0_9(temp))){
                return temp+1;
            }
        }else{
            /* get data from data register 10-41 */
            if(data + (temp * 0x50) != BKP_DATA_GET(BKP_DATA10_41(temp))){
                return temp+1;
            }
        }
    }
    return 0;
}

/*!
    \brief      configure led 
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
