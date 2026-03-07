// Includes
#include "tusb.h"
#include "device/usbd.h"         // General device descriptors (e.g., TUD_CONFIG_DESCRIPTOR, TUD_*_IAD)
#include "class/audio/audio_device.h"  // UAC-specific macros (e.g., TUD_AUDIO_HEADSET_STEREO_DESCRIPTOR)
#include "class/video/video_device.h"  // UVC-specific macros (e.g., TUD_VIDEO_DESC_STD_VC, TUD_VIDEO_DESC_CS_VS_FMT_MJPEG)

// Define interface numbers to avoid conflicts
#define ITF_NUM_AUDIO_CONTROL 0
#define ITF_NUM_AUDIO_STREAMING_SPK 1
#define ITF_NUM_AUDIO_STREAMING_MIC 2
#define ITF_NUM_VIDEO_CONTROL 3
#define ITF_NUM_VIDEO_STREAMING 4
#define ITF_NUM_TOTAL 5

// Define endpoints (no overlaps)
#define EPNUM_AUDIO_SPK 0x01  // OUT for speaker (host to device)
#define EPNUM_AUDIO_MIC 0x81  // IN for microphone (device to host)
#define EPNUM_VIDEO 0x82      // IN for video streaming (device to host)

// Audio parameters (adjust based on your mic/speaker, e.g., 16-bit, 48kHz, stereo)
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_BIT_DEPTH 16
#define AUDIO_CHANNELS 2  // Stereo for headset

// Video parameters (example for MJPEG, adjust resolutions/framerates)
#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 240
#define VIDEO_FPS 15
#define VIDEO_MAX_FRAME_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT * 2)  // Rough estimate for uncompressed; adjust for JPEG


// Configuration Descriptor (composite: UAC + UVC)
static const tusb_desc_device_t s_device_descriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,           // USB 2.0
    .bDeviceClass       = TUSB_CLASS_MISC,  // Composite Device
    .bDeviceSubClass    = 0x02,             // Common Class
    .bDeviceProtocol    = 0x01,             // IAD Protocol
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x303A,           // Espressif VID
    .idProduct          = 0x8001,           // Custom PID
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

/* ========================================================================
 * String Descriptors
 * ======================================================================== */
static const char *s_string_descriptors[] = {
    "\x09\x04",                // 0: Sprache (Englisch)
    "Earpod-Cam",              // 1: Hersteller
    "Earpod Camera Prototype", // 2: Produkt
    "000001",                  // 3: Seriennummer
    "Video Control",           // 4: Video Control Interface
    "Video Streaming",         // 5: Video Streaming Interface
    "Audio Control",           // 6: Audio Control Interface
    "Audio Streaming",         // 7: Audio Streaming Interface
};

// Function to calculate actual CONFIG_TOTAL_LEN (call this in init if needed, or compute statically)
uint16_t get_config_total_len() {
    // In practice, compute sum of lengths from macros or sizeof(desc_configuration) after definition
    return sizeof(s_device_descriptor);
}
#define CONFIG_TOTAL_LEN sizeof(desc_configuration)
