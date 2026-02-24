#pragma once

//--------------------------------------------------------------------+
//  MCU & USB Mode
//--------------------------------------------------------------------+
#define CFG_TUD_ENABLED 1

#define CFG_TUSB_MCU           OPT_MCU_ESP32S3

// ESP32-S3 hat einen eingebauten USB-PHY auf Port 1 (High Speed)
// Port 0 = Full Speed intern, Port 1 = High Speed (über internes HSPHY)
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
//  Enabled USB Classes
//--------------------------------------------------------------------+
#define CFG_TUD_AUDIO          1
#define CFG_TUD_VIDEO          1
#define CFG_TUD_VIDEO_STREAMING 1

// Alles andere deaktivieren
#define CFG_TUD_CDC            0
#define CFG_TUD_HID            0
#define CFG_TUD_MSC            0
#define CFG_TUD_MIDI           0
#define CFG_TUD_VENDOR         0

//--------------------------------------------------------------------+
//  UVC (Video) Konfiguration
//  WICHTIG: Diese Defines werden von der espressif__usb_device_uvc
//  Bibliothek in deren usb_descriptors.c benötigt!
//--------------------------------------------------------------------+

// Nur eine Kamera (CAM1), CAM2 deaktiviert
#define UVC_CAM1_FRAME_RATE              30
#define UVC_CAM2_FRAME_RATE              0   // Zweite Kamera nicht verwendet

// Endpoint-Buffergröße für den Video-Stream (Bulk: 512 Bytes max pro Paket,
// aber der interne Buffer kann größer sein)
// 40KB ist ein guter Wert für MJPEG bei 640x480
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE  (40 * 1024)
#define CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE  0

// Interface-Nummern für das Video-Control Interface
// (Wird von der UVC-Bibliothek's usb_descriptors.c benötigt)
// Diese müssen NACH den Audio-Interfaces kommen (Audio belegt 0,1,2)
#define ITF_NUM_VIDEO_CONTROL            3
#define ITF_NUM_VIDEO_STREAMING          4

// Endpoint-Nummer für Video-IN (Device → Host)
// Muss sich von Audio-Endpoints (0x01 OUT, 0x81 IN) unterscheiden!
#define EPNUM_CAM1_VIDEO_IN              0x82

// Anzahl der konfigurierten Auflösungen/Framerates in den UVC-Descriptoren
// Die Bibliotheks-usb_descriptors.c erwartet diesen Wert
#define UVC_CAM1_MULTI_FRAMESIZE_COUNT   4

//--------------------------------------------------------------------+
//  UAC (Audio) Konfiguration
//--------------------------------------------------------------------+
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT              2
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ           64

// Mikrofon (IN, TX aus Gerät-Perspektive)
#define CFG_TUD_AUDIO_IN_PATH                      1
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX         1       // Mono-Mikrofon
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_TX   2       // 16-bit
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_TYPE_TX        AUDIO_FORMAT_TYPE_I_PCM

// Speaker (OUT, RX aus Gerät-Perspektive)  
#define CFG_TUD_AUDIO_OUT_PATH                     1
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX         1       // Mono-Speaker
#define CFG_TUD_AUDIO_FUNC_1_BYTES_PER_SAMPLE_RX   2       // 16-bit
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_TYPE_RX        AUDIO_FORMAT_TYPE_I_PCM

// WICHTIG: Konsistent 48kHz (I2S läuft auch auf 48kHz!)
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE       48000
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE           48000

// EP-Größe: 48000 Hz * 2 Bytes * 1 Kanal / 1000ms = 96 Bytes pro ms
// + 2 Bytes Header für UAC2 Feedback → 98, aufrunden auf 100
#define CFG_TUD_AUDIO_EP_SZ_IN                     (48*2*1 + 2)
#define CFG_TUD_AUDIO_EP_SZ_OUT                    (48*2*1 + 2)

// Software FIFO Größen (mindestens EP-Größe)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ       (CFG_TUD_AUDIO_EP_SZ_IN  * 4)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ      (CFG_TUD_AUDIO_EP_SZ_OUT * 4)

// Descriptor-Länge (wird von UAC-Bibliothek benötigt)
#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN              TUD_AUDIO_DESC_LEN

// Feedback-Endpoint für präzises Audio-Timing
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP           1