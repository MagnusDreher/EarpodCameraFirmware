#pragma once
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define SPEAKER_I2S_DOUT  GPIO_NUM_33
#define SPEAKER_I2S_BCLK  GPIO_NUM_36
#define SPEAKER_I2S_LRC   GPIO_NUM_37
#define SPEAKER_SD_MODE   GPIO_NUM_34

esp_err_t MAX98357_init();
esp_err_t sendAudiotoAmpflifeier(void* buf,size_t bytes_to_send,size_t* bytes_send,
    TickType_t ticks_to_wait);
esp_err_t MAX98357_deinit();
