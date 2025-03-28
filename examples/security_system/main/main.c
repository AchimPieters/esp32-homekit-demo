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

// Error checking macro with detailed logging
#define CHECK_ERROR(x) do {                             \
                esp_err_t __err_rc = (x);                           \
                if (__err_rc != ESP_OK) {                           \
                        ESP_LOGE("ERROR", "Function %s failed at line %d with error: %s", \
                                 __FUNCTION__, __LINE__, esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);                         \
                }                                                   \
} while(0)

static void handle_error(esp_err_t err) {
        switch (err) {
        case ESP_ERR_WIFI_NOT_STARTED:
        case ESP_ERR_WIFI_CONN:
                ESP_LOGI("INFORMATION", "Recoverable WiFi error, attempting restart...");
                esp_wifi_stop();
                esp_wifi_start();
                break;
        default:
                ESP_LOGE("ERROR", "Unrecoverable error, restarting device...");
                esp_restart();
                break;
        }
}

// Forward declarations for clarity
static void on_wifi_ready();
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

// WiFi initialization with optimized stack usage
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

// GPIO and LED control
static void gpio_init() {
        gpio_set_direction(CONFIG_ESP_LED_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(CONFIG_ESP_LED_GPIO, 0); // Ensure LED is off initially
}

// Accessory identification
static void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                gpio_set_level(CONFIG_ESP_LED_GPIO, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(CONFIG_ESP_LED_GPIO, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelete(NULL);
}

static void accessory_identify(homekit_value_t _value) {
        ESP_LOGI("INFORMATION", "Accessory identify");
        xTaskCreate(accessory_identify_task, "AccessoryIdentify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// Update security system state
static void update_state();

static void on_update(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
        update_state();
}

// Security system characteristics
homekit_characteristic_t security_system_current_state = HOMEKIT_CHARACTERISTIC_(SECURITY_SYSTEM_CURRENT_STATE, 0);
homekit_characteristic_t security_system_target_state = HOMEKIT_CHARACTERISTIC_(SECURITY_SYSTEM_TARGET_STATE, 0, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));

static void update_state() {
        int current_state = security_system_current_state.value.int_value;
        int target_state = security_system_target_state.value.int_value;

        if (current_state != target_state) {
                security_system_current_state.value = HOMEKIT_UINT8(target_state);
                const char* state_name;
                switch (target_state) {
                case 0: state_name = "Stay Arm"; break;
                case 1: state_name = "Away Arm"; break;
                case 2: state_name = "Night Arm"; break;
                case 3: state_name = "Disarmed"; break;
                case 4: state_name = "Alarm Triggered"; break;
                default: state_name = "Unknown"; break;
                }
                ESP_LOGI("SECURITY", "Security System State Changed: %s", state_name);
                homekit_characteristic_notify(&security_system_current_state, security_system_current_state.value);
        }
}

// =======================
// HomeKit Accessory Info
// =======================
#define DEVICE_NAME         "security system"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL       "NLDA4SQN1466"
#define DEVICE_MODEL        "SD466NL/A"
#define FW_VERSION          "0.0.1"

homekit_characteristic_t name         = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, DEVICE_MANUFACTURER);
homekit_characteristic_t serial       = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model        = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision     = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_security_systems, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(SECURITY_SYSTEM, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Security System"),
                        &security_system_current_state,
                        &security_system_target_state,
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

// Event handler for WiFi events
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
        if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
                ESP_LOGI("INFORMATION", "Connecting to WiFi...");
                esp_wifi_connect();
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                ESP_LOGI("INFORMATION", "WiFi connected, IP obtained");
                on_wifi_ready();
        }
}

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
