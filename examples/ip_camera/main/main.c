/**
   Copyright 2024 Achim Pieters | StudioPieters®

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   for more information visit https://www.studiopieters.nl
 **/

#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_camera.h>
#include <esp_h264.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <homekit/tlv.h>  // Added for TLV support
#include <lwip/sockets.h>

// Custom error handling macro
#define CHECK_ERROR(x) do {                        \
                esp_err_t __err_rc = (x);                  \
                if (__err_rc != ESP_OK) {                  \
                        ESP_LOGE("INFORMATION", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);                \
                }                                          \
} while(0)

void handle_error(esp_err_t err) {
        if (err == ESP_ERR_WIFI_NOT_STARTED || err == ESP_ERR_WIFI_CONN) {
                ESP_LOGI("INFORMATION", "Restarting WiFi...");
                esp_wifi_stop();
                esp_wifi_start();
        } else {
                ESP_LOGE("ERROR", "Critical error, restarting device...");
                esp_restart();
        }
}

void on_wifi_ready();

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
        if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
                ESP_LOGI("INFORMATION", "Connecting to WiFi...");
                esp_wifi_connect();
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                ESP_LOGI("INFORMATION", "WiFi connected, IP obtained");
                on_wifi_ready();
        }
}

static void wifi_init() {
        CHECK_ERROR(esp_netif_init());
        CHECK_ERROR(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        CHECK_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
        CHECK_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        CHECK_ERROR(esp_wifi_init(&wifi_init_config));
        CHECK_ERROR(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        wifi_config_t wifi_config = {
                .sta = {
                        .ssid = CONFIG_ESP_WIFI_SSID,
                        .password = CONFIG_ESP_WIFI_PASSWORD,
                        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                },
        };

        CHECK_ERROR(esp_wifi_set_mode(WIFI_MODE_STA));
        CHECK_ERROR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        CHECK_ERROR(esp_wifi_start());
}

// LED control
#define LED_GPIO CONFIG_ESP_LED_GPIO
bool led_on = false;

void led_write(bool on) {
        gpio_set_level(LED_GPIO, on ? 1 : 0);
}

// Camera configuration
#define CAMERA_PIN_PWDN    -1
#define CAMERA_PIN_RESET   -1
#define CAMERA_PIN_XCLK    0
#define CAMERA_PIN_SIOD    26
#define CAMERA_PIN_SIOC    27
#define CAMERA_PIN_D7      35
#define CAMERA_PIN_D6      34
#define CAMERA_PIN_D5      39
#define CAMERA_PIN_D4      36
#define CAMERA_PIN_D3      21
#define CAMERA_PIN_D2      19
#define CAMERA_PIN_D1      18
#define CAMERA_PIN_D0      5
#define CAMERA_PIN_VSYNC   25
#define CAMERA_PIN_HREF    23
#define CAMERA_PIN_PCLK    22

void camera_init() {
        camera_config_t config = {
                .pin_pwdn = CAMERA_PIN_PWDN,
                .pin_reset = CAMERA_PIN_RESET,
                .pin_xclk = CAMERA_PIN_XCLK,
                .pin_sccb_sda = CAMERA_PIN_SIOD,
                .pin_sccb_scl = CAMERA_PIN_SIOC,
                .pin_d7 = CAMERA_PIN_D7,
                .pin_d6 = CAMERA_PIN_D6,
                .pin_d5 = CAMERA_PIN_D5,
                .pin_d4 = CAMERA_PIN_D4,
                .pin_d3 = CAMERA_PIN_D3,
                .pin_d2 = CAMERA_PIN_D2,
                .pin_d1 = CAMERA_PIN_D1,
                .pin_d0 = CAMERA_PIN_D0,
                .pin_vsync = CAMERA_PIN_VSYNC,
                .pin_href = CAMERA_PIN_HREF,
                .pin_pclk = CAMERA_PIN_PCLK,
                .xclk_freq_hz = 20000000,
                .pixel_format = PIXFORMAT_JPEG,
                .frame_size = FRAMESIZE_UXGA,
                .jpeg_quality = 10,
                .fb_count = 2
        };

        if (esp_camera_init(&config) != ESP_OK) {
                ESP_LOGE("CAMERA", "Camera init failed");
                esp_restart();
        }

        ESP_LOGI("CAMERA", "Initializing H.264 encoder...");
        h264_encoder_config_t h264_config = {
                .width = 1600,
                .height = 1200,
                .bitrate = 1024 * 1024,
                .framerate = 15,
        };

        if (esp_h264_init(&h264_config) != ESP_OK) {
                ESP_LOGE("H264", "H.264 init failed");
                esp_restart();
        }
}

// RTP and HomeKit Streaming Characteristics
static const uint8_t video_config_tlv[] = {
        0x01, 0x01, 0x00, // Video Codec: H.264
        0x02, 0x02, 0x00, 0x02, // Profile: baseline, Level: 3.1
        0x03, 0x0C, // Video Attributes
        0x01, 0x40, 0x02, 0xF0, // Resolution: 640x480
        0x03, 0x1E, // Framerate: 30 fps
        0x05, 0x00 // End
};

static const uint8_t audio_config_tlv[] = {
        0x01, 0x01, 0x01, // Audio Codec: AAC-ELD
        0x02, 0x02, 0x01, 0x00, // Mono, Sample rate: 16kHz
        0x03, 0x02, 0x40, 0x00 // Bitrate: 64kbps
};

static const uint8_t rtp_config_tlv[] = {
        0x01, 0x01, 0x01, // RTP Payload Type: H.264
        0x02, 0x02, 0x00, 0x60 // Packetization mode: 1, MTU: 1500
};

homekit_characteristic_t streaming_status = HOMEKIT_CHARACTERISTIC_(STREAMING_STATUS, "\x00");
homekit_characteristic_t supported_video_stream_configuration = HOMEKIT_CHARACTERISTIC_(SUPPORTED_VIDEO_STREAM_CONFIGURATION, video_config_tlv);
homekit_characteristic_t supported_audio_stream_configuration = HOMEKIT_CHARACTERISTIC_(SUPPORTED_AUDIO_STREAM_CONFIGURATION, audio_config_tlv);
homekit_characteristic_t supported_rtp_configuration = HOMEKIT_CHARACTERISTIC_(SUPPORTED_RTP_CONFIGURATION, rtp_config_tlv);
homekit_characteristic_t setup_endpoints = HOMEKIT_CHARACTERISTIC_(SETUP_ENDPOINTS, .setter=setup_endpoints_handler);
homekit_characteristic_t selected_rtp_stream_configuration = HOMEKIT_CHARACTERISTIC_(SELECTED_RTP_STREAM_CONFIGURATION, "");

// Dynamic Endpoint Configuration
void setup_endpoints_handler(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
        ESP_LOGI("SETUP_ENDPOINTS", "Handling Setup Endpoints request...");

        tlv_values_t *request_tlv = tlv_new();
        if (!request_tlv) {
                ESP_LOGE("SETUP_ENDPOINTS", "Failed to allocate memory for TLV");
                return;
        }

        if (tlv_parse(value.string_value, value.string_len, request_tlv) != 0) {
                ESP_LOGE("SETUP_ENDPOINTS", "Failed to parse TLV data");
                tlv_free(request_tlv);
                return;
        }

        // Generate dynamic endpoint response
        tlv_values_t *response_tlv = tlv_new();
        if (!response_tlv) {
                ESP_LOGE("SETUP_ENDPOINTS", "Failed to allocate memory for response TLV");
                tlv_free(request_tlv);
                return;
        }

        tlv_add_string_value(response_tlv, TLV_TYPE_ADDRESS, "192.168.1.100");
        tlv_add_int_value(response_tlv, TLV_TYPE_VIDEO_RTP_PORT, 2, htons(12345));

        char *response_buf = NULL;
        size_t response_len = 0;
        if (tlv_format(response_tlv, &response_buf, &response_len) != 0) {
                ESP_LOGE("SETUP_ENDPOINTS", "Failed to format response TLV");
                tlv_free(request_tlv);
                tlv_free(response_tlv);
                return;
        }

        homekit_value_destruct(&setup_endpoints.value);
        setup_endpoints.value = HOMEKIT_STRING_N(response_buf, response_len);

        tlv_free(request_tlv);
        tlv_free(response_tlv);
        free(response_buf);
}

// Streaming Task
void video_streaming_task(void *args) {
        while (true) {
                camera_fb_t *frame = esp_camera_fb_get();
                if (frame) {
                        h264_encoder_frame_t h264_frame;
                        if (esp_h264_encode_frame(frame->buf, frame->len, &h264_frame) == ESP_OK) {
                                // TODO: Implement RTP packetization and sending
                                ESP_LOGI("STREAMING", "Encoded frame of size %d", h264_frame.len);
                        }
                        esp_camera_fb_return(frame);
                }
                vTaskDelay(pdMS_TO_TICKS(66)); // ~15 FPS
        }
}

// Status Updates
void update_streaming_status(bool is_streaming) {
        uint8_t status = is_streaming ? 0x01 : 0x00;
        homekit_value_destruct(&streaming_status.value);
        streaming_status.value = HOMEKIT_UINT8(status);
        homekit_characteristic_notify(&streaming_status, streaming_status.value);
}

// GPIO Settings
void gpio_init() {
        gpio_reset_pin(LED_GPIO);
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
        led_write(led_on);
}

// Accessory identification
void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 2; j++) {
                        led_write(true);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        led_write(false);
                        vTaskDelay(pdMS_TO_TICKS(100));
                }
                vTaskDelay(pdMS_TO_TICKS(250));
        }
        led_write(led_on);
        vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
        ESP_LOGI("INFORMATION", "Accessory identify");
        xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// HomeKit characteristics
