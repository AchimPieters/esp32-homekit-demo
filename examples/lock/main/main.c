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
#include <button.h>

// Custom error handling macro
#define CHECK_ERROR(x) do {                        \
                esp_err_t __err_rc = (x);                  \
                if (__err_rc != ESP_OK) {                  \
                        ESP_LOGE("INFORMATION", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);                \
                }                                          \
} while(0)

void handle_error(esp_err_t err) {
        // Custom error handling logic
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

#define BUTTON_GPIO CONFIG_ESP_BUTTON_GPIO
#define LED_GPIO CONFIG_ESP_LED_GPIO
#define RELAY_GPIO CONFIG_ESP_RELAY_GPIO

// Static variable definitions
static bool led_on = false;
static bool relay_on = false;

// Function declarations
static void on_wifi_ready(void);
static void handle_error(esp_err_t err);
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void wifi_init(void);
static void gpio_init(void);
static void accessory_identify_task(void *args);
static void accessory_identify(homekit_value_t _value);
static void button_callback(button_event_t event, void *context);
static void lock_target_state_setter(homekit_value_t value);
static void lock_lock(void);
static void lock_timeout(void);
static void lock_init(void);
static void lock_unlock(void);
static void lock_control_point(homekit_value_t value);
static void app_main(void);
static void gpio_init(void) {
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
        led_write(led_on);

        gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
        relay_write(relay_on);
}

static void accessory_identify_task(void *args) {
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

static void accessory_identify(homekit_value_t _value) {
        ESP_LOGI("INFORMATION", "Accessory identify");
        xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

static void button_callback(button_event_t event, void *context) {
        switch (event) {
        case button_event_single_press:
                ESP_LOGI("INFORMATION", "Single press");
                lock_unlock();
                break;
        case button_event_double_press:
                ESP_LOGI("INFORMATION", "Double press");
                break;
        case button_event_long_press:
                ESP_LOGI("INFORMATION", "Long press");
                break;
        default:
                ESP_LOGI("INFORMATION", "Unknown button event: %d", event);
                break;
        }
}

static void lock_target_state_setter(homekit_value_t value) {
        lock_target_state.value = value;
        if (value.int_value == lock_state_unsecured) {
                lock_unlock();
        } else {
                lock_lock();
        }
}

static void lock_lock(void) {
        relay_write(false);
        led_write(false);
        if (lock_current_state.value.int_value != lock_state_secured) {
                lock_current_state.value = HOMEKIT_UINT8(lock_state_secured);
                homekit_characteristic_notify(&lock_current_state, lock_current_state.value);
                lock_last_known_action.value = HOMEKIT_UINT8(1); // Lock
                homekit_characteristic_notify(&lock_last_known_action, lock_last_known_action.value);
        }
}

static void lock_timeout(void) {
        if (lock_target_state.value.int_value != lock_state_secured) {
                lock_target_state.value = HOMEKIT_UINT8(lock_state_secured);
                homekit_characteristic_notify(&lock_target_state, lock_target_state.value);
        }
        lock_lock();
}

static void lock_init(void) {
        lock_current_state.value = HOMEKIT_UINT8(lock_state_secured);
        homekit_characteristic_notify(&lock_current_state, lock_current_state.value);
        lock_timeout();
}

static void lock_unlock(void) {
        relay_write(true);
        led_write(true);
        lock_current_state.value = HOMEKIT_UINT8(lock_state_unsecured);
        homekit_characteristic_notify(&lock_current_state, lock_current_state.value);
        lock_last_known_action.value = HOMEKIT_UINT8(0); // Unlock
        homekit_characteristic_notify(&lock_last_known_action, lock_last_known_action.value);
        vTaskDelay(pdMS_TO_TICKS(1000 * CONFIG_ESP_LOCK_OPEN)); // Delay
        lock_init();
}

static void lock_control_point(homekit_value_t value) {
        // Intentionally left empty
}

static void on_wifi_ready(void) {
        ESP_LOGI("INFORMATION", "Starting HomeKit server...");
        homekit_server_init(&config);
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
        lock_init();

        button_config_t button_config = BUTTON_CONFIG(
                button_active_low,
                .max_repeat_presses = 2,
                .long_press_time = 1000
                );

        if (button_create(BUTTON_GPIO, button_config, button_callback, NULL)) {
                ESP_LOGI("INFORMATION", "Failed to initialize button");
        }
}
