#include "audiodata_usb.h"
#include "usb_descriptors.h"
#include <math.h>

extern void nvs_log_save(void);
static const char* TAG = "USB_Audio";

static bool     s_is_muted     = false;
static uint32_t s_volume_factor = 100; // percentage (100 = unity gain)

static void uac_device_set_mute_cb(uint32_t mute, void *arg)
{
    s_is_muted = (mute != 0);
    ESP_LOGI(TAG, "Mute: %lu", mute);
}

static void uac_device_set_volume_cb(uint32_t _volume, void *arg)
{
    // _volume = (volume_db + 50) * 2  (see usb_device_uac.c)
    int volume_db = (int)(_volume / 2) - 50;
    s_volume_factor = (uint32_t)(powf(10.0f, volume_db / 20.0f) * 100.0f);
    ESP_LOGI(TAG, "Volume: %lu dB=%d factor=%lu", _volume, volume_db, s_volume_factor);
}

// Called by UAC component when host wants microphone audio
static esp_err_t uac_device_input_cb(uint8_t *buf, size_t len, size_t *bytes_read, void *arg)
{
    return readMicrophone(buf, len, bytes_read, portMAX_DELAY);
}

// Called by UAC component when host sends speaker audio
static esp_err_t uac_device_output_cb(uint8_t *buf, size_t len, void *arg)
{
    int16_t *samples = (int16_t *)buf;
    size_t sample_count = len / 2;

    if (s_is_muted) {
        memset(buf, 0, len);
    } else if (s_volume_factor != 100) {
        for (size_t i = 0; i < sample_count; i++) {
            int32_t s = (int32_t)samples[i] * (int32_t)s_volume_factor / 100;
            if (s >  32767) s =  32767;
            if (s < -32768) s = -32768;
            samples[i] = (int16_t)s;
        }
    }

    size_t total = 0;
    while (total < len) {
        size_t written = 0;
        esp_err_t err = sendAudiotoAmpflifeier((uint8_t*)buf + total, len - total, &written, portMAX_DELAY);
        if (err != ESP_OK) return err;
        total += written;
    }
    return ESP_OK;
}

esp_err_t my_uac_device_init(void)
{
    uac_device_config_t config = {
        .skip_tinyusb_init = true,
        .output_cb      = uac_device_output_cb,
        .input_cb       = uac_device_input_cb,
        .set_mute_cb    = uac_device_set_mute_cb,
        .set_volume_cb  = uac_device_set_volume_cb,
        .cb_ctx         = NULL,
        .spk_itf_num    = ITF_NUM_AUDIO_STREAMING_SPK,
        .mic_itf_num    = ITF_NUM_AUDIO_STREAMING_MIC,
    };

    esp_err_t err = uac_device_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init UAC device: 0x%x", err);
        nvs_log_save();
        return err;
    }
    nvs_log_save();
    ESP_LOGI(TAG, "UAC Device initialized");
    return ESP_OK;
}
