/*!
    \file  readme.txt
    \brief description of the TIMER0 DMA burst demo

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

  This demo is based on the GD32VF103V-EVAL-V1.0 board, it shows how to use DMA with 
TIMER0 update request to transfer data from memory to TIMER0 capture compare 
register 0~3.
 
  TIMER0CLK is fixed to systemcoreclock, the TIMER0 prescaler is equal to 108
so the TIMER0 counter clock used is 1MHz.

  The objective is to configure TIMER0 channel 0~3(PA8~PA11) to generate PWM signal.
capture compare register 0~3 are to be updated twice per circle. On the first update 
DMA request, data1 is transferred to CH0CV, data2 is transferred to CH1CV, data3 is 
transferred to CH2CV, data4 is transferred to CH3CV and duty cycle(10%, 20%, 30%, 40%). 
On the second update DMA request, data5 is transferred to CH0CV, data6 is transferred 
to CH1CV, data7 is transferred to CH2CV, data8 is transferred to CH3CV and duty cycle
(50%, 60%, 70%, 80%).
