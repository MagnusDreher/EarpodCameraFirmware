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
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define VIDEO_FPS 30
#define VIDEO_MAX_FRAME_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT * 2)  // Rough estimate for uncompressed; adjust for JPEG

// Device Descriptor
const tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,  // USB 2.0
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,  // Espressif VID (or your own)
    .idProduct = 0x4003, // Custom PID for composite UAC+UVC
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 1,
};

// String Descriptors (array of pointers)
const char *string_descriptor[] = {
    (const char[]){0x09, 0x04},  // 0: Language ID (US English)
    "Espressif",                  // 1: Manufacturer
    "Earpod Camera Composite Device",  // 2: Product
    "1234567890",                 // 3: Serial Number
    "UAC Audio Interface",        // 4: UAC string
    "UVC Video Interface",        // 5: UVC string
    // Add more if needed
};

// Configuration Descriptor (composite: UAC + UVC)
uint8_t desc_configuration[] = {
    // Configuration Descriptor

    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, sizeof(desc_configuration), TUSB_DESC_CONFIG_ATT_SELF_POWERED | TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),  // 500mA max power

    // IAD for UAC (associates 3 interfaces: control, speaker streaming, mic streaming)
    TUD_AUDIO_DESC_IAD(ITF_NUM_AUDIO_CONTROL, 3, 0x00),

    // UAC Headset Descriptor (Speaker + Microphone, stereo)
    TUD_AUDIO_HEADSET_STEREO_DESCRIPTOR(ITF_NUM_AUDIO_CONTROL, 4,  // String index for UAC
                                        ITF_NUM_AUDIO_STREAMING_SPK, EPNUM_AUDIO_SPK, CFG_TUD_AUDIO_EP_SZ, CFG_TUD_AUDIO_RX_FIFO_SZ,  // Speaker (RX)
                                        ITF_NUM_AUDIO_STREAMING_MIC, EPNUM_AUDIO_MIC, CFG_TUD_AUDIO_EP_SZ, CFG_TUD_AUDIO_TX_FIFO_SZ   // Microphone (TX)
                                        ),

    // IAD for UVC (associates 2 interfaces: video control, video streaming)
    TUD_VIDEO_DESC_IAD(ITF_NUM_VIDEO_CONTROL, 2, 0x00),

    // UVC Video Control Interface Descriptor
    TUD_VIDEO_DESC_STD_VC(ITF_NUM_VIDEO_CONTROL, 5,  // String index for UVC
                          0x0100,  // bcdUVC 1.00
                          0x00,    // No trigger support
                          1,       // One streaming interface
                          ITF_NUM_VIDEO_STREAMING),

    // UVC Video Streaming Interface (Zero Bandwidth Alternate Setting)
    TUD_VIDEO_DESC_STD_VS(ITF_NUM_VIDEO_STREAMING, 0,  // Alt 0: zero bandwidth
                          0,  // No endpoints
                          0), // No endpoint size

    // UVC Format Descriptor for MJPEG (adjust GUID and parameters)
    TUD_VIDEO_DESC_CS_VS_FMT_MJPEG(1,  // Format index
                                   1,  // Number of frame descriptors following
                                   0,  // String index
                                   8,  // Bits per pixel (for JPEG, variable but often 8)
                                   0,  // Default frame index
                                   VIDEO_WIDTH, VIDEO_HEIGHT,  // Min/max width/height (fixed for simplicity)
                                   VIDEO_WIDTH, VIDEO_HEIGHT,
                                   VIDEO_MAX_FRAME_SIZE, VIDEO_MAX_FRAME_SIZE,  // Min/max frame size
                                   (10000000 / VIDEO_FPS),  // Frame interval (100ns units)
                                   1, VIDEO_FPS,  // Min/max fps
                                   VIDEO_MAX_FRAME_SIZE * VIDEO_FPS),  // Max bit rate

    // UVC Frame Descriptor (example for one resolution)
    TUD_VIDEO_DESC_CS_VS_FRM_CONT(1,  // Frame index
                                  0,  // Still image capture not supported
                                  VIDEO_WIDTH, VIDEO_HEIGHT,  // Width/height
                                  VIDEO_MAX_FRAME_SIZE, VIDEO_MAX_FRAME_SIZE,  // Min/max frame size
                                  (10000000 / VIDEO_FPS),  // Default frame interval
                                  1,  // Continuous frame interval
                                  (10000000 / VIDEO_FPS),  // Min frame interval
                                  (10000000 / VIDEO_FPS),  // Max frame interval
                                  10000000),  // Steps in 100ns

    // UVC Video Streaming Interface (Operational Alternate Setting)
    TUD_VIDEO_DESC_STD_VS(ITF_NUM_VIDEO_STREAMING, 1,  // Alt 1: operational
                          1,  // One endpoint
                          EPNUM_VIDEO, TUSB_DIR_IN, 512, 1),  // IN endpoint, bulk, 512 bytes max, polling interval 1
};

// Function to calculate actual CONFIG_TOTAL_LEN (call this in init if needed, or compute statically)
uint16_t get_config_total_len() {
    // In practice, compute sum of lengths from macros or sizeof(desc_configuration) after definition
    return sizeof(desc_configuration);
}
#define CONFIG_TOTAL_LEN sizeof(desc_configuration)
