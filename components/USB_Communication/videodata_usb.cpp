#include "videodata_usb.h"

// Expose NVS log save from main.cpp so camera events are captured
extern void nvs_log_save(void);

static const char *TAG = "USB_Video";

static TaskHandle_t camera_task_handle = NULL;
static RingbufHandle_t buf_handle_camera = NULL;
static volatile bool camera_ready = false;
static camera_fb_t *last_camera_fb = NULL;

// Semaphore + params for non-blocking camera_start_cb
static SemaphoreHandle_t s_camera_start_sem = NULL;
static SemaphoreHandle_t s_camera_stop_sem  = NULL;
static uvc_format_t s_pending_format;
static int s_pending_width, s_pending_height, s_pending_rate;
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

    camera_config_t camera_config = {
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

    .xclk_freq_hz = 20000000, // OV2640 datasheet: typ 24 MHz max — 20 MHz is safe
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = framesize,

    .jpeg_quality = 12,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
    };

    if (CAM_PIN_PWDN != -1) {
        gpio_reset_pin((gpio_num_t)CAM_PIN_PWDN);
        gpio_set_direction((gpio_num_t)CAM_PIN_PWDN, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)CAM_PIN_PWDN, 0);
        vTaskDelay(pdMS_TO_TICKS(10)); // wait for sensor to come out of power-down
    }

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
};

// Called from TinyUSB task — must return immediately to not block USB processing
static esp_err_t camera_start_cb(uvc_format_t format, int width, int height, int rate, void *ctx)
{
    ESP_LOGI(TAG, "Camera Start: %dx%d@%dfps", width, height, rate);
    s_pending_format = format;
    s_pending_width  = width;
    s_pending_height = height;
    s_pending_rate   = rate;
    xSemaphoreGive(s_camera_start_sem);  // signal camera_task to initialize
    return ESP_OK;
}

// Called from TinyUSB task — signal camera_task and return immediately
static void camera_stop_cb(void *ctx)
{
    ESP_LOGI(TAG, "Camera Stop");
    camera_ready = false;
    xSemaphoreGive(s_camera_stop_sem);
}

static void camera_stop_cleanup(void)
{
    vTaskDelay(pdMS_TO_TICKS(60)); // let camera_task exit esp_camera_fb_get safely

    // Drain ring buffer — frame pointers become invalid after deinit
    size_t item_size;
    void *item;
    while ((item = xRingbufferReceive(buf_handle_camera, &item_size, 0)) != NULL) {
        if (item_size == sizeof(camera_fb_t *)) {
            camera_fb_t *fb = *(camera_fb_t **)item;
            if (fb) esp_camera_fb_return(fb);
        }
        vRingbufferReturnItem(buf_handle_camera, item);
    }
    last_camera_fb = NULL;

    esp_camera_deinit();
    if (CAM_PIN_PWDN != -1) {
        gpio_set_level((gpio_num_t)CAM_PIN_PWDN, 1);
    }
}

static uvc_fb_t uvc_fb;

static uvc_fb_t* camera_fb_get_cb(void *ctx)
{
    size_t item_size;
    const TickType_t timeout = pdMS_TO_TICKS(100); // Wait up to 100ms for a frame
    void *item = xRingbufferReceive(buf_handle_camera, &item_size, timeout);

    if (!item || item_size != sizeof(camera_fb_t *)) {
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

    s_camera_start_sem = xSemaphoreCreateBinary();
    s_camera_stop_sem  = xSemaphoreCreateBinary();
    if (!s_camera_start_sem || !s_camera_stop_sem) {
        ESP_LOGE(TAG, "Failed to create semaphores");
        return ESP_FAIL;
    }

    // NOSPLIT ring buffer needs 8-byte header + data per item, aligned to 4 bytes.
    // Per item: 8 (header) + 4 (pointer) = 12 bytes. For 4 items: 48 bytes minimum.
    // Use 256 bytes to have a safe margin.
    buf_handle_camera = xRingbufferCreate(256, RINGBUF_TYPE_NOSPLIT);
    if (buf_handle_camera == NULL) {
        printf("Failed to create ring buffer\n");
        return ESP_FAIL;
    }

    const size_t buff_size = 64 * 1024;
    uint8_t *uvc_buffer = (uint8_t *)heap_caps_malloc(buff_size, MALLOC_CAP_DEFAULT);
    if (uvc_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate UVC buffer");
        vRingbufferDelete(buf_handle_camera);
        return ESP_FAIL;
    }


    //Initialiazing UVC device
  uvc_device_config_t config = {
    .uvc_buffer = uvc_buffer,
    .uvc_buffer_size = buff_size,
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
        // Check if camera_stop_cb signalled cleanup
        if (xSemaphoreTake(s_camera_stop_sem, 0) == pdTRUE) {
            camera_stop_cleanup();
            continue;
        }

        // Check if camera_start_cb signalled init
        if (xSemaphoreTake(s_camera_start_sem, pdMS_TO_TICKS(50)) == pdTRUE) {
            // Do the slow init + warmup here, NOT in the TinyUSB callback
            esp_err_t ret = camera_init(s_pending_format, s_pending_width, s_pending_height, s_pending_rate);

            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Camera init failed: 0x%x", ret);
                nvs_log_save(); // persist failure for next-boot inspection
                continue;
            }
            ESP_LOGI(TAG, "Camera init OK");
            nvs_log_save(); // persist success + camera_start params for next-boot inspection

            // Warmup: discard frames while auto-exposure stabilizes
            for (int i = 0; i < 5; i++) {
                camera_fb_t *warmup = esp_camera_fb_get();
                if (warmup) esp_camera_fb_return(warmup);
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            // Pre-fill ring buffer so USB video task has a frame ready immediately
            camera_fb_t *first = esp_camera_fb_get();
            if (first) {
                camera_fb_t *fb_ptr = first;
                if (xRingbufferSend(buf_handle_camera, &fb_ptr, sizeof(camera_fb_t *), pdMS_TO_TICKS(200)) != pdTRUE) {
                    esp_camera_fb_return(first);
                }
            }

            camera_ready = true;
            continue;
        }

        if (!camera_ready) {
            continue;
        }

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        camera_fb_t *fb_ptr = fb;
        if (xRingbufferSend(buf_handle_camera, &fb_ptr, sizeof(camera_fb_t *), pdMS_TO_TICKS(10)) != pdTRUE) {
            ESP_LOGW(TAG, "Ringbuffer full, dropping frame");
            esp_camera_fb_return(fb);
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}