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
extern uint8_t *rb_data;
extern size_t rb_size;

// Initialize TinyUSB + UAC device
esp_err_t my_uac_device_init(void);

// Tasks for audio streaming
void microphone_task(void *arg);
void speaker_task(void *arg);
void usb_task(void *arg);

// Callbacks used by UAC stack
static esp_err_t uac_device_input_cb(uint8_t* data, size_t len, void *arg);
static esp_err_t uac_device_output_cb(uint8_t* data, size_t len, size_t *bytes_read, void *arg);
static void uac_device_set_mute_cb(uint32_t mute, void *arg);
static void uac_device_set_volume_cb(uint32_t volume, void *arg);