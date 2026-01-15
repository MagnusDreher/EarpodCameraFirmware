#include "speaker.h"

static const char* TAG = "SPEAKER";

static i2s_chan_handle_t tx_handle = NULL;

esp_err_t MAX98357_init(){
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO,I2S_ROLE_MASTER);
    esp_err_t err=i2s_new_channel(&chan_cfg,NULL,&tx_handle);
    if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2s_new_channel failed: 0x%x", err);
            return err;
    }
    i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(48000),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = GPIO_NUM_12,
        .ws = GPIO_NUM_13,
        .dout = GPIO_NUM_33,
        .din = I2S_GPIO_UNUSED,
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
        },
         },
    };

    err=i2s_channel_init_std_mode(tx_handle, &std_cfg);
if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_init_std_mode failed: 0x%x", err);
        return err;
}

 err=i2s_channel_enable(tx_handle);
if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_enable failed: 0x%x", err);
        return err;
}
    // Configurate GPIO-Pin 34 to start the Amplifier
gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_34), // Bitmaske für Pin 34
        .mode = GPIO_MODE_OUTPUT,              // Als Ausgang setzen
        .pull_up_en = GPIO_PULLUP_DISABLE,      // Kein Pull-up nötig
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // Kein Pull-down nötig
        .intr_type = GPIO_INTR_DISABLE          // Keine Interrupts für Ausgänge
    };
    
    
    esp_err_t ret = gpio_config(&io_conf);
    
    if (ret == ESP_OK) {
        
        gpio_set_level(GPIO_NUM_34, 1);
    }
return err;
}


esp_err_t sendAudiotoAmpflifeier(void* buf,size_t bytes_to_send,size_t* bytes_send,
    TickType_t ticks_to_wait){
        if (!tx_handle) return ESP_ERR_INVALID_STATE;
        return i2s_channel_write(tx_handle, buf, bytes_to_send, bytes_send, ticks_to_wait);
}

esp_err_t MAX98357_deinit(){
    if (tx_handle) {
        esp_err_t err = i2s_channel_disable(tx_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2s_channel_disable failed: 0x%x", err);
            return err;
        }
        err = i2s_del_channel(tx_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2s_del_channel failed: 0x%x", err);
            return err;
        }
        tx_handle = NULL;
    }
    esp_err_t ret = gpio_set_level(GPIO_NUM_34, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable amplifier GPIO: 0x%x", ret);
        return ret;
    }
    return ESP_OK;
}