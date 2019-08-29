/*!
    \file  audio_core.h
    \brief the header file of USB audio device class core functions

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

#ifndef AUDIO_CORE_H
#define AUDIO_CORE_H

#include "usbd_enum.h"
#include "usb_ch9_std.h"
#include "usbd_transc.h"

#define USB_SPEAKER_CONFIG_DESC_SIZE                       109
#define FORMAT_24BIT(X)  (uint8_t)(X);(uint8_t)(X >> 8);(uint8_t)(X >> 16)

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define AUDIO_OUT_PACKET                             (uint32_t)(((USBD_AUDIO_FREQ * 2 * 2) / 1000))

/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
   that it is an even number and higher than 3 */
#define OUT_PACKET_NUM                               4

/* Total size of the audio transfer buffer */
#define TOTAL_OUT_BUF_SIZE                           ((uint32_t)(AUDIO_OUT_PACKET * OUT_PACKET_NUM))

#define AUDIO_CONFIG_DESC_SIZE                       109
#define AUDIO_INTERFACE_DESC_SIZE                    9
#define USB_AUDIO_DESC_SIZ                           0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE            0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE           0x07

#define AUDIO_DESCRIPTOR_TYPE                        0x21
#define USB_DEVICE_CLASS_AUDIO                       0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                  0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                0x02
#define AUDIO_PROTOCOL_UNDEFINED                     0x00
#define AUDIO_STREAMING_GENERAL                      0x01
#define AUDIO_STREAMING_FORMAT_TYPE                  0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE              0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE               0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                         0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                 0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                0x03
#define AUDIO_CONTROL_FEATURE_UNIT                   0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE               0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE              0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE          0x07

#define AUDIO_CONTROL_MUTE                           0x0001

#define AUDIO_FORMAT_TYPE_I                          0x01
#define AUDIO_FORMAT_TYPE_III                        0x03

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                0x01
#define AUDIO_ENDPOINT_GENERAL                       0x01

#define AUDIO_REQ_GET_CUR                            0x81
#define AUDIO_REQ_SET_CUR                            0x01
#define NO_CMD                                       0xFF

#define AUDIO_OUT_STREAMING_CTRL                     0x02

#define DEVICE_ID                     (0x40022100)

#define PACKET_SIZE(freq)             ((freq * 2) * 2 / 1000)

#define AUDIO_PACKET_SIZE(frq)        (uint8_t)(PACKET_SIZE(frq) & 0xFF), \
                                      (uint8_t)((PACKET_SIZE(frq) >> 8) & 0xFF)

#define SAMPLE_FREQ(frq)              (uint8_t)(frq), (uint8_t)((frq >> 8)), \
                                      (uint8_t)((frq >> 16))


#pragma pack(1)

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< HEADER descriptor subtype */
    uint16_t bcdADC;                      /*!< Audio 1.0 */
    uint16_t wTotalLength;                /*!< Total number of bytes returned */
    uint8_t  bInCollection;               /*!< One OUT AudioStreaming interface in the Collection */
    uint8_t  baInterfaceNr;               /*!< Interface number of the OUT AudioStreaming interface in the Collection */
} usb_descriptor_AC_interface_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< AS_GENERAL subtype */
    uint8_t  bTerminalLink;               /*!< Terminal ID of the Input Terminal, associated with this inetrface */
    uint8_t  bDelay;                      /*!< Total interface delay, expressed in frames */
    uint16_t wFormatTag;                  /*!< PCM Format */
} usb_descriptor_AS_interface_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< INPUT_TERMINAL descriptor subtype */
    uint8_t  bTerminalID;                 /*!< ID of this Input Terminal */
    uint16_t wTerminalType;               /*!< Terminal Type */
    uint8_t  bAssocTerminal;              /*!< No association */
    uint8_t  bNrChannels;                 /*!< Signal path */
    uint16_t wChannelConfig;              /*!< The Front channel */
    uint8_t  iChannelNames;               /*!< Unused */
    uint8_t  iTerminal;                   /*!< Index of a string descriptor,describing the Input Terminal */
} usb_descriptor_input_terminal_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< OUT_TERMINAL descriptor subtype */
    uint8_t  bTerminalID;                 /*!< ID of this Input Terminal */
    uint16_t wTerminalType;               /*!< Headphones Terminal Type */
    uint8_t  bAssocTerminal;              /*!< No association */
    uint8_t  bSourceID;                   /*!< From Feature Unit ID2 */
    uint8_t  iTerminal;                   /*!< Index of a string descriptor,describing the Output Terminal */
} usb_descriptor_output_terminal_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< FEATURE_UNIT descriptor subtype */
    uint8_t  bUnitID;                     /*!< ID of this Feature Uint */
    uint8_t  bSourceID;                   /*!< The ID of the Entity to which this Feature Unit is connected */
    uint8_t  bControlSize;                /*!< 2bytes for each element of the bmaControls array */
    uint8_t bmaControls0;                 /*!< Mute Control on Master Channel */
    uint8_t bmaControls1;                 /*!< Volume Control on Center Front channel */
    uint8_t  iFeature;                    /*!< Index of a string descriptor, describing the Feature Unit */
} usb_descriptor_mono_feature_unit_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< FEATURE_UNIT descriptor subtype */
    uint8_t  bUnitID;                     /*!< ID of this Feature Uint */
    uint8_t  bSourceID;                   /*!< The ID of the Entity to which this Feature Unit is connected */
    uint8_t  bControlSize;                /*!< 2bytes for each element of the bmaControls array */
    uint16_t bmaControls0;                /*!< Mute Control on Master Channel */
    uint16_t bmaControls1;                /*!< Volume Control on Left Front channel */
    uint16_t bmaControls2;                /*!< Volume Control on Right Front channel */
    uint8_t  iFeature;                    /*!< Index of a string descriptor, describing the Feature Unit */
} usb_descriptor_stereo_feature_unit_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< FORMAT_TYPE subtype */
    uint8_t  bFormatType;                 /*!< The index of FORMAT_TYPE */
    uint8_t  bNrChannels;                 /*!< The number of channel */
    uint8_t  bSubFrameSize;               /*!< Two bytes per audio subframe */
    uint8_t  bBitResolution;              /*!< The bits number per sample */
    uint8_t  bSamFreqType;                /*!< One frequency supported */
    uint8_t  bSamFreq[3];                 /*!< The frequency of sampling */
} usb_descriptor_format_type_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bEndpointAddress;            /*!< logical address of the endpoint */
    uint8_t  bmAttributes;                /*!< endpoint attributes */
    uint16_t wMaxPacketSize;              /*!< size of the endpoint bank, in bytes */
    uint8_t  bInterval;                   /*!< polling interval in milliseconds for the endpoint if it is an INTERRUPT or ISOCHRONOUS type */
    uint8_t  bRefresh;                    /*!< Unused */
    uint8_t  bSynchAddress;               /*!< Unused */
} usb_descriptor_std_endpoint_struct;

