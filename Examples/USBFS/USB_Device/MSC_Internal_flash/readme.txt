/*!
    \file  readme.txt
    \brief description of the USB MSC device(Udisk) demo

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

  This demo is based on the GD32VF103V-EVAL-V1.0 board,it provides a description of 
how to use the USB device application based on the Mass Storage Class (MSC).

  The GD32 MCU is enumerated as a MSC device using the native PC Host MSC driver
to which the GD32VF103V-EVAL-V1.0 board is connected.

  It illustrates an implementation of the MSC class which uses the bulk transfer
while the internal sram is used as storage media.

  This example supports the BOT (bulk only transfer) protocol and all needed SCSI
(small computer system interface) commands, and is compatible with Windows platform.

  After running the example, the user just has to plug the USB cable into a PC host
and the device is automatically detected. One new removable drives appear in the
system window and write/read/format operations can be performed as with any other
removable drive.

  To select the appropriate USB Core to work with, user must add the following macro 
defines within the compiler preprocessor (already done in the preconfigured projects 
provided with this application):
  - "USE_USB_FS" when using USB Full Speed (FS) Core

  In order to make the program work, you must do the following :
    - Open your preferred toolchain 

    - Rebuild all files and load your image into target memory

    - Run the application
