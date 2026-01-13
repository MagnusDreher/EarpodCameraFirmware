#include "videodata_usb.h"

static const char* TAG = "USB_Video";
RingbufHandle_t buf_handle_camera;

   
esp_err_t my_uvc_device_init(){
    //Initialiazing tiny usb
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,  // TinyUSB verwendet default
        .string_descriptor = NULL,
        .string_descriptor_count = 0,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    //create ring buffer for video data
    
    buf_handle_camera = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
    if (buf_handle_camera == NULL) {
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
        .output_cb = uvc_device_output_cb,           // Speaker-Daten vom Host
        .input_cb = uvc_device_input_cb,             // Microphone-Daten zum Host
        .set_mute_cb = uvc_device_set_mute_cb,       // Mute-Control
        .set_volume_cb = uvc_device_set_volume_cb,   // Volume-Control
        .cb_ctx = NULL,
    };
    
    err = uvc_device_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init UVC device: 0x%x", err);
        return err;
    }
    ESP_LOGI(TAG, "UVC Device initialized");

    err= camera_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init Camera: 0x%x", err);
        return err;
    }

}


esp_err_t uvc_device_output_cb(uint8_t *data, size_t len)
{
    uint8_t *rb_data;
    size_t rb_size;

    rb_data = (uint8_t *) xRingbufferReceiveUpTo(buf_handle_camera, &rb_size, 0, len);

    if(!rb_data) {
        memset(data, 0, len);
        return ESP_OK;
    }

    
    memcpy(data, rb_data, rb_size);

    if(rb_size < len)
        memset(data + rb_size, 0, len - rb_size);

    vRingbufferReturnItem(buf_handle_camera, rb_data);
    return ESP_OK;
}

void camera_task(void *arg)
{
    uint8_t *frame_buf;
    size_t frame_size;

    while(1) {
        
        esp_err_t err = livestream_ov2640(&frame_buf, &frame_size, portMAX_DELAY);
        if(err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to read camera frame");
            continue;
        }

       
        if(xRingbufferSend(buf_handle_camera, frame_buf, frame_size, 0) != pdTRUE) {
            ESP_LOGW(TAG, "Ringbuffer full, dropping frame");
        }
    }
}