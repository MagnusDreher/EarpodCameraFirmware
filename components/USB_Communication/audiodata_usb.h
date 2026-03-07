#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/ringbuf.h"
#include "esp_err.h"
#include "esp_log.h"
#include "tusb_config.h"
#include "usb_device_uac.h"
#include "microphone.h"
#include "speaker.h"



// Ring buffer handles shared with audiodata_usb.cpp
extern RingbufHandle_t buf_handle_microfon;
extern RingbufHandle_t buf_handle_audio;


// Initialize TinyUSB + UAC device
esp_err_t my_uac_device_init(void);

// Tasks for audio streaming
void microphone_task(void *arg);
void speaker_task(void *arg);
