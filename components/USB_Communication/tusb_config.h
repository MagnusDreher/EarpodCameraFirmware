#pragma once

//--------------------------------------------------------------------+
//  MCU & USB Mode
//--------------------------------------------------------------------+
#define CFG_TUD_ENABLED        1
#define CFG_TUSB_MCU           OPT_MCU_ESP32S3

// ESP32-S3: Port 1 = High Speed (internes HSPHY)
#define CFG_TUSB_RHPORT1_MODE  (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)

//--------------------------------------------------------------------+
//  OS & Memory
//--------------------------------------------------------------------+
#define CFG_TUSB_OS            OPT_OS_FREERTOS
#define CFG_TUSB_OS_INC_PATH   freertos/

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN     __attribute__((aligned(4)))

//--------------------------------------------------------------------+
//  Endpoint 0
//--------------------------------------------------------------------+
#define CFG_TUD_ENDPOINT0_SIZE 64

//--------------------------------------------------------------------+
//  Enabled USB Classes  <- BEIDE müssen hier aktiviert sein!
//--------------------------------------------------------------------+
#define CFG_TUD_AUDIO           1
#define CFG_TUD_VIDEO           1   // <- MUSS gesetzt sein damit UVC-Makros geladen werden
#define CFG_TUD_VIDEO_STREAMING 1

#define CFG_TUD_CDC             0
#define CFG_TUD_HID             0
#define CFG_TUD_MSC             0
#define CFG_TUD_MIDI            0
#define CFG_TUD_VENDOR          0

//--------------------------------------------------------------------+
//  UVC Konfiguration
//  Diese Defines werden von espressif__usb_device_uvc benoetigt.
//  usb_device_uvc.c und usb_descriptors.c lesen sie aus diesem Header.
//--------------------------------------------------------------------+

// Framerate fuer Kamera 1 (Kamera 2 nicht verwendet)
#define UVC_CAM1_FRAME_RATE              30
#define UVC_CAM2_FRAME_RATE              0

// Transfer-Buffer fuer den USB-Video-Stream (40KB fuer MJPEG 640x480)
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE  (40 * 1024)
#define CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE  0

// Interface-Nummern: Audio belegt 0,1,2 -> Video beginnt bei 3
#define ITF_NUM_VIDEO_CONTROL            3
#define ITF_NUM_VIDEO_STREAMING          4

// Endpoint-Nummer fuer Video-IN (muss verschieden von Audio-Endpoints sein)
// Audio: 0x01 (SPK OUT), 0x81 (MIC IN) -> Video: 0x82
#define EPNUM_CAM1_VIDEO_IN              0x82

// Anzahl der MJPEG-Aufloesungen im UVC-Descriptor
#define UVC_CAM1_MULTI_FRAMESIZE_COUNT   4

//--------------------------------------------------------------------+
//  UAC Konfiguration
//  Diese Defines werden von espressif__usb_device_uac benoetigt.
//--------------------------------------------------------------------+
#define CFG_TUD_AUDIO_IN_PATH                      1
#define CFG_TUD_AUDIO_OUT_PATH                     1

#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT              2
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ           64

// WICHTIG: Konsistent 48kHz! I2S in microphone.cpp und speaker.cpp
// laufen auf 48kHz -> hier muss dasselbe stehen, sonst verzerrter Sound.
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE       48000
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE           48000

// Mikrofon (IN = Device->Host = TX aus TinyUSB-Sicht)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX         1
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_TX   2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_TYPE_TX        AUDIO_FORMAT_TYPE_I_PCM

// Speaker (OUT = Host->Device = RX aus TinyUSB-Sicht)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX         1
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_RX   2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_TYPE_RX        AUDIO_FORMAT_TYPE_I_PCM

// EP-Groesse: 48000Hz * 2 Bytes * 1 Kanal / 1000ms = 96 Bytes/ms + 2 Header = 98
#define CFG_TUD_AUDIO_EP_SZ_IN                     (48*2*1 + 2)
#define CFG_TUD_AUDIO_EP_SZ_OUT                    (48*2*1 + 2)

// Software-FIFO (4x EP-Groesse = ~400 Bytes Puffer)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ      (CFG_TUD_AUDIO_EP_SZ_IN  * 4)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ     (CFG_TUD_AUDIO_EP_SZ_OUT * 4)

// Feedback-Endpoint fuer Audio-Synchronisation mit dem Host
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP           1

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN              TUD_AUDIO_DESC_LEN






