#ifndef AUDIODATA_USB_H
#define AUDIODATA_USB_H

#include "stdint.h"
#include "stddef.h"
#include "freertos/ringbuf.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb_device_uac.h"
#include "esp_tinyusb.h"
#include "freertos/ringbuf.h"
#include "audio/microphone.h"
#include "audio/speaker.h"

#ifdef __cplusplus
extern "C" {
#endif

extern RingbufHandle_t buf_handle_microfon;
extern RingbufHandle_t buf_handle_audio;
extern uint8_t *rb_data;
extern size_t rb_size;

esp_err_t my_uac_device_init(void);
void microphone_task(void *arg);
void speaker_task(void *arg);
esp_err_t uac_device_input_cb(uint8_t* data, size_t len);
esp_err_t uac_device_output_cb(uint8_t* data, size_t len);
void usb_task(void *arg);

#ifdef __cplusplus
}
#endif

#endif // AUDIODATA_USB_H