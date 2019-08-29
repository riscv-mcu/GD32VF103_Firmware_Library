/*!
    \file  readme.txt
    \brief description of the TIMER0 complementary signals demo

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

  This demo is based on the GD32VF103V-EVAL-V1.0 board, it shows how to configure the 
TIMER0 peripheral to generate three complementary TIMER0 signals. TIMER0CLK is fixed 
to systemcoreclock, the TIMER0 prescaler is equal to 5400, so the TIMER0 counter clock 
used is 20KHz and the PWM frequency is 1.25Hz.

  The Three duty cycles are computed as the following description: 
  The channel 0 duty cycle is set to 25% so channel 0N is set to 75%.
  The channel 1 duty cycle is set to 50% so channel 1N is set to 50%.
  The channel 2 duty cycle is set to 75% so channel 2N is set to 25%.
 
  Connect the TIMER0 pins to an oscilloscope to monitor the different waveforms:
    - TIMER0_CH0  pin(PA8)
    - TIMER0_CH0N pin(PB13)
    - TIMER0_CH1  pin(PA9)
    - TIMER0_CH1N pin(PB14)
    - TIMER0_CH2  pin(PA10)
    - TIMER0_CH2N pin(PB15)
