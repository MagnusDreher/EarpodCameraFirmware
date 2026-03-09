#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "usb_device_uac.h"
#include "microphone.h"
#include "speaker.h"
#include "freertos/ringbuf.h"

esp_err_t my_uac_device_init(void);
