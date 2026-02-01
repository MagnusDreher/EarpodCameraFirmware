#pragma once

#include "esp_camera.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define CAM_PIN_PWDN    17//power down is not used
#define CAM_PIN_RESET   14 //software reset will be performed
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    6
#define CAM_PIN_SIOC    7

//Y0 and Y1 doesn't initialize
#define CAM_PIN_D7      12
#define CAM_PIN_D6      11
#define CAM_PIN_D5      10
#define CAM_PIN_D4      9
#define CAM_PIN_D3      5
#define CAM_PIN_D2      3
#define CAM_PIN_D1      2
#define CAM_PIN_D0      4
#define CAM_PIN_VSYNC   18
#define CAM_PIN_HREF    13
#define CAM_PIN_PCLK    8

static const char* TAG = "Camera";
esp_err_t camera_init();

esp_err_t livestream_ov2640(bool camera_on);