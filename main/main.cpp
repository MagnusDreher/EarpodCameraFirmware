#include <stdio.h>
//#include "esp_tinyusb.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "microphone.h"
#include "speaker.h"
#include "audiodata_usb.h"
#include "videodata_usb.h"
#include "esp_rom_sys.h"

static const char* TAG = "APP_MAIN";

// Define constants for interface numbers
#define ITF_NUM_AUDIO_CONTROL 0
#define ITF_NUM_AUDIO_STREAMING_SPK 1
#define ITF_NUM_AUDIO_STREAMING_MIC 2
#define ITF_NUM_VIDEO_CONTROL 3
#define ITF_NUM_VIDEO_STREAMING 4
#define ITF_NUM_TOTAL 5

// Define constants for endpoints (ensure no overlap)
#define EPNUM_AUDIO_SPK 0x01
#define EPNUM_AUDIO_MIC 0x81
#define EPNUM_VIDEO 0x82

// Placeholder for total configuration length - calculate based on your descriptors
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_AUDIO_HEADSET_STEREO_DESC_LEN + TUD_VIDEO_DESC_LEN) // Adjust with actual lengths

// Example USB Device Descriptor
/*static tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200, // USB 2.0
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A, // Espressif VID
    .idProduct = 0x4002, // Example PID for composite
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 1,
};

// Example string descriptors (adjust as needed)
static const char *string_descriptor[] = {
    (const char[]) { 0x09, 0x04 }, // 0: Supported Language: English
    "Espressif",                   // 1: Manufacturer
    "Earpod Camera Composite",     // 2: Product
    "123456",                      // 3: Serial
    "UAC Interface",               // 4: UAC string
    "UVC Interface",               // 5: UVC string
    // Add more if needed for UAC/UVC
};

// Composite Configuration Descriptor (combine UAC and UVC)
static uint8_t desc_configuration[] = {
    // Config Descriptor
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_SELF_POWERED | TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // IAD for Audio (to associate control and streaming interfaces)
    TUD_AUDIO_DESC_IAD(ITF_NUM_AUDIO_CONTROL, 3, 0x00), // 3 interfaces for headset (control, spk, mic)

    // UAC Descriptors for Headset (Speaker + Mic)
    TUD_AUDIO_HEADSET_STEREO_DESCRIPTOR(ITF_NUM_AUDIO_CONTROL, 4, // str index
                                        ITF_NUM_AUDIO_STREAMING_SPK, EPNUM_AUDIO_SPK, CFG_TUD_AUDIO_EP_SZ, CFG_TUD_AUDIO_RX_FIFO_SZ, // spk
                                        ITF_NUM_AUDIO_STREAMING_MIC, EPNUM_AUDIO_MIC, CFG_TUD_AUDIO_EP_SZ, CFG_TUD_AUDIO_TX_FIFO_SZ // mic
                                        ), 

    // IAD for Video
    TUD_VIDEO_DESC_IAD(ITF_NUM_VIDEO_CONTROL, 2, 0x00),

    // UVC Descriptors (example for MJPEG, adjust for your camera)
    TUD_VIDEO_DESC_STD_VC(ITF_NUM_VIDEO_CONTROL, 5, // str index
                          0x0100, // bcdUVC 1.00
                          0x00, // bTriggerSupport
                          1, // 1 streaming interface
                          ITF_NUM_VIDEO_STREAMING),

    // Video Streaming Interface
    TUD_VIDEO_DESC_STD_VS(ITF_NUM_VIDEO_STREAMING, 0, // alt 0 = zero bandwidth
                          0, // no endpoint for zero bandwidth
                          0), // no endpoint size

    // Add more for alt settings with bulk/isoc endpoint
    TUD_VIDEO_DESC_CS_VS_FMT_UNCOMPRESSED(/* index *//* 1,*/ /* formats */ /* 1,*//* str index */ /* 1,*/ 
                                          /* GUID */ /*0x59,0x55,0x59,0x32,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71, // YUY2
                                          /* bits */ /*16, /* subs */ /* 1,*/ /* keyframe rate */ /* 0,*/ /* pframe rate */ /* 0,*/ /* quality */ /* 0,*/ /* latency */ /* 0,*/ /* max frame size */ /* 0,),*/

    // Add endpoint descriptor for video streaming
