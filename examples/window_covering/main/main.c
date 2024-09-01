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
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

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


// Function declarations for better code organization
static void handle_error(esp_err_t err);
static void on_wifi_ready();
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void wifi_init();
static void motor_write(const homekit_value_t value);
static void led_write(bool on);
static void gpio_init();
static void accessory_identify_task(void *args);
static void accessory_identify(homekit_value_t _value);

// Motor control GPIO pins
#define MOTOR_UP_GPIO CONFIG_ESP_MOTOR_UP_GPIO
#define MOTOR_DOWN_GPIO CONFIG_ESP_MOTOR_DOWN_GPIO

homekit_characteristic_t current_position = HOMEKIT_CHARACTERISTIC_(CURRENT_POSITION, 0);
homekit_characteristic_t position_state = HOMEKIT_CHARACTERISTIC_(POSITION_STATE, 0);

// Function to control the motor
static void motor_write(const homekit_value_t value) {
        uint8_t target_position = value.int_value;
        target_position = (target_position > 100) ? 100 : target_position;

        uint8_t current_position_value = current_position.value.int_value;

        if (target_position == current_position_value) {
                return;
        }

        gpio_set_level(MOTOR_UP_GPIO, 0);
        gpio_set_level(MOTOR_DOWN_GPIO, 0);

        vTaskDelay(pdMS_TO_TICKS(100)); // Short delay for the motor to stop

        int movement_direction = (target_position > current_position_value) ? 1 : -1;

        if (movement_direction == 1) {
                gpio_set_level(MOTOR_UP_GPIO, 1);
                gpio_set_level(MOTOR_DOWN_GPIO, 0);
                position_state.value.int_value = 0; // Set position state to "up"
        } else {
                gpio_set_level(MOTOR_UP_GPIO, 0);
                gpio_set_level(MOTOR_DOWN_GPIO, 1);
                position_state.value.int_value = 1; // Set position state to "down"
        }

        homekit_characteristic_notify(&position_state, position_state.value);

        int movement_time = abs(target_position - current_position_value);
        vTaskDelay(pdMS_TO_TICKS(movement_time * 100)); // Move the motor for 'movement_time' seconds

        gpio_set_level(MOTOR_UP_GPIO, 0);
        gpio_set_level(MOTOR_DOWN_GPIO, 0);
        position_state.value.int_value = 2; // Set position state to "stopped"

        current_position.value.int_value = target_position;
        homekit_characteristic_notify(&current_position, current_position.value);
}

// LED control
#define LED_GPIO CONFIG_ESP_LED_GPIO
static bool led_on = false;

static void led_write(bool on) {
        gpio_set_level(LED_GPIO, on ? 1 : 0);
}

// GPIO initialization
static void gpio_init() {
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
        led_write(led_on);

        gpio_set_direction(MOTOR_UP_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_direction(MOTOR_DOWN_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(MOTOR_UP_GPIO, 0);
        gpio_set_level(MOTOR_DOWN_GPIO, 0);
}

// Accessory identification
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
        xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE + 128, NULL, 2, NULL);
}

homekit_characteristic_t target_position = HOMEKIT_CHARACTERISTIC_(TARGET_POSITION, 0, .setter = motor_write);

// HomeKit characteristics
#define DEVICE_NAME "HomeKit window covering"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/A"
#define FW_VERSION "0.0.1"

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, DEVICE_MANUFACTURER);
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_window_coverings, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(WINDOW_COVERING, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit window covering"),
                        &target_position,
                        &current_position,
                        &position_state,
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

static void on_wifi_ready() {
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
}
