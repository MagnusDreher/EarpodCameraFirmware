#pragma once
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t MAX98357_init();
esp_err_t sendAudiotoAmpflifeier(void* buf,size_t bytes_to_send,size_t* bytes_send,
    TickType_t ticks_to_wait);
esp_err_t MAX98357_deinit();