/*  TUD_VIDEO_DESC_STD_VS(ITF_NUM_VIDEO_STREAMING, 1, // alt 1 = operational
                          1, // one endpoint
                          EPNUM_VIDEO, TUSB_DIR_IN, 512, 1), // bulk endpoint, max 512 bytes, interval 1
    // Add more as needed
};
*/

static const char* TAG = "APP_MAIN";

/*static esp_err_t usb_stack_init(void)
{
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = &desc_device,
        .string_descriptor = string_descriptor,
        .string_descriptor_count = sizeof(string_descriptor)/sizeof(string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = desc_configuration,
    };

    esp_err_t ret = esp_tinyusb_init(&tusb_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "USB TinyUSB init failed");
    }
    return ret;
}
bool usb_boot_request_received(void)
{
    tud_task();   // TinyUSB Poll

    if (tud_cdc_available()) {
        char buf[8] = {0};
        int len = tud_cdc_read(buf, sizeof(buf));

        if (len >= 4) {
            if (!strncasecmp(buf, "BOOT", 4)) {
                return true;
            }
        }
    }
    return false;
}



bool usb_boot_request_received(void)
{
    tud_task();   // TinyUSB Poll

    if (tud_cdc_available()) {
        char buf[8] = {0};
        int len = tud_cdc_read(buf, sizeof(buf));

        if (len >= 4) {
            if (!strncasecmp(buf, "BOOT", 4)) {
                return true;
            }
        }
    }
    return false;
}*/
void app_main(void)
{
    // usb_stack_init();
   // ESP_LOGI(TAG, "Boot window 3s - send 'BOOT' over USB CDC");

    //TickType_t start = xTaskGetTickCount();

   // while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(3000)) {
    //    if (usb_boot_request_received()) {
    //        enter_bootloader();
    //    }
    //    vTaskDelay(pdMS_TO_TICKS(10));
   // }

    ESP_LOGI(TAG, "Starting Earpod Camera Firmware...");

    
    ESP_LOGI(TAG, "Camera initialized successfully");

    // Initialize audio input (microphone)
    ESP_LOGI(TAG, "Initializing microphone...");
    if (ics43434_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize microphone");
        return;
    }
    ESP_LOGI(TAG, "Microphone initialized successfully");

    // Initialize audio output (speaker)
    ESP_LOGI(TAG, "Initializing speaker...");
    if (MAX98357_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize speaker");
        return;
    }
    ESP_LOGI(TAG, "Speaker initialized successfully");

    // Initialize USB Audio/Video Class device
    ESP_LOGI(TAG, "Initializing USB UAC device...");
    if (my_uac_device_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize USB UAC device");
        return;
    }
    ESP_LOGI(TAG, "USB UAC device initialized successfully");

     ESP_LOGI(TAG, "Initializing USB UVC device...");
    if (my_uvc_device_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize USB UVC device");
        return;
    }
    ESP_LOGI(TAG, "USB UVC device initialized successfully");

    // Create task for microphone to capture audio and send to USB
    ESP_LOGI(TAG, "Creating microphone task...");
    xTaskCreate(microphone_task, "microphone_task", 4096, NULL, 5, NULL);

    // Create task for speaker to receive audio from USB and play
    ESP_LOGI(TAG, "Creating speaker task...");
    xTaskCreate(speaker_task, "speaker_task", 4096, NULL, 5, NULL);


    // Create task for video streaming from camera to USB
    ESP_LOGI(TAG, "Starting video livestream...");
    xTaskCreate(camera_task, "livestream_task", 8192, NULL, 5, NULL);     

    ESP_LOGI(TAG, "All components initialized. Earpod Camera is running...");
}