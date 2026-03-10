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
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP                    1
// iOS/macOS require 10.14 feedback format (3 bytes) for FS devices.
// TinyUSB computes feedback in 16.16 internally and converts to 10.14 before sending.
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_FORMAT_CORRECTION     1
#define CFG_TUD_AUDIO_ENABLE_EP_OUT               1    // enables tud_audio_available, tud_audio_read, tud_audio_clear_ep_out_ff, audio_feedback_params_t
#define CFG_TUD_AUDIO_ENABLE_EP_IN                1    // enables tud_audio_get_ep_in_ff, tud_audio_write
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT              2
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ           64

#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX         1    // Microphone (device->host), 1ch mono
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX         2    // Speaker   (host->device), 2ch stereo (CONFIG_UAC_SPEAKER_CHANNEL_NUM=2)

#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE           48000

// Speaker OUT (host -> device, "RX")
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX    2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX             16
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_OUT                 196  // 48000/1000 * 2ch * 2bytes + 4 = 196

// Microphone IN (device -> host, "TX")
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX    2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX             16
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN                  100

// Backwards-compatibility aliases
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_TX   CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_RX   CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN  241
// Berechnung fuer SPEAK=2, MIC=1:
// IAD(8)+STD_AC(9)+CS_AC(9)+CS_AC_TOTAL(98)+SPK_ALT0(9)+SPK_ALT1(46)+MIC_ALT0(9)+MIC_ALT1(46)+FB_EP(7) = 241
// audiod_open() gibt (241-8)=233 Bytes zurueck — TinyUSB parser springt korrekt ans Ende des Audio-Abschnitts.

//====================================================================
//  UVC Configuration (usb_device_uvc v1.1.3)
//====================================================================
#define CFG_TUD_VIDEO_STREAMING                    1
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE         256   // Must be <=1023 for USB Full Speed ISO!
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE    CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE

// Anzahl der Kameras

#define UVC_CAM_NUM          1

#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS  1

// UVC_CAM2_FRAME_RATE wird von usb_device_uvc.c benoetigt auch wenn nur 1 Kamera vorhanden
#define UVC_CAM2_FRAME_RATE  0

#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS             1   // war komplett fehlend
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX         196  // muss >= EP_SZ_OUT (196 fuer 2ch speaker)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX          100  // 1ch mic: 48000/1000 * 1 * 2 = 96 + puffer
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ      2156 // 196 * (SPK_INTERVAL_MS+1) = 196*11
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ       1100 // 100 * (MIC_INTERVAL_MS+1) = 100*11