#define DEVICE_NAME "HomeKit IP Camera"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/C"
#define FW_VERSION "0.0.5"

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  DEVICE_MANUFACTURER);
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_ip_camera, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(CAMERA_RTP_STREAM_MANAGEMENT, .characteristics = (homekit_characteristic_t*[]) {
                        &streaming_status,
                        &supported_video_stream_configuration,
                        &supported_audio_stream_configuration,
                        &supported_rtp_configuration,
                        &setup_endpoints,
                        &selected_rtp_stream_configuration,
                        NULL
                }),
                NULL
        }),
        NULL
};
#pragma GCC diagnostic pop

homekit_server_config_t config = {
        .accessories = accessories,
        .password = CONFIG_ESP_SETUP_CODE,
        .setupId = CONFIG_ESP_SETUP_ID,
};

void on_wifi_ready() {
        ESP_LOGI("INFORMATION", "Starting HomeKit server...");
        homekit_server_init(&config);
        xTaskCreate(video_streaming_task, "Video Streaming", 4096, NULL, 5, NULL);
}

void app_main(void) {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_LOGW("WARNING", "NVS flash initialization failed, erasing...");
                CHECK_ERROR(nvs_flash_erase());
                ret = nvs_flash_init();
        }
        CHECK_ERROR(ret);

        wifi_init();
        gpio_init();
        camera_init();
}
