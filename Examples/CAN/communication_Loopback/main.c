/*!
    \file  main.c
    \brief communication_Loopback in normal mode
    
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
#include <stdio.h>
#include "gd32vf103v_eval.h"

/* select CAN */
#define CAN0_USED
//#define CAN1_USED

#ifdef  CAN0_USED
    #define CANX CAN0
#else 
    #define CANX CAN1
#endif
    
volatile ErrStatus test_flag;
volatile ErrStatus test_flag_interrupt;

void clic_config(void);
void led_config(void);
ErrStatus can_loopback(void);
ErrStatus can_loopback_interrupt(void);
void can_loopback_init(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL2_PRIO2);
    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_CAN1);
    
    /* configure CLIC */
    clic_config();
    
    /* configure leds */
    led_config();
    /* set all the leds off */
    gd_eval_led_off(LED1);
    gd_eval_led_off(LED2);
    gd_eval_led_off(LED3);
    gd_eval_led_off(LED4);
    /* loopback of polling */
    test_flag = can_loopback();
 
    if(SUCCESS == test_flag){
        /* loopback test is success */
        gd_eval_led_on(LED1);
        gd_eval_led_on(LED2);
    }else{
        /* loopback test is failed */
        gd_eval_led_off(LED1);
        gd_eval_led_off(LED2);
    }
    /* loopback of interrupt */
    test_flag_interrupt = can_loopback_interrupt();

    if(SUCCESS == test_flag_interrupt){
        /* interrupt loopback test is success */
        gd_eval_led_on(LED3);
        gd_eval_led_on(LED4);
    }else{
        /* interrupt loopback test is failed */
        gd_eval_led_off(LED3);
        gd_eval_led_off(LED4);
    }
    while (1);
}

/*!
    \brief      function for CAN loopback communication
    \param[in]  none
    \param[out] none
    \retval     ErrStatus
*/
ErrStatus can_loopback(void)
{
    can_trasnmit_message_struct transmit_message;
    can_receive_message_struct  receive_message;
    uint32_t timeout = 0xFFFF;
    uint8_t transmit_mailbox = 0;
    /* initialize CAN */
    can_loopback_init();

    /* initialize transmit message */
    can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
    transmit_message.tx_sfid = 0x11;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_STANDARD;
    transmit_message.tx_dlen = 2;
    transmit_message.tx_data[0] = 0xAB;
    transmit_message.tx_data[1] = 0xCD;
    
    /* initialize receive message */
    can_struct_para_init(CAN_RX_MESSAGE_STRUCT, &receive_message);
    
    /* transmit message */
    transmit_mailbox = can_message_transmit(CANX, &transmit_message);
    /* waiting for transmit completed */
    while((CAN_TRANSMIT_OK != can_transmit_states(CANX, transmit_mailbox)) && (0 != timeout)){
        timeout--;
    }
    timeout = 0xFFFF;
    /* waiting for receive completed */
    while((can_receive_message_length_get(CANX, CAN_FIFO1) < 1) && (0 != timeout)){
        timeout--; 
    }

    /* initialize receive message*/
    receive_message.rx_sfid = 0x00;
    receive_message.rx_ff = 0;
    receive_message.rx_dlen = 0;
    receive_message.rx_data[0] = 0x00;
    receive_message.rx_data[1] = 0x00;
    can_message_receive(CANX, CAN_FIFO1, &receive_message);
    
    /* check the receive message */
    if((0x11 == receive_message.rx_sfid) && (CAN_FF_STANDARD == receive_message.rx_ff)
       && (2 == receive_message.rx_dlen) && (0xCDAB == (receive_message.rx_data[1]<<8|receive_message.rx_data[0]))){
        return SUCCESS;
    }else{
        return ERROR;
    }
}

/*!
    \brief      function for CAN loopback interrupt communication
    \param[in]  none
    \param[out] none
    \retval     ErrStatus
*/
ErrStatus can_loopback_interrupt(void)
{
    can_trasnmit_message_struct transmit_message;
    uint32_t timeout = 0x0000FFFF;
    
    /* initialize CAN and filter */
    can_loopback_init();

    /* enable CAN receive FIFO1 not empty interrupt  */ 
    can_interrupt_enable(CANX, CAN_INT_RFNE1);

    /* initialize transmit message */
    transmit_message.tx_sfid = 0;
    transmit_message.tx_efid = 0x1234;
    transmit_message.tx_ff = CAN_FF_EXTENDED;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_dlen = 2;
    transmit_message.tx_data[0] = 0xDE;
    transmit_message.tx_data[1] = 0xCA;
    /* transmit a message */
    can_message_transmit(CANX, &transmit_message);
    
    /* waiting for receive completed */
    while((SUCCESS != test_flag_interrupt) && (0 != timeout)){
        timeout--;
    }
    if(0 == timeout){
        test_flag_interrupt = ERROR;
    }

    /* disable CAN receive FIFO1 not empty interrupt  */ 
    can_interrupt_disable(CANX, CAN_INTEN_RFNEIE1);
    
    return test_flag_interrupt;
}

/*!
    \brief      initialize CAN and filter
    \param[in]  can_parameter
      \arg        can_parameter_struct
    \param[in]  can_filter
      \arg        can_filter_parameter_struct
    \param[out] none
    \retval     none
*/
void can_loopback_init(void)
{
    can_parameter_struct        can_parameter;
    can_filter_parameter_struct can_filter;
    
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
    
    /* initialize CAN register */
    can_deinit(CANX);
    
    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_LOOPBACK_MODE;
    /* configure baudrate to 125kbps */
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_3TQ;
    can_parameter.prescaler = 48;
    can_init(CANX, &can_parameter);

    /* initialize filter */
#ifdef  CAN0_USED
    /* CAN0 filter number */
    can_filter.filter_number = 0;
#else
    /* CAN1 filter number */
    can_filter.filter_number = 15;
#endif
    /* initialize filter */    
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;  
    can_filter.filter_fifo_number = CAN_FIFO1;
    can_filter.filter_enable=ENABLE;
    can_filter_init(&can_filter);
}

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void clic_config(void)
{
    eclic_global_interrupt_enable();
    /* configure CAN0 CLIC */
    eclic_irq_enable(CAN0_RX1_IRQn,1,0);
    /* configure CAN1 CLIC */
    eclic_irq_enable(CAN1_RX1_IRQn,1,0);
}

/*!
    \brief      configure the leds
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
