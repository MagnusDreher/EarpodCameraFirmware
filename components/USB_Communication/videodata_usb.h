#pragma once

#include "camera.h"
#include <string.h>
#include "esp_camera.h"
#include "esp_err.h"
#include "esp_log.h"
#include "tusb_config.h"
#include "usb_device_uvc.h"
#include "freertos/ringbuf.h"



// Initialize TinyUSB + UVC device
esp_err_t my_uvc_device_init(void);

// Camera capture task that fills the ring buffer
void camera_task(void *arg);



