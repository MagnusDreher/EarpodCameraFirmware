#include "microphone.h"

static const char* TAG = "MICROPHONE";
static i2s_chan_handle_t rx_handle = NULL;

esp_err_t ics43434_init(){
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num  = 8;  
    chan_cfg.dma_frame_num = 240; 
    esp_err_t err=i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2s_new_channel failed: 0x%x", err);
        return err;
    }
    i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CONFIG_UAC_SAMPLE_RATE),
      
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = MIC_I2S_CLK,
            .ws   = MIC_I2S_LR,
            .dout = I2S_GPIO_UNUSED,
            .din  = MIC_I2S_DATA,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        
    },
    };

    std_cfg.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT;
    std_cfg.slot_cfg.ws_width       = 32;

    err=i2s_channel_init_std_mode(rx_handle, &std_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2s_channel_init_std_mode failed: 0x%x", err);
            return err;
        }

     err=i2s_channel_enable(rx_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2s_channel_enable failed: 0x%x", err);
            return err;
        }

    gpio_pulldown_en(MIC_I2S_DATA);

    return err;
    }


esp_err_t readMicrophone(void* buf,size_t bytes_to_read,size_t* bytes_read,
    TickType_t ticks_to_wait){
        if (!rx_handle) return ESP_ERR_INVALID_STATE;
        return i2s_channel_read(rx_handle, buf, bytes_to_read, bytes_read, ticks_to_wait);
}

esp_err_t ics43434_deinit(){
    if (rx_handle) {
        esp_err_t err = i2s_channel_disable(rx_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2s_channel_disable failed: 0x%x", err);
            return err;
        }
        err = i2s_del_channel(rx_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "i2s_del_channel failed: 0x%x", err);
            return err;
        }
        rx_handle = NULL;
    }
    return ESP_OK;
}