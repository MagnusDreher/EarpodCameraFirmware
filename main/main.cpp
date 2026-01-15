#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "audio/camera.h"
#include "audio/microphone.h"
#include "audio/speaker.h"
#include "audiodata_usb.h"
#include "videodata_usb.h"

static const char* TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Earpod Camera Firmware...");

    // Initialize camera
    ESP_LOGI(TAG, "Initializing camera...");
    if (camera_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize camera");
        return;
    }
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

    // Create task for microphone to capture audio and send to USB
    ESP_LOGI(TAG, "Creating microphone task...");
    xTaskCreate(microphone_task, "microphone_task", 4096, NULL, 5, NULL);

    // Create task for speaker to receive audio from USB and play
    ESP_LOGI(TAG, "Creating speaker task...");
    xTaskCreate(speaker_task, "speaker_task", 4096, NULL, 5, NULL);

    // Create task for USB communication handling
    ESP_LOGI(TAG, "Creating USB task...");
    xTaskCreate(usb_task, "usb_task", 8192, NULL, 6, NULL);

    // Create task for video streaming from camera to USB
    ESP_LOGI(TAG, "Starting video livestream...");
    xTaskCreate(livestream_ov2640, "livestream_task", 8192, NULL, 5, NULL);     

    ESP_LOGI(TAG, "All components initialized. Earpod Camera is running...");
}