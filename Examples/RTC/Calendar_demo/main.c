/*!
    \file  main.c
    \brief RTC calendar 

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
#include "rtc.h"
#include <stdio.h>

#define BKP_DATA_REG_NUM              42

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* COM0 configure */
    gd_eval_com_init(EVAL_COM0);

    /* ECLIC configure */
    eclic_global_interrupt_enable();
    eclic_set_nlbits(ECLIC_GROUP_LEVEL3_PRIO1);
    eclic_irq_enable(RTC_IRQn, 1, 0);

    printf( "\r\n This is a RTC demo...... \r\n" );

    if (bkp_data_read(BKP_DATA_0) != 0xA5A5){
        /* backup data register value is not correct or not yet programmed
        (when the first time the program is executed) */
        printf("\r\nThis is a RTC demo!\r\n");
        printf("\r\n\n RTC not yet configured....");

        /* RTC configuration */
        rtc_configuration();

        printf("\r\n RTC configured....");

        /* adjust time by values entred by the user on the hyperterminal */
        time_adjust();

        bkp_data_write(BKP_DATA_0, 0xA5A5);
    }else{
        /* check if the power on reset flag is set */
        if (rcu_flag_get(RCU_FLAG_PORRST) != RESET){
            printf("\r\n\n Power On Reset occurred....");
        }else if (rcu_flag_get(RCU_FLAG_SWRST) != RESET){
            /* check if the pin reset flag is set */
            printf("\r\n\n External Reset occurred....");
        }
        
        /* allow access to BKP domain */
        rcu_periph_clock_enable(RCU_PMU);
        pmu_backup_write_enable();
        
        printf("\r\n No need to configure RTC....");
        /* wait for RTC registers synchronization */
        rtc_register_sync_wait();

        /* enable the RTC second */
        rtc_interrupt_enable(RTC_INT_SECOND);

        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
    }

#ifdef RTCCLOCKOUTPUT_ENABLE
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* disable the tamper pin */
    bkp_tamper_detection_disable();

    /* enable RTC clock output on tamper Pin */
    bkp_rtc_calibration_output_enable();
#endif

    /* clear reset flags */
    rcu_all_reset_flag_clear();

    /* display time in infinite loop */
    time_show();

    while (1){
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

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM0, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));

    return ch;
}