typedef struct
{
    usb_desc_header header;  /*!< descriptor header, including type and size */

    uint8_t  bDescriptorSubtype;          /*!< AS_ENDPOINT subtype */
    uint8_t  bmAttributes;                /*!< endpoint attributes */
    uint8_t  bLockDelayUnits;             /*!< Unused */
    uint16_t wLockDelay;                  /*!< Unused */
} usb_descriptor_AS_endpoint_struct;

#pragma pack()

/* USB configuration descriptor struct */
typedef struct
{
    usb_desc_config                            Config;
    usb_desc_itf                               Speaker_Std_Interface;
    usb_descriptor_AC_interface_struct         Speaker_AC_Interface;
    usb_descriptor_input_terminal_struct       Speaker_IN_Terminal;
    usb_descriptor_mono_feature_unit_struct    Speaker_Feature_Unit;
    usb_descriptor_output_terminal_struct      Speaker_OUT_Terminal;
    usb_desc_itf                               Speaker_Std_AS_Interface_ZeroBand;
    usb_desc_itf                               Speaker_Std_AS_Interface_Opera;
    usb_descriptor_AS_interface_struct         Speaker_AS_Interface;
    usb_descriptor_format_type_struct          Speaker_Format_TypeI;
    usb_descriptor_std_endpoint_struct         Speaker_Std_Endpoint;
    usb_descriptor_AS_endpoint_struct          Speaker_AS_Endpoint;
} usb_descriptor_configuration_set_struct;

/* AUDIO_FOPS_TypeDef definitions */
typedef struct _Audio_Fops
{
    uint8_t  (*Init)       (uint32_t AudioFreq, uint32_t Volume, uint32_t options);
    uint8_t  (*DeInit)     (uint32_t options);
    uint8_t  (*AudioCmd)   (uint8_t* pbuf, uint32_t size, uint8_t cmd);
    uint8_t  (*VolumeCtl)  (uint8_t vol);
    uint8_t  (*MuteCtl)    (uint8_t cmd);
    uint8_t  (*PeriodicTC) (uint8_t cmd);
    uint8_t  (*GetState)   (void);
}AUDIO_FOPS_TypeDef;

extern usb_class_core usbd_audio_cb;
extern uint8_t usbd_serial_string[USB_SERIAL_STRING_SIZE];
extern void* const usbd_strings[USB_STRING_COUNT];
extern const usb_desc_dev device_descripter;
extern const usb_descriptor_configuration_set_struct configuration_descriptor;

uint8_t audio_init (usb_dev *pudev, uint8_t config_index);
uint8_t audio_deinit (usb_dev *pudev, uint8_t config_index);
uint8_t audio_req_handler (usb_dev *pudev, usb_req *req);
uint8_t audio_data_in_handler (usb_dev *pudev, uint8_t ep_id);
uint8_t audio_data_out_handler (usb_dev *pudev, uint8_t ep_id);

#endif
