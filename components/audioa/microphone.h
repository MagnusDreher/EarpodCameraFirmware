#pragma once

#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#define MIC_I2S_CLK  GPIO_NUM_38
#define MIC_I2S_LR   GPIO_NUM_35
#define MIC_I2S_DATA GPIO_NUM_45


esp_err_t ics43434_init();
esp_err_t readMicrophone(void* buf,size_t bytes_to_read,size_t* bytes_read,
    TickType_t ticks_to_wait);
esp_err_t ics43434_deinit();
