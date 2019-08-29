/*!
    \file  exmc_norflash.c
    \brief EXMC NOR Flash(M29W128FH) driver

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
#include "exmc_norflash.h"

#define BANK0_NOR1_ADDR                 ((uint32_t)0x60000000)
#define BANK_NORFLASH_ADDR              BANK0_NOR1_ADDR

#define BLOCKERASE_TIMEOUT              ((uint32_t)0x00A00000)
#define CHIPERASE_TIMEOUT               ((uint32_t)0x30000000)
#define PROGRAM_TIMEOUT                 ((uint32_t)0x00001400)

#define ADDR_SHIFT(A)                   (BANK_NORFLASH_ADDR + (2 * (A)))
#define NOR_WRITE(Address, Data)        (*(__IO uint16_t *)(Address) = (Data))

/*!
    \brief      nor flash peripheral initialize
    \param[in]  none 
    \param[out] none
    \retval     none
*/
void exmc_norflash_init(void)
{
    exmc_norsram_parameter_struct nor_init_struct;
    exmc_norsram_timing_parameter_struct nor_timing_init_struct;

    /* EXMC clock enable */
    rcu_periph_clock_enable(RCU_EXMC);

    /* EXMC enable */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);

    /* configure EXMC_D[0~15]*/
    /* PD14(EXMC_D0), PD15(EXMC_D1),PD0(EXMC_D2), PD1(EXMC_D3), PD8(EXMC_D13), PD9(EXMC_D14), PD10(EXMC_D15) */
    gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1| GPIO_PIN_8 | GPIO_PIN_9 |
                                                         GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15);

    /* PE7(EXMC_D4), PE8(EXMC_D5), PE9(EXMC_D6), PE10(EXMC_D7), PE11(EXMC_D8), PE12(EXMC_D9), 
       PE13(EXMC_D10), PE14(EXMC_D11), PE15(EXMC_D12) */
    gpio_init(GPIOE, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | 
                                                         GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | 
                                                         GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    /* PD11(EXMC_A16), PD12(EXMC_A17), PD13(EXMC_A18) */
    gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
    
    /* PE3(EXMC_A19), PE4(EXMC_A20), PE5(EXMC_A21), PE6(EXMC_A22) */
    gpio_init(GPIOE, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);
    
    /* configure PD4(NOE), PD5(NWE), PD7(NE0) */
    gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7);

    /* configure NADV */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);

    /* configure NWAIT */
    gpio_init(GPIOD, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_6);

    nor_timing_init_struct.bus_latency = 1;
    nor_timing_init_struct.asyn_data_setuptime = 7;
    nor_timing_init_struct.asyn_address_holdtime = 2;
    nor_timing_init_struct.asyn_address_setuptime = 6;

    nor_init_struct.norsram_region = EXMC_BANK0_NORSRAM_REGION0;
    nor_init_struct.asyn_wait = DISABLE;
    nor_init_struct.nwait_signal = DISABLE;
    nor_init_struct.memory_write = ENABLE;
    nor_init_struct.nwait_polarity = EXMC_NWAIT_POLARITY_LOW;
    nor_init_struct.databus_width = EXMC_NOR_DATABUS_WIDTH_16B;
    nor_init_struct.memory_type = EXMC_MEMORY_TYPE_NOR;
    nor_init_struct.address_data_mux = ENABLE;
    nor_init_struct.read_write_timing = &nor_timing_init_struct;

    exmc_norsram_init(&nor_init_struct);

    exmc_norsram_enable(EXMC_BANK0_NORSRAM_REGION0);
}


/*!
    \brief      read NOR memory's manufacturer, device code, block_protection_indicator, block_protection_status
    \param[in]  nor_id: pointer to a nor_idtypedef structure
    \param[out] none
    \retval     none
*/
void nor_read_id(nor_id_struct* nor_id)
{
    NOR_WRITE(ADDR_SHIFT(CMD_READID_ADD_1ST), CMD_READID_DATA_1ST);
    NOR_WRITE(ADDR_SHIFT(CMD_READID_ADD_2ND), CMD_READID_DATA_2ND);
    NOR_WRITE(ADDR_SHIFT(CMD_READID_ADD_3RD), CMD_READID_DATA_3RD);

    /* read NOR Flash ID */
    nor_id->manufacturer_code = *(__IO uint16_t *) ADDR_SHIFT(0x0000);
    nor_id->device_code1 = *(__IO uint16_t *) ADDR_SHIFT(0x0001);
    nor_id->device_code2 = *(__IO uint16_t *) ADDR_SHIFT(0x000E);
    nor_id->device_code3 = *(__IO uint16_t *) ADDR_SHIFT(0x000F);
}

