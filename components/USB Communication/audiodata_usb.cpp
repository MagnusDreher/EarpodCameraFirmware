#include "microphone.h"
#include "speaker.h"
#include "camera.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb_device_uac.h"
#include "esp_tinyusb.h"
#include "freertos/ringbuf.h"

static const char* TAG = "USB_Audio";


   
esp_err_t my_uac_device_init(){
    //Initialiazing tiny usb
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,  // TinyUSB verwendet default
        .string_descriptor = NULL,
        .string_descriptor_count = 0,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    //create ring buffer for audio data
    RingbufHandle_t buf_handle_microfon;
    buf_handle_microfon = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
    if (buf_handle_microfon == NULL) {
        printf("Failed to create ring buffer\n");
    }
    //create ring buffer for audio data
    RingbufHandle_t buf_handle_audio;
    buf_handle_audio = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
    if (buf_handle_audio == NULL) {
        printf("Failed to create ring buffer\n");
    }

    esp_err_t err = esp_tinyusb_init(&tusb_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize TinyUSB: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "TinyUSB initialized successfully");
    //Initialiazing UAC device
    uac_device_config_t config = {
        .output_cb = uac_device_output_cb,           // Speaker-Daten vom Host
        .input_cb = uac_device_input_cb,             // Microphone-Daten zum Host
        .set_mute_cb = uac_device_set_mute_cb,       // Mute-Control
        .set_volume_cb = uac_device_set_volume_cb,   // Volume-Control
        .cb_ctx = NULL,
    };
    
    err = uac_device_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init UAC device: 0x%x", err);
        return err;
    }
    ESP_LOGI(TAG, "UAC Device initialized");

    err= ics43434_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init Microphone: 0x%x", err);
        return err;
    }

    err=MAX98357_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init Speaker: 0x%x", err);
        return err;
        }
    return ESP_OK;
}


esp_err_t uac_device_input_cb(const uint8_t* data, size_t len) 
{
    size_t bytes_read = 0;
    esp_err_t err = readMicrophone(data, len, &bytes_read, portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from microphone: 0x%x", err);
        return err;
    }
    if (bytes_read < len) {
        ESP_LOGE(TAG, "Read less data from microphone than requested: %d < %d", bytes_read, len);
    }
    UBaseType_t res =  xRingbufferSend(buf_handle_microfon, data, len, pdMS_TO_TICKS(1000));
    if (res != pdTRUE) {
        ESP_LOGE(TAG, "Failed to send item from ring buffer");
    }
    return ESP_OK;
}
esp_err_t uac_device_output_cb(uint8_t* data, size_t len) 
{

    size_t bytes_written = 0;
    esp_err_t err = sendAudioToAmpflifeier((void*)data, len, &bytes_written, portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write to speaker: 0x%x", err);
        return err;
    }
    if (bytes_written < len) {
        ESP_LOGW(TAG, "Wrote less data to speaker than requested: %d < %d", bytes_written, len);
    }
    
    UBaseType_t res =  xRingbufferReceive(buf_handle_audio, data, len, pdMS_TO_TICKS(1000));
    if (res != pdTRUE) {
        ESP_LOGE(TAG, "Failed to receive item from ring buffer");
    }
    return ESP_OK;
}


