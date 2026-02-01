#include "videodata_usb.h"


RingbufHandle_t buf_handle_camera;
static TaskHandle_t camera_task_handle = NULL;

static esp_err_t camera_init(uvc_format_t format, int width, int height, int rate)
{
    framesize_t framesize;
    // Map width and height to ESP framesize
    if (width == 1600 && height == 1200) {
        framesize = FRAMESIZE_UXGA;
    } else if (width == 1280 && height == 1024) {
        framesize = FRAMESIZE_SXGA;
    } else if (width == 1280 && height == 720) {
        framesize = FRAMESIZE_HD;
    } else if (width == 800 && height == 600) {
        framesize = FRAMESIZE_SVGA;
    } else if (width == 640 && height == 480) {
        framesize = FRAMESIZE_VGA;
    } else if (width == 320 && height == 240) {
        framesize = FRAMESIZE_QVGA;
    } else {
        ESP_LOGE(TAG, "Unsupported resolution: %dx%d", width, height);
        return ESP_ERR_NOT_SUPPORTED;
    }

    static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 36000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_UXGA,//QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY//CAMERA_GRAB_LATEST. Sets when buffers should be filled
    };

    if (CAM_PIN_PWDN != -1) {
    
    gpio_reset_pin((gpio_num_t)CAM_PIN_PWDN);
    gpio_set_direction((gpio_num_t)CAM_PIN_PWDN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)CAM_PIN_PWDN, 0);
    }

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
};

static esp_err_t camera_start_cb(uvc_format_t format, int width, int height, int rate, void *ctx)
{
     ESP_LOGI(TAG, "Camera Start");

     esp_err_t ret = camera_init(format, width, height, rate);
     if (ret != ESP_OK) {
         ESP_LOGE(TAG, "Camera initialization failed in start_cb");
         return ret;
     }
    return ESP_OK;
}

static void camera_stop_cb(void *ctx)
{
    ESP_LOGI(TAG, "Camera Stop");
    esp_camera_deinit();
    if (CAM_PIN_PWDN != -1) {
        gpio_set_level((gpio_num_t)CAM_PIN_PWDN, 1);
    }
}

static camera_fb_t *last_camera_fb = NULL;
static uvc_fb_t uvc_fb;

static uvc_fb_t* camera_fb_get_cb(void *ctx)
{
    size_t item_size;
    const TickType_t timeout = pdMS_TO_TICKS(100); // Wait up to 100ms for a frame
    void *item = xRingbufferReceive(buf_handle_camera, &item_size, timeout);

    if (!item || item_size != sizeof(camera_fb_t *)) {
        ESP_LOGE(TAG, "No frame available in ringbuffer");
        if (item) vRingbufferReturnItem(buf_handle_camera, item);
        return NULL;
    }

    camera_fb_t *fb = *(camera_fb_t **)item;
    vRingbufferReturnItem(buf_handle_camera, item);
    if (!fb) {
        ESP_LOGE(TAG, "Invalid frame buffer");
        return NULL;
    }

    last_camera_fb = fb;

    uvc_fb.buf = fb->buf;
    uvc_fb.len = fb->len;
    uvc_fb.width = fb->width;
    uvc_fb.height = fb->height;
    uvc_fb.format = UVC_FORMAT_JPEG;
    

    return &uvc_fb;
}
static void camera_fb_return_cb(uvc_fb_t *fb, void *ctx)
{
    if (!fb || !last_camera_fb) return;

    esp_camera_fb_return(last_camera_fb);
    last_camera_fb = NULL;
}

esp_err_t my_uvc_device_init(){
  
    
    buf_handle_camera = xRingbufferCreate(sizeof(camera_fb_t *)*4, RINGBUF_TYPE_NOSPLIT);
    if (buf_handle_camera == NULL) {
        printf("Failed to create ring buffer\n");
        return ESP_FAIL;
    }

    const size_t buff_size = 40 * 1024;
    uint8_t *uvc_buffer = (uint8_t *)heap_caps_malloc(buff_size, MALLOC_CAP_DEFAULT);
    if (uvc_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate UVC buffer");
        vRingbufferDelete(buf_handle_camera);
        return ESP_FAIL;
    }


    //Initialiazing UVC device
  uvc_device_config_t config = {
    .uvc_buffer = NULL,
    .uvc_buffer_size = 40 * 1024,
    .start_cb = camera_start_cb,
    .fb_get_cb = camera_fb_get_cb,
    .fb_return_cb = camera_fb_return_cb,
    .stop_cb = camera_stop_cb,
    .cb_ctx = NULL,

};

esp_err_t err =uvc_device_config(0,&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UVC device: 0x%x", err);
        free(uvc_buffer);
        vRingbufferDelete(buf_handle_camera);
        return err;
    }
 err = uvc_device_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init UVC device: 0x%x", err);
        free(uvc_buffer);
        vRingbufferDelete(buf_handle_camera);
        return err;
    }   
    ESP_LOGI(TAG, "UVC Device initialized");

    return ESP_OK;


}


/*esp_err_t uvc_device_output_cb(uint8_t *data, size_t len)
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
*/
void camera_task(void *arg)
{

    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGW(TAG, "Camera capture failed");
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // Frame in Ringbuffer schreiben
        camera_fb_t *fb_ptr = fb;
        if (xRingbufferSend(buf_handle_camera, &fb_ptr, sizeof(camera_fb_t *), pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGW(TAG, "Ringbuffer full, dropping video frame");
            esp_camera_fb_return(fb); // Return immediately if can't send
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // ~100fps Limit
    }
}