/*!
    \brief      erase the specified nor flash block
    \param[in]  blockaddr: address of the block to be erased
    \param[out] none
    \retval     NOR_SUCCESS,NOR_ERROR,NOR_TIMEOUT
*/
nor_status_struct nor_eraseblock(uint32_t blockaddr)
{
    NOR_WRITE(ADDR_SHIFT(CMD_BLOCKERASE_ADD_1ST), CMD_BLOCKERASE_DATA_1ST);
    NOR_WRITE(ADDR_SHIFT(CMD_BLOCKERASE_ADD_2ND), CMD_BLOCKERASE_DATA_2ND);
    NOR_WRITE(ADDR_SHIFT(CMD_BLOCKERASE_ADD_3RD), CMD_BLOCKERASE_DATA_3RD);
    NOR_WRITE(ADDR_SHIFT(CMD_BLOCKERASE_ADD_4TH), CMD_BLOCKERASE_DATA_4TH);
    NOR_WRITE(ADDR_SHIFT(CMD_BLOCKERASE_ADD_5TH), CMD_BLOCKERASE_DATA_5TH);
    NOR_WRITE((BANK_NORFLASH_ADDR  + blockaddr), CMD_BLOCKERASE_DATA_6TH);

    return (nor_get_status(BLOCKERASE_TIMEOUT));
}

/*!
    \brief      erase the entire chip
    \param[in]  none
    \param[out] none
    \retval     NOR_SUCCESS,NOR_ERROR,NOR_TIMEOUT
*/
nor_status_struct nor_erasechip(void)
{
    NOR_WRITE(ADDR_SHIFT(CMD_CHIPERASE_ADD_1ST), CMD_CHIPERASE_DATA_1ST);
    NOR_WRITE(ADDR_SHIFT(CMD_CHIPERASE_ADD_2ND), CMD_CHIPERASE_DATA_2ND);
    NOR_WRITE(ADDR_SHIFT(CMD_CHIPERASE_ADD_3RD), CMD_CHIPERASE_DATA_3RD);
    NOR_WRITE(ADDR_SHIFT(CMD_CHIPERASE_ADD_4TH), CMD_CHIPERASE_DATA_4TH);
    NOR_WRITE(ADDR_SHIFT(CMD_CHIPERASE_ADD_5TH), CMD_CHIPERASE_DATA_5TH);
    NOR_WRITE(ADDR_SHIFT(CMD_CHIPERASE_ADD_6TH), CMD_CHIPERASE_DATA_6TH);

    return (nor_get_status(CHIPERASE_TIMEOUT));
}

/*!
    \brief      write a half-word to the specified address of nor flash
    \param[in]  writeaddr: NOR flash internal address to write to
    \param[in]  data: data to be written
    \param[out] none
    \retval     NOR_SUCCESS,NOR_ERROR,NOR_TIMEOUT
*/
nor_status_struct nor_write_halfword(uint32_t writeaddr, uint16_t data)
{
    NOR_WRITE(ADDR_SHIFT(CMD_WRITE_ADD_1ST), CMD_WRITE_DATA_1ST);
    NOR_WRITE(ADDR_SHIFT(CMD_WRITE_ADD_2ND), CMD_WRITE_DATA_2ND);
    NOR_WRITE(ADDR_SHIFT(CMD_WRITE_ADD_3RD), CMD_WRITE_DATA_3RD);
    NOR_WRITE((BANK_NORFLASH_ADDR  + writeaddr), data);

    return (nor_get_status(PROGRAM_TIMEOUT));
}

/*!
    \brief      write a half-word buffer to the specified address of nor flash
    \param[in]  pbuffer: pointer to a half-word buffer
    \param[in]  writeaddr: NOR flash internal address from which the data will be written
    \param[in]  halfword_count: count of half words to write
    \param[out] none
    \retval     NOR_SUCCESS,NOR_ERROR,NOR_TIMEOUT
*/
nor_status_struct nor_write_buffer(uint16_t* pbuffer, uint32_t writeaddr, uint32_t halfword_count)
{
    nor_status_struct status = NOR_ONGOING; 
    
    do{
        /* write data to nor flash */
        status = nor_write_halfword(writeaddr, *pbuffer++);
        writeaddr = writeaddr + 2;
        halfword_count--;
    }while((NOR_SUCCESS == status) && (halfword_count != 0));

    return (status);
}

