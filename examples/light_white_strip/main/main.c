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
#include <driver/ledc.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// WiFi setup
void on_wifi_ready();

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
        if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
                ESP_LOGI("WIFI_EVENT", "STA start");
                esp_wifi_connect();
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                ESP_LOGI("IP_EVENT", "WiFI ready");
                on_wifi_ready();
        }
}

static void wifi_init() {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        wifi_config_t wifi_config = {
                .sta = {
                        .ssid = CONFIG_ESP_WIFI_SSID,
                        .password = CONFIG_ESP_WIFI_PASSWORD,
                },
        };

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
}

#define LPF_SHIFT 4  // divide by 16
#define LPF_INTERVAL 10  // in milliseconds

#define WW_PWM_PIN CONFIG_ESP_WW_LED_GPIO
#define CW_PWM_PIN CONFIG_ESP_CW_LED_GPIO

// Color smoothing variables
uint16_t current_ww = 0;
uint16_t target_ww = 0;
uint16_t current_cw = 0;
uint16_t target_cw = 0;

// Global variables
float led_brightness_ww = 100; // brightness is scaled 0 to 100
float led_brightness_cw = 100; // brightness is scaled 0 to 100
bool led_on = false;           // on is boolean on or off

void ledc_task(void *pvParameters) {
        const TickType_t xPeriod = pdMS_TO_TICKS(LPF_INTERVAL);
        TickType_t xLastWakeTime = xTaskGetTickCount();

        ledc_channel_config_t ledc_channel[2] = {
                {
                        .channel    = LEDC_CHANNEL_0,
                        .duty       = 0,
                        .gpio_num   = WW_PWM_PIN,
                        .speed_mode = LEDC_HIGH_SPEED_MODE,
                        .hpoint     = 0,
                        .timer_sel  = LEDC_TIMER_0
                },
                {
                        .channel    = LEDC_CHANNEL_1,
                        .duty       = 0,
                        .gpio_num   = CW_PWM_PIN,
                        .speed_mode = LEDC_HIGH_SPEED_MODE,
                        .hpoint     = 0,
                        .timer_sel  = LEDC_TIMER_0
                }
        };

        ledc_timer_config_t ledc_timer = {
                .speed_mode       = LEDC_HIGH_SPEED_MODE,
                .timer_num        = LEDC_TIMER_0,
                .duty_resolution  = LEDC_TIMER_13_BIT,
                .freq_hz          = 5000,
                .clk_cfg          = LEDC_AUTO_CLK
        };

        ledc_timer_config(&ledc_timer);
        for (int ch = 0; ch < 2; ch++) {
                ledc_channel_config(&ledc_channel[ch]);
        }

        while (1) {
                if (led_on) {
                        target_ww = (uint16_t)((led_brightness_ww / 100.0) * 8191); // 13-bit resolution
                        target_cw = (uint16_t)((led_brightness_cw / 100.0) * 8191); // 13-bit resolution
                } else {
                        target_ww = 0;
                        target_cw = 0;
                }

                current_ww += ((target_ww * 256) - current_ww) >> LPF_SHIFT;
                current_cw += ((target_cw * 256) - current_cw) >> LPF_SHIFT;

                ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, current_ww >> 8);
                ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, current_cw >> 8);
                ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);

                vTaskDelayUntil(&xLastWakeTime, xPeriod);
        }
}

void ledc() {
        ESP_LOGI("LEDC", "Start ledc");
        xTaskCreate(ledc_task, "ledc_task", 4096, NULL, 2, NULL);
}

void led_identify_task(void *_args) {
        ESP_LOGI("LED_IDENTIFY_TASK", "Starting LED identify task");

        uint16_t original_ww = target_ww;
        uint16_t original_cw = target_cw;
        uint16_t off = 0;
        uint16_t on = 4096;

        for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 2; j++) {
                        led_on = true;
                        led_brightness_ww = 50;
                        led_brightness_cw = 50;
                        target_ww = on;
                        target_cw = on;
                        ESP_LOGI("LED_IDENTIFY_TASK", "Setting LEDs to on");
                        vTaskDelay(pdMS_TO_TICKS(100));

                        led_on = false;
                        led_brightness_ww = 0;
                        led_brightness_cw = 0;
                        target_ww = off;
                        target_cw = off;
                        ESP_LOGI("LED_IDENTIFY_TASK", "Setting LEDs to off");
                        vTaskDelay(pdMS_TO_TICKS(100));
                }
                vTaskDelay(pdMS_TO_TICKS(250));
        }

        led_on = false;
        led_brightness_ww = 100;
        led_brightness_cw = 100;
        target_ww = original_ww;
        target_cw = original_cw;
        ESP_LOGI("LED_IDENTIFY_TASK", "LED identify task completed");

        vTaskDelete(NULL);
}

void led_identify(homekit_value_t _value) {
        ESP_LOGI("ACCESSORY_IDENTIFY", "Accessory identify");
        xTaskCreate(led_identify_task, "LED identify", 2048, NULL, 2, NULL);
}

homekit_value_t led_on_get() {
        return HOMEKIT_BOOL(led_on);
}

void led_on_set(homekit_value_t value) {
        if (value.format != homekit_format_bool) {
                return;
        }

        led_on = value.bool_value;
}

homekit_value_t led_brightness_ww_get() {
        return HOMEKIT_INT(led_brightness_ww);
}

void led_brightness_ww_set(homekit_value_t value) {
        if (value.format != homekit_format_int) {
                return;
        }
        led_brightness_ww = value.int_value;
}

homekit_value_t led_brightness_cw_get() {
        return HOMEKIT_INT(led_brightness_cw);
}

void led_brightness_cw_set(homekit_value_t value) {
        if (value.format != homekit_format_int) {
                return;
        }
        led_brightness_cw = value.int_value;
}

// HomeKit characteristics
#define DEVICE_NAME "HomeKit White Strip"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/A"
#define FW_VERSION "0.0.1"
s
homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  DEVICE_MANUFACTURER);
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_lighting, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit White Strip"),
                        HOMEKIT_CHARACTERISTIC(
                                ON, true,
                                .getter = led_on_get,
                                .setter = led_on_set
                                ),
                        HOMEKIT_CHARACTERISTIC(
                                BRIGHTNESS, 100,
                                .getter = led_brightness_ww_get,
                                .setter = led_brightness_ww_set
                                ),
                        HOMEKIT_CHARACTERISTIC(
                                BRIGHTNESS, 100,
                                .getter = led_brightness_cw_get,
                                .setter = led_brightness_cw_set
                                ),
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
        homekit_server_init(&config);
}

void app_main(void) {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
        ledc();
        wifi_init();
}
