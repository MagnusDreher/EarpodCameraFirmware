#include "audiodata_usb.h"
#include "tusb_config.h"
#include "usb_descriptors.h"
#include <math.h>

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


#define MIC_GAIN 1


static int32_t s_hp_x_prev = 0;
static int32_t s_hp_y_prev = 0;

static inline int32_t mic_highpass(int32_t x)
{
    int32_t y = x - s_hp_x_prev + (s_hp_y_prev * 255) / 256;
    s_hp_x_prev = x;
    s_hp_y_prev = y;
    return y;
}


#define NOISE_GATE_OPEN  2000  
#define NOISE_GATE_HOLD  50    
static bool     s_gate_open = false;
static uint32_t s_gate_hold = 0;


static int32_t s_mic_i2s_buf[960]; 

// Called by UAC component when host wants microphone audio
static esp_err_t uac_device_input_cb(uint8_t *buf, size_t len, size_t *bytes_read, void *arg)
{
    
    size_t i2s_len = len * 4;
    if (i2s_len > sizeof(s_mic_i2s_buf)) i2s_len = sizeof(s_mic_i2s_buf);

    size_t i2s_read = 0;
    esp_err_t err = readMicrophone(s_mic_i2s_buf, i2s_len, &i2s_read, portMAX_DELAY);
    if (err != ESP_OK) return err;

    
    int16_t *out = (int16_t *)buf;
    size_t n_stereo = i2s_read / 4;   
    size_t n_out    = n_stereo / 2;   

  
    uint64_t sum_sq = 0;
    for (size_t i = 0; i < n_out; i++) {
        int32_t s = mic_highpass((s_mic_i2s_buf[i * 2] >> 8) * MIC_GAIN);
        if (s >  32767) s =  32767;
        if (s < -32768) s = -32768;
        out[i] = (int16_t)s;
        sum_sq += (uint64_t)((int64_t)s * s);
    }
    int32_t rms = (n_out > 0) ? (int32_t)sqrtf((float)sum_sq / (float)n_out) : 0;

   
    if (rms >= NOISE_GATE_OPEN) {
        s_gate_open = true;
        s_gate_hold = NOISE_GATE_HOLD;
    } else if (s_gate_hold > 0) {
        s_gate_hold--;
    } else {
        s_gate_open = false;
    }

    if (!s_gate_open) memset(out, 0, n_out * 2);

    *bytes_read = n_out * 2;
    return ESP_OK;
}

// Called by UAC component when host sends speaker audio
// 80% = -2 dBFS headroom. Raise if too quiet, lower if still distorting.
#define SPK_MAX_LEVEL 80

static esp_err_t uac_device_output_cb(uint8_t *buf, size_t len, void *arg)
{
    int16_t *samples = (int16_t *)buf;
    size_t sample_count = len / 2;

    if (s_is_muted) {
        memset(buf, 0, len);
    } else {
        
        uint32_t factor = (uint32_t)s_volume_factor * SPK_MAX_LEVEL / 100;
        for (size_t i = 0; i < sample_count; i++) {
            int32_t s = (int32_t)samples[i] * (int32_t)factor / 100;
            if (s >  32767) s =  32767;
            if (s < -32768) s = -32768;
            samples[i] = (int16_t)s;
        }
    }

    size_t total = 0;
    while (total < len) {
        size_t written = 0;
        esp_err_t err = sendAudiotoAmpflifeier((uint8_t*)buf + total, len - total, &written, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S write failed: 0x%x written=%d len=%d", err, (int)written, (int)len);
            return err;
        }
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
        
        return err;
    }
    
    ESP_LOGI(TAG, "UAC Device initialized");
    return ESP_OK;
}
