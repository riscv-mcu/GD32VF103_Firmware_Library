/*!
    \file  readme.txt
    \brief This file provides the Audio Out (palyback) interface API

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

  @brief 
    The Audio device example allows device to communicate with host (PC) as USB headphone
  using isochronous pipe for audio data transfer along with some control commands (i.e.
  Mute). 
    It follows the "Universal Serial Bus Device Class Definition for Audio Devices
  Release 1.0 March 18, 1998" defined by the USB Implementers Forum for reprogramming
  an application through USB-FS-Device. 
    Following this specification, it is possible to manage only Full Speed USB mode 
  (High Speed is not supported). 

    This class is natively supported by most Operating Systems (no need for specific
  driver setup).
  
    This example uses the I2S interface to stream audio data from USB Host to the audio
  codec implemented on the evaluation board. Then audio stream is output to the Headphone.
 
    For the GD32VF103V-EVAL-V1.0 board, it possible to use one of the two quartz belwo:
    - 14.7456MHz which provides best audio quality
    - Standard 25MHz which provides lesser quality.

    The device supports one audio frequency (the host driver manages the sampling rate
  conversion from original audio file sampling rate to the sampling rate supported
  by the device). It is possible to configure this audio frequency by modifying the
  usbd_conf.h file (define USBD_AUDIO_FREQ). It is advised to set high frequencies
  to guarantee a high audio quality.  
    It is also possible to modify the default volume through define DEFAULT_VOLUME in file
  usbd_conf.h.

@note: The audio frequencies leading to non integer number of data (44.1KHz, 22.05KHz, 
       11.025KHz...) will not allow an optimum audio quality since one data will be lost
       every two/more frames.
