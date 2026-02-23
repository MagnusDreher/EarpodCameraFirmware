#pragma once

//--------------------------------------------------------------------+
//  MCU & USB Mode
//--------------------------------------------------------------------+
#define CFG_TUD_ENABLED 1     // Enable device mode

#define CFG_TUSB_MCU           OPT_MCU_ESP32S3
#define CFG_TUSB_RHPORT0_MODE  OPT_MODE_DEVICE

//--------------------------------------------------------------------+
//  Memory
//--------------------------------------------------------------------+

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN     __attribute__((aligned(4)))

//--------------------------------------------------------------------+
//  Enabled USB Classes
//--------------------------------------------------------------------+

#define CFG_TUD_AUDIO          1
#define CFG_TUD_VIDEO          1

// Disable everything else
#define CFG_TUD_CDC            0
#define CFG_TUD_HID            0
#define CFG_TUD_MSC            0
#define CFG_TUD_MIDI           0
#define CFG_TUD_VENDOR         0

//--------------------------------------------------------------------+
//  AUDIO Class Configuration
//--------------------------------------------------------------------+

// One Audio Function with:
// - 1 channel Microphone (IN)
// - 1 channel Speaker (OUT)
#define CFG_TUD_AUDIO_IN_PATH       1 // Mikrofon aktivieren
#define CFG_TUD_AUDIO_OUT_PATH      1 // Speaker aktivieren
#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN              TUD_AUDIO_DESC_LEN

#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT              2
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ           64

#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX         1   // Speaker
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX         1   // Microphone

#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE       16000
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE           16000

#define CFG_TUD_AUDIO_FUNC_1_FORMAT_TYPE_TX        AUDIO_FORMAT_TYPE_I_PCM
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_TYPE_RX        AUDIO_FORMAT_TYPE_I_PCM

#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_TX  2
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_RX  2

//--------------------------------------------------------------------+
//  VIDEO (UVC) Configuration
//--------------------------------------------------------------------+
#define CFG_TUD_VIDEO_STREAMING 1
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE  2048
#define CFG_TUD_VIDEO_STREAMING_INTERFACE_COUNT 1


//--------------------------------------------------------------------+
//  Endpoints
//--------------------------------------------------------------------+

#define CFG_TUD_ENDPOINT0_SIZE 64
