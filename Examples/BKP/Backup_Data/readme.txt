/*!
    \file    readme.txt
    \brief   description of the Backup_Data demo

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

  This demo is based on the GD32VF103V-EVAL-V1.0 board, it shows how to store user data 
in the Backup data registers.
  As the Backup domain still powered by VBAT when VDD is switched off, its contents
are not lost if a battery is connected to VBAT pin. When JP0 is connected to Vmcu, 
the board is powered up, LED2, LED3 and LED4 are on. After an external reset, LED2
and LED3 are off, LED4 is on. Change JP0 connected to external battery, the board is
executed a power-down and power-up operation, LED1, LED3 and LED4 are on.
  
  The program behaves as follows:
  1. After startup the program checks if the board has been powered up. If yes, the
  values in the BKP data registers are checked:
   - if a battery is connected to the VBAT pin, the values in the BKP data registers
     are retained
   - if no battery is connected to the VBAT pin, the values in the BKP data registers
     are lost
  2. After an external reset, the BKP data registers contents are not checked.

  Four LEDs are used to show the system state as follows:
  1. LED3 on / LED1 on: a POR/PDR reset occurred and the values in the BKP data
   registers are correct
  2. LED3 on / LED2 on: a POR/PDR reset occurred and the values in the BKP data
   registers are not correct or they have not yet been programmed (if it is the
   first time the program is executed)
  3. LED3 off / LED1 off / LED3 off: no POR/PDR reset occurred
  4. LED4 on: program running
  
  BT1 should have a 3.3V battery, JP0 can change the VBAT source.  

