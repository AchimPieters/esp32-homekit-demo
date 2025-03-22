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

// Define GPIO Pins
#define LED_GPIO 2                 // GPIO pin for LED
#define MOTION_SENSOR_GPIO 4        // GPIO pin for Motion Sensor

// Device Information
#define DEVICE_NAME "ESP32-Motion-Sensor"
#define DEVICE_MANUFACTURER "StudioPieters"
#define DEVICE_SERIAL "123456789"
#define DEVICE_MODEL "ESP32-HK-MS"
#define FW_VERSION "1.0.0"

// Global Variables
bool led_on = false;

// Function to control the LED
void led_write(bool state) {
    gpio_set_level(LED_GPIO, state);
}

// Error Handling Macro
#define CHECK_ERROR(x) do {                                  \
                esp_err_t __err_rc = (x);                    \
                if (__err_rc != ESP_OK) {                    \
                        ESP_LOGE("ERROR", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);              \
                }                                            \
} while(0)

// Error Handling Function
static void handle_error(esp_err_t err) {
    switch (err) {
        case ESP_ERR_WIFI_NOT_STARTED:
        case ESP_ERR_WIFI_CONN:
            ESP_LOGI("ERROR", "Restarting WiFi...");
            esp_wifi_stop();
            esp_wifi_start();
            break;
        default:
            ESP_LOGE("ERROR", "Critical error, restarting device...");
            esp_restart();
            break;
    }
}

// Forward Declaration
static void on_wifi_ready();

// Event Handler for WiFi
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI("WIFI", "Connecting to WiFi...");
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI("WIFI", "WiFi connected, IP obtained");
        on_wifi_ready();
    }
}

// WiFi Initialization
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

// GPIO Initialization
static void gpio_init() {
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTION_SENSOR_GPIO, GPIO_MODE_INPUT);
    led_write(led_on);
}

// Accessory Identification Task
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

// Accessory Identify Function
static void accessory_identify(homekit_value_t _value) {
    ESP_LOGI("HOMEKIT", "Accessory identify");
    xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// HomeKit Characteristic for Motion Detection
homekit_characteristic_t Motion_detected = HOMEKIT_CHARACTERISTIC_(MOTION_DETECTED, 0);

// Motion Sensor Task
static void motion_sensor_task(void *pvParameters) {
    while (1) {
        bool motion_detected = gpio_get_level(MOTION_SENSOR_GPIO) == 1;
        homekit_characteristic_notify(&Motion_detected, HOMEKIT_BOOL(motion_detected));
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

// Motion Sensor Initialization
static void motion_sensor_init() {
    ESP_LOGI("HOMEKIT", "Initializing Motion Sensor Task");
    xTaskCreate(motion_sensor_task, "MotionSensorTask", 1024, NULL, 2, NULL);
}

// HomeKit Accessory Information
homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, DEVICE_MANUFACTURER);
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

// HomeKit Accessories Definition
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_sensors, .services = (homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
            &name, &manufacturer, &serial, &model, &revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(MOTION_SENSOR, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Motion Sensor"),
            &Motion_detected, NULL
        }),
        NULL
    }),
    NULL
};
#pragma GCC diagnostic pop

// HomeKit Server Configuration
static homekit_server_config_t config = {
    .accessories = accessories,
    .password = CONFIG_ESP_SETUP_CODE,
    .setupId = CONFIG_ESP_SETUP_ID,
};

// Callback when WiFi is Ready
static void on_wifi_ready() {
    ESP_LOGI("HOMEKIT", "Starting HomeKit server...");
    homekit_server_init(&config);
}

// Application Entry Point
void app_main(void) {
    CHECK_ERROR(nvs_flash_init());
    wifi_init();
    gpio_init();
    motion_sensor_init();
}
