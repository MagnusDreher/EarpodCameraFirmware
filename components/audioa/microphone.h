#pragma once

#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char* TAG = "MICROPHONE";

static i2s_chan_handle_t rx_handle = NULL;

esp_err_t ics43434_init();
esp_err_t readMicrophone(void* buf,size_t bytes_to_read,size_t* bytes_read,
    TickType_t ticks_to_wait);
esp_err_t ics43434_deinit();
