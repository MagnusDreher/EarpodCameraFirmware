#include "audiodata_usb.h"

static const char* TAG = "USB_Audio";
static RingbufHandle_t buf_handle_microfon = NULL;
static RingbufHandle_t buf_handle_audio = NULL;

   
esp_err_t my_uac_device_init()
{
    //Initialiazing tiny usb
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,  // TinyUSB verwendet default
        .string_descriptor = NULL,
        .string_descriptor_count = 0,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    //create ring buffer for audio data
    buf_handle_microfon = xRingbufferCreate(8192, RINGBUF_TYPE_BYTEBUF);
    if (buf_handle_microfon == NULL) {
        printf("Failed to create ring buffer\n");
    }
    //create ring buffer for audio data
    buf_handle_audio = xRingbufferCreate(8192, RINGBUF_TYPE_BYTEBUF);
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

    return ESP_OK;
}

void microphone_task(void *arg)
{
    uint8_t i2s_buf[512];
    size_t bytes_read;

    while (1) {
        esp_err_t err = readMicrophone(
            i2s_buf,
            sizeof(i2s_buf),
            &bytes_read,
            portMAX_DELAY
        );

        if (err == ESP_OK && bytes_read > 0) {
            if (xRingbufferSend(
                    buf_handle_microfon,
                    i2s_buf,
                    bytes_read,
                    portMAX_DELAY
                ) != pdTRUE) {
                ESP_LOGW(TAG, "Ringbuffer full, dropping audio");
            }
        }
    }
}
void speaker_task(void *arg)
{
    uint8_t *rb_data;
    size_t rb_size;
    size_t bytes_sent;

    while (1) {
        rb_data = (uint8_t *) xRingbufferReceiveUpTo(
            buf_handle_audio,
            &rb_size,
            portMAX_DELAY,   
            512              
        );

        if (!rb_data) {
            continue;
        }

        esp_err_t err = sendAudioToAmplifier(
            rb_data,
            rb_size,
            &bytes_sent,
            portMAX_DELAY
        );

        if (err != ESP_OK || bytes_sent != rb_size) {
            ESP_LOGW(TAG, "Speaker underrun or error");
        }

        vRingbufferReturnItem(buf_handle_audio, rb_data);
    }
}

esp_err_t uac_device_input_cb(uint8_t* data, size_t len) 
{
    uint8_t *rb_data;
    size_t rb_size;
    rb_data = (uint8_t *) xRingbufferReceiveUpTo(
        buf_handle_microfon,
        &rb_size,
        0,      
        len
    );

    if (!rb_data) {
        memset(data, 0, len);   // Silence
        return ESP_OK;
    }

    memcpy(data, rb_data, rb_size);

    if (rb_size < len) {
        memset(data + rb_size, 0, len - rb_size);
    }

    vRingbufferReturnItem(buf_handle_microfon, rb_data);
    return ESP_OK;
}

esp_err_t uac_device_output_cb(uint8_t* data, size_t len) 
{

    BaseType_t res = xRingbufferSend(
        buf_handle_audio,
        data,
        len,
        0       
    );

    if (res != pdTRUE) {
        // Audio-Drop ist besser als USB-Stall
        ESP_LOGW(TAG, "Audio ringbuffer full, dropping %d bytes", len);
    }

    return ESP_OK;
}

void usb_task(void *arg)
{
    while (1) {
        tud_task();   // TinyUSB Scheduler
        vTaskDelay(1);
    }
}

