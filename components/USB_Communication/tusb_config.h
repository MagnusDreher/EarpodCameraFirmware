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
//====================================================================
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP          1
#define CFG_TUD_AUDIO_ENABLE_EP_OUT               1    // enables tud_audio_available, tud_audio_read, tud_audio_clear_ep_out_ff, audio_feedback_params_t
#define CFG_TUD_AUDIO_ENABLE_EP_IN                1    // enables tud_audio_get_ep_in_ff, tud_audio_write
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT              2
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ           64

#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX         1    // Microphone (device->host)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX         1    // Speaker   (host->device)

#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE           48000

// Speaker OUT (host -> device, "RX")
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX    2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX             16
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT                 100

// Microphone IN (device -> host, "TX")
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX    2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX             16
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN                  100

// Backwards-compatibility aliases
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_TX   CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_RX   CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN  232
// Es kommt automatisch aus managed_components/espressif__usb_device_uac/include/uac_descriptors.h
// (via CMakeLists.txt target_include_directories BEFORE).

//====================================================================
//  UVC Configuration (usb_device_uvc v1.1.3)
//====================================================================
#define CFG_TUD_VIDEO_STREAMING                    1
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE         2048
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE    CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE

// Anzahl der Kameras

#define UVC_CAM_NUM          1

#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS  1

// UVC_CAM2_FRAME_RATE wird von usb_device_uvc.c benoetigt auch wenn nur 1 Kamera vorhanden
#define UVC_CAM2_FRAME_RATE  0

#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS             1   // war komplett fehlend
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX         100 // aktiviert tud_audio_read
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX          100 // aktiviert tud_audio_write
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ      400 // aktiviert tud_audio_available
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ       400 // aktiviert tud_audio_get_ep_in_ff

