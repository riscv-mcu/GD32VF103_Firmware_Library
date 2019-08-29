/*!
    \file    readme.txt
    \brief   description of the DMA flash to ram example

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

  This example is based on the GD32VF103V-EVAL-V1.0 board, it provides a description of how to 
use DMA0 channel0 to transfer data buffer from FLASH memory to embedded SRAM memory.
    
  Before programming the flash addresses, an erase operation is performed firstly.
After the erase operation, a comparison between FLASH memory and 0xFFFFFFFF(Reset value) 
is done to check that the FLASH memory has been correctly erased.

  Once the erase operation is finished correctly, the programming operation will be
performed by using the fmc_word_program function. The written data is transfered to 
embedded SRAM memory by DMA0 Channel0. The transfer is started by enabling the DMA0 Channel0.
At the end of the transfer, a Transfer Complete interrupt is generated since it
is enabled. A comparison between the FLASH memory and embedded SRAM memory is done to
check that all data have been correctly transferred.If the result of comparison is passed,
LED1 to LED4 light up. Otherwise LED1 and LED3 light up.
