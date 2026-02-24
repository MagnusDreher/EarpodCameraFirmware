#pragma once
#include <stdint.h>
//--------------------------------------------------------------------+
//  MCU & USB Mode
//--------------------------------------------------------------------+
#define CFG_TUD_ENABLED              1
#define CFG_TUSB_MCU                 OPT_MCU_ESP32S3
#define CFG_TUSB_RHPORT0_MODE        OPT_MODE_DEVICE

//--------------------------------------------------------------------+
//  Memory
//--------------------------------------------------------------------+
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN           __attribute__((aligned(4)))

//--------------------------------------------------------------------+
//  Endpoint 0
//--------------------------------------------------------------------+
#define CFG_TUD_ENDPOINT0_SIZE       64

//--------------------------------------------------------------------+
//  Enabled USB Classes
//--------------------------------------------------------------------+
#define CFG_TUD_AUDIO                1
#define CFG_TUD_VIDEO                1
#define CFG_TUD_CDC                  0
#define CFG_TUD_HID                  0
#define CFG_TUD_MSC                  0
#define CFG_TUD_MIDI                 0
#define CFG_TUD_VENDOR               0

//====================================================================
//  UAC Audio Configuration (usb_device_uac v1.2.2)
//  NOTE: v1.2.2 renamed the audio config defines to FORMAT_1 style
//====================================================================

// General UAC settings
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT              2
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ           64

// Number of channels
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX         1    // Microphone (device→host)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX         1    // Speaker   (host→device)

// Sample rate (must match I2S config in microphone.cpp / speaker.cpp)
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE           48000

// ---- NEW FORMAT_1 defines required by uac_descriptors.h in v1.2.2 ----
// These replace the old CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_* names.
//
// Audio format: 16-bit PCM, 48 kHz, mono (1 channel)
// Endpoint size at Full-Speed = (48 samples + 1 extra) × 2 bytes × 1 channel = 98 bytes
// Using 100 as a round safe value above 98.

// Speaker OUT (host → device, "RX" in TinyUSB terms)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX    2    // 16-bit = 2 bytes
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX             16   // 16-bit resolution
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT                 100  // endpoint packet size (bytes)

// Microphone IN (device → host, "TX" in TinyUSB terms)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX    2    // 16-bit = 2 bytes
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX             16   // 16-bit resolution
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN                  100  // endpoint packet size (bytes)

// Keep old-style names as aliases so nothing else breaks
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_TX   CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_RX   CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX

//====================================================================
//  UVC Runtime Defines + Struct (für usb_device_uvc.c)
//====================================================================
#define UVC_CAM1_FRAME_RATE  30
#define UVC_CAM2_FRAME_RATE  0
#define UVC_FRAME_NUM        1
#define UVC_CAM_NUM          1

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  fps;
    uint32_t rate;          // <--- fehlte! (wird in tud_video_commit_cb verwendet)
} uvc_frame_info_t;

const uvc_frame_info_t UVC_FRAMES_INFO[1][1] = {{
    {640, 480, 30, 640*480*2*30}   // rate ≈ bytes/sec für 30 fps
}};

#define CFG_TUD_VIDEO_STREAMING                    1
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE         2048


#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN   (TUD_AUDIO_DESC_IAD_LEN + \
                                         TUD_AUDIO_DESC_STD_AC_LEN + \
                                         TUD_AUDIO_DESC_CS_AC_LEN + \
                                         TUD_AUDIO_DESC_CLK_SRC_LEN + \
                                         TUD_AUDIO_DESC_INPUT_TERM_LEN + \
                                         TUD_AUDIO_DESC_FEATURE_UNIT_ONE_CHANNEL_LEN + \
                                         TUD_AUDIO_DESC_OUTPUT_TERM_LEN + \
                                         TUD_AUDIO_DESC_STD_AS_INT_LEN * 2 + \
                                         TUD_AUDIO_DESC_CS_AS_INT_LEN + \
                                         TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN + \
                                         TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN + \
                                         TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN)


#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE   CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE
