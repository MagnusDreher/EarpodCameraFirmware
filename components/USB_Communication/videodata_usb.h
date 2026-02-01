#pragma once

#include "camera.h"
#include <string.h>
#include "esp_camera.h"
#include "esp_err.h"
#include "esp_log.h"
#include "tusb_config.h"
#include "usb_device_uvc.h"
#include "freertos/ringbuf.h"

static const char* TAG = "USB_Video";

extern RingbufHandle_t buf_handle_camera;

static esp_err_t camera_init(uvc_format_t format, int width, int height, int rate);

static void camera_stop_cb(void *ctx);

static esp_err_t camera_start_cb(uvc_format_t format, int width, int height, int rate, void *ctx);

static uvc_fb_t* camera_fb_get_cb(void *ctx);

static void camera_fb_return_cb(uvc_fb_t *fb, void *ctx);
// Ring buffer handle shared with videodata_usb.cpp

// Initialize TinyUSB + UVC device
esp_err_t my_uvc_device_init(void);



// Camera capture task that fills the ring buffer
void camera_task(void *arg);



