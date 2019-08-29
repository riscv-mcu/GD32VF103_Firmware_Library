/*!
    \file  readme.txt
    \brief description of the USB CDC_ACM demo

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

   This example is based on the GD32VF103V-EVAL-V1.0 board,it provides a description of 
 how to use the USBFS.

  The target of this example is to read data from and write data to USB devices using 
the CDC protocol.
 
  It makes the USB device look like a serial port (NO serial cable connectors: You 
can see the data transferred to and from via USB instead of USB-to-USART bridge connection).
 
  This example loops back the contents of a text file over usb port. To run the example, 
Type a message using the Pc's keyboard. Any data that shows in HyperTerminal is received 
from the device.

  This CDC_ACM Demo provides the firmware examples for the GD32VF103V families.

  - OUT transfers (receive the data from the PC to GD32):
  When a packet is received from the PC on the OUT pipe (EP3),by calling cdc_acm_data_receive()
  it will be stored in the usb_data_buffer[]. 
 
  - IN transfers (to send the data received from the GD32 to the PC):
  When a packet is sent from the GD32 on the IN pipe (EP1), by calling cdc_acm_data_send(),
  put the data into the usb_data_buffer[] buffer for sending data to the host.