/*!
    \brief      read a half-word from the specified address of nor flash
    \param[in]  readaddr: NOR flash internal address to read from
    \param[out] none
    \retval     A half-word read from the nor flash
*/
uint16_t nor_read_halfword(uint32_t readaddr)
{
    NOR_WRITE(ADDR_SHIFT(CMD_READ_ADD_1ST), CMD_READ_DATA_1ST);
    NOR_WRITE(ADDR_SHIFT(CMD_READ_ADD_2ND), CMD_READ_DATA_2ND);
    NOR_WRITE((BANK_NORFLASH_ADDR  + readaddr), CMD_READ_DATA_3RD);

    return (*(__IO uint16_t *)((BANK_NORFLASH_ADDR  + readaddr)));
}

/*!
    \brief      read a set of data from the specified address of nor flash
    \param[in]  pbuffer: pointer to a half-word buffer
    \param[in]  readaddr: NOR flash internal address to read from
    \param[in]  halfword_count: count of half words to write
    \param[out] none
    \retval     none
*/
void nor_readbuffer(uint16_t* pbuffer, uint32_t readaddr, uint32_t halfword_count)
{
    NOR_WRITE(ADDR_SHIFT(CMD_READ_ADD_1ST), CMD_READ_DATA_1ST); 
    NOR_WRITE(ADDR_SHIFT(CMD_READ_ADD_2ND), CMD_READ_DATA_2ND);  
    NOR_WRITE((BANK_NORFLASH_ADDR  + readaddr), CMD_READ_DATA_3RD);

    for(; halfword_count != 0x00; halfword_count--){
        /* read a halfword from the nor flash */
        *pbuffer++ = *(__IO uint16_t *)((BANK_NORFLASH_ADDR  + readaddr));
        readaddr = readaddr + 2; 
    }  
}

/*!
    \brief      return the nor flash to read mode and reset the errors in the nor flash status register
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nor_reset(void)
{
    NOR_WRITE(ADDR_SHIFT(CMD_RESET_ADD_1ST), CMD_RESET_DATA_1ST); 
    NOR_WRITE(ADDR_SHIFT(CMD_RESET_ADD_2ND), CMD_RESET_DATA_2ND); 
    NOR_WRITE(BANK_NORFLASH_ADDR , CMD_RESET_DATA_3RD); 
}

/*!
    \brief      return the nor flash to read mode
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nor_return_to_read_mode(void)
{
    NOR_WRITE(BANK_NORFLASH_ADDR , CMD_RESET_DATA_3RD); 
}

/*!
    \brief      return the nor flash operation status
    \param[in]  time_out: NOR flash programming timeout
    \param[out] none
    \retval     none
*/
nor_status_struct nor_get_status(uint32_t time_out)
{ 
    uint16_t val1 = 0x00, val2 = 0x00;
    nor_status_struct status = NOR_ONGOING;
    uint32_t timeout = time_out;

    /* poll on nor flash ready/busy signal */
    while(RESET != (gpio_input_bit_get(GPIOD, GPIO_PIN_6)) && (timeout > 0)){
        timeout--;
    }

    timeout = time_out;

    while((gpio_input_bit_get(GPIOD, GPIO_PIN_6) == RESET) && (timeout > 0)){
        timeout--;
    }

    /* get the nor flash operation status */
    while((time_out != 0x00) && (status != NOR_SUCCESS)){
        time_out--;

        /* read DQ6 and DQ5 */
        val1 = *(__IO uint16_t *)(BANK_NORFLASH_ADDR );
        val2 = *(__IO uint16_t *)(BANK_NORFLASH_ADDR );

        if((val1 & 0x0040) == (val2 & 0x0040)) {
            return NOR_SUCCESS;
        }

        if((val1 & 0x0020) != 0x0020){
            status = NOR_ONGOING;
        }

        val1 = *(__IO uint16_t *)(BANK_NORFLASH_ADDR );
        val2 = *(__IO uint16_t *)(BANK_NORFLASH_ADDR );

        if((val1 & 0x0040) == (val2 & 0x0040)) {
            return NOR_SUCCESS;
        }else if((val1 & 0x0020) == 0x0020){
            return NOR_ERROR;
        }
    }

    if(time_out == 0x00){
        status = NOR_TIMEOUT;
    }

    return (status);
}

/*!
    \brief      fill the buffer with specified value
    \param[in]  pbuffer: pointer on the buffer to fill
    \param[in]  buffer_lenght: length of the buffer to fill
    \param[in]  value: value to fill on the buffer
    \param[out] none
    \retval     none
*/
void nor_fill_buffer(uint16_t *pbuffer, uint16_t buffer_lenght, uint32_t value)
{
    uint16_t index = 0;

    /* put in global buffer same values */
    for (index = 0; index < buffer_lenght; index++ ){
        pbuffer[index] = value + index;
    }
}
