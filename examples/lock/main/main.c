/**
   Copyright 2025 Achim Pieters | StudioPieters®

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

   For more information visit https://www.studiopieters.nl
 **/
 
#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// Custom error handling macro
#define CHECK_ERROR(x) do {                        \
                esp_err_t __err_rc = (x);                  \
                if (__err_rc != ESP_OK) {                  \
                        ESP_LOGE("ERROR", "Error: %s", esp_err_to_name(__err_rc)); \
                        esp_restart();                \
                }                                          \
} while(0)

void on_wifi_ready();

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        ESP_LOGI("INFO", "Connecting to WiFi...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI("INFO", "WiFi connected, IP obtained");
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
    CHECK_ERROR(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    CHECK_ERROR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    CHECK_ERROR(esp_wifi_start());
}

// GPIO Settings
#define LED_GPIO CONFIG_ESP_LED_GPIO
#define RELAY_GPIO CONFIG_ESP_RELAY_GPIO
#define LOCK_OPEN_TIME CONFIG_ESP_LOCK_OPEN

bool lock_state = false;

void led_write(bool on) {
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

void lock_write(bool open) {
    gpio_set_level(RELAY_GPIO, open ? 1 : 0);
    if (open) {
        vTaskDelay(pdMS_TO_TICKS(LOCK_OPEN_TIME * 1000));
        gpio_set_level(RELAY_GPIO, 0);
        lock_state = false;
    }
}

void gpio_init() {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    led_write(false);

    gpio_reset_pin(RELAY_GPIO);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_GPIO, 0);
}

// HomeKit Accessory Identification
void accessory_identify_task(void *args) {
    for (int i = 0; i < 3; i++) {
        led_write(true);
        vTaskDelay(pdMS_TO_TICKS(200));
        led_write(false);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    led_write(false);
    vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
    ESP_LOGI("INFO", "Accessory identify");
    xTaskCreate(accessory_identify_task, "Accessory Identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

homekit_value_t lock_current_state_get() {
    return HOMEKIT_UINT8(lock_state ? 1 : 0);
}

homekit_value_t lock_target_state_get() {
    return HOMEKIT_UINT8(lock_state ? 1 : 0);
}

void lock_target_state_set(homekit_value_t value) {
    if (value.format != homekit_format_uint8) {
        ESP_LOGE("ERROR", "Invalid format");
        return;
    }
    bool new_state = value.uint8_value == 1;
    if (new_state != lock_state) {
        lock_state = new_state;
        lock_write(lock_state);
    }
}

// HomeKit Characteristics
#define DEVICE_NAME "HomeKit Lock"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/A"
#define FW_VERSION "1.0.0"

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, DEVICE_MANUFACTURER);
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);
homekit_characteristic_t lock_current_state = HOMEKIT_CHARACTERISTIC_(LOCK_CURRENT_STATE, 0, .getter = lock_current_state_get);
homekit_characteristic_t lock_target_state = HOMEKIT_CHARACTERISTIC_(LOCK_TARGET_STATE, 0, .getter = lock_target_state_get, .setter = lock_target_state_set);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_locks, .services = (homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
            &name,
            &manufacturer,
            &serial,
            &model,
            &revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LOCK_MECHANISM, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Smart Lock"),
            &lock_current_state,
            &lock_target_state,
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
    ESP_LOGI("INFO", "Starting HomeKit server...");
    homekit_server_init(&config);
}

void app_main(void) {
    CHECK_ERROR(nvs_flash_init());
    wifi_init();
    gpio_init();
}
