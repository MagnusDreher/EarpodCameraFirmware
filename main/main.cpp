#include <stdio.h>
//#include "esp_tinyusb.h"
#include <string.h>
#include <stdarg.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "microphone.h"
#include "speaker.h"
#include "audiodata_usb.h"
#include "videodata_usb.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_rom_sys.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"


static const char* TAG = "APP_MAIN";

// ---- NVS-Logging -------------------------------------------------------
#define NVS_LOG_NS  "diag"
#define NVS_LOG_KEY "boot_log"
#define LOG_BUF_SZ  3000

static char s_log_buf[LOG_BUF_SZ];
static int  s_log_len = 0;

static int log_capture_vprintf(const char *fmt, va_list args)
{
    int rem = LOG_BUF_SZ - s_log_len - 1;
    if (rem > 0) {
        va_list copy;
        va_copy(copy, args);
        int n = vsnprintf(s_log_buf + s_log_len, rem, fmt, copy);
        va_end(copy);
        if (n > 0) s_log_len += (n < rem ? n : rem - 1);
    }
    return vprintf(fmt, args);
}

static void nvs_log_read_prev(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_LOG_NS, NVS_READONLY, &h) != ESP_OK) return;
    size_t len = 0;
    if (nvs_get_blob(h, NVS_LOG_KEY, NULL, &len) == ESP_OK && len > 0) {
        char *p = (char*)malloc(len + 1);
        if (p) {
            nvs_get_blob(h, NVS_LOG_KEY, p, &len);
            p[len] = '\0';
            printf("\n=== LOG LETZTER BOOT ===\n%s=== ENDE LOG ===\n\n", p);
            free(p);
        }
    }
    nvs_close(h);
}

void nvs_log_save(void)
{
    if (s_log_len == 0) return;
    nvs_handle_t h;
    if (nvs_open(NVS_LOG_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, NVS_LOG_KEY, s_log_buf, s_log_len);
    nvs_commit(h);
    nvs_close(h);
}
// ------------------------------------------------------------------------
#define BOOT_LIMIT 3

void enter_download_mode(void)
{
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    esp_rom_delay_us(1000);
    esp_restart();
}

void check_bootloader_fallback_nvs(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open("system", NVS_READWRITE, &handle);
    if (err != ESP_OK) return;

    int32_t count = 0;
    nvs_get_i32(handle, "boot_cnt", &count);

    count++;
    nvs_set_i32(handle, "boot_cnt", count);
    nvs_commit(handle);
    nvs_close(handle);

    if (count >= BOOT_LIMIT) {
        // Counter löschen
        nvs_open("system", NVS_READWRITE, &handle);
        nvs_erase_key(handle, "boot_cnt");
        nvs_commit(handle);
        nvs_close(handle);

        enter_download_mode();
    }
}

void clear_boot_counter(void)
{
    nvs_handle_t handle;
    if (nvs_open("system", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_erase_key(handle, "boot_cnt");
        nvs_commit(handle);
        nvs_close(handle);  
    }
}


extern "C" void app_main(void)
{
    // Drive PWDN HIGH immediately — keeps OV2640 in power-down until camera_init()
    // Without this, PWDN floats (no internal pull resistor per datasheet) and the
    // sensor draws full current with no XCLK, causing it to overheat on plug-in.
    gpio_reset_pin((gpio_num_t)CAM_PIN_PWDN);
    gpio_set_direction((gpio_num_t)CAM_PIN_PWDN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)CAM_PIN_PWDN, 1);


    nvs_flash_init();

    nvs_log_read_prev();
    esp_log_set_vprintf(log_capture_vprintf);

    check_bootloader_fallback_nvs();

    ESP_LOGI(TAG, "Starting Earpod Camera Firmware...");

     vTaskDelay(pdMS_TO_TICKS(5000));

    // Initialize audio input (microphone)
    ESP_LOGI(TAG, "Initializing microphone...");
    if (ics43434_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize microphone");
        return;
    }
    ESP_LOGI(TAG, "Microphone initialized successfully");

    // Initialize audio output (speaker)
    ESP_LOGI(TAG, "Initializing speaker...");
    if (MAX98357_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize speaker");
        return;
    }
    ESP_LOGI(TAG, "Speaker initialized successfully");


   
    
    // UVC zuerst (installiert TinyUSB), dann UAC (registriert sich dazu)
    ESP_LOGI(TAG, "Initializing USB UVC device...");
    if (my_uvc_device_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize USB UVC device");
        return;
    }
    ESP_LOGI(TAG, "USB UVC device initialized successfully");

    ESP_LOGI(TAG, "Initializing USB UAC device...");
    if (my_uac_device_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize USB UAC device");
        return;
    }
    ESP_LOGI(TAG, "USB UAC device initialized successfully");

    // Create task for video streaming from camera to USB
    ESP_LOGI(TAG, "Starting video livestream...");
    xTaskCreate(camera_task, "livestream_task", 8192, NULL, 5, NULL);     

    ESP_LOGI(TAG, "All components initialized. Earpod Camera is running...");
    nvs_log_save();
    clear_boot_counter();
}