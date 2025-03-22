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
#include <esp_system.h>    // for esp_restart()
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_timer.h>     // for esp_timer_get_time()
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// Removed: #include <button.h>

// =======================
// Error handling macros
// =======================
#define CHECK_ERROR(x) do {                                               \
    esp_err_t __err_rc = (x);                                             \
    if (__err_rc != ESP_OK) {                                             \
        ESP_LOGE("INFORMATION", "Error: %s", esp_err_to_name(__err_rc));  \
        handle_error(__err_rc);                                           \
    }                                                                     \
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

// Forward declaration for once Wi-Fi is ready
void on_wifi_ready(void);

// ==================
// Wi-Fi Event Logic
// ==================
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI("INFORMATION", "Connecting to WiFi...");
            esp_wifi_connect();
        }
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

// ====================================
// Single Button / LED GPIO Definition
// ====================================
#define BUTTON_GPIO  CONFIG_ESP_BUTTON_GPIO
#define LED_GPIO     CONFIG_ESP_LED_GPIO

bool led_on = false;
void led_write(bool on) {
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

// Setup all initial GPIO states
void gpio_init() {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    led_write(led_on);
}

// ========================
// Accessory Identification
// ========================
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
    // Restore LED
    led_write(led_on);
    vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
    ESP_LOGI("INFORMATION", "Accessory identify");
    xTaskCreate(accessory_identify_task, "Accessory identify",
                configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// (Optional) "Button identify" if you had a second identify
void button_identify(homekit_value_t _value) {
    ESP_LOGI("INFORMATION", "Button identify");
}

// ============================
// HomeKit Characteristic(s)
// ============================

// This is the characteristic we update when the button is pressed
// PROGRAMMABLE_SWITCH_EVENT can be 0=single, 1=double, 2=long, etc.
homekit_characteristic_t button_event =
    HOMEKIT_CHARACTERISTIC_(PROGRAMMABLE_SWITCH_EVENT, 0);

// Original callback function that used to come from <button.h> logic
// We'll define an enum for these events
typedef enum {
    button_event_single_press,
    button_event_double_press,
    button_event_long_press
} custom_button_event_t;

// This is the callback your code triggers for single/double/long:
void button_callback(custom_button_event_t event, void *context) {
    switch (event) {
    case button_event_single_press:
        ESP_LOGI("SINGLE_PRESS", "single press");
        homekit_characteristic_notify(&button_event, HOMEKIT_UINT8(0));
        break;
    case button_event_double_press:
        ESP_LOGI("DOUBLE_PRESS", "Double press");
        homekit_characteristic_notify(&button_event, HOMEKIT_UINT8(1));
        break;
    case button_event_long_press:
        ESP_LOGI("LONG_PRESS", "Long press");
        homekit_characteristic_notify(&button_event, HOMEKIT_UINT8(2));
        break;
    default:
        ESP_LOGI("INFORMATION", "Unknown button event: %d", event);
        break;
    }
}

// ===============================================
// Replace <button.h> with native ESP-IDF logic
// ===============================================

// We'll track press times for single/double/long press
static QueueHandle_t button_evt_queue = NULL;

// State machine for 1 button
static bool button_pressed = false;
static int press_count     = 0;
static bool waiting_for_second_press  = false;
static bool double_press_detected     = false;
static int64_t press_start_time_ms    = 0;
static int64_t last_release_time_ms   = 0;

// ISR: push the GPIO number into a queue
static void IRAM_ATTR button_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(button_evt_queue, &gpio_num, NULL);
}

static void button_task(void *pvParameter) {
    // Press thresholds (ms)
    const int DOUBLE_PRESS_WINDOW_MS  = 500;
    const int LONG_PRESS_THRESHOLD_MS = 1000;

    uint32_t io_num;
    for (;;) {
        // Wait for ISR event
        if (xQueueReceive(button_evt_queue, &io_num, portMAX_DELAY)) {
            int64_t now_ms = esp_timer_get_time() / 1000;
            bool level = gpio_get_level(io_num); // active low

            if (level == 0) {
                // Press
                button_pressed = true;
                press_start_time_ms = now_ms;
                press_count++;
                double_press_detected = false;
                waiting_for_second_press = false;
            } else {
                // Release
                button_pressed = false;
                int64_t press_duration_ms = now_ms - press_start_time_ms;

                if (press_duration_ms >= LONG_PRESS_THRESHOLD_MS) {
                    // Long press
                    ESP_LOGI("BUTTON", "Long press detected");
                    button_callback(button_event_long_press, NULL);
                    press_count = 0;
                } else {
                    // Short press
                    if (press_count == 1) {
                        // Possibly single press => wait for second press
                        waiting_for_second_press = true;
                        last_release_time_ms     = now_ms;
                    } else if (press_count == 2) {
                        // Possibly double press
                        int64_t diff = now_ms - last_release_time_ms;
                        if (diff <= DOUBLE_PRESS_WINDOW_MS) {
                            ESP_LOGI("BUTTON", "Double press detected");
                            button_callback(button_event_double_press, NULL);
                            double_press_detected = true;
                        }
                        waiting_for_second_press = false;
                        press_count = 0;
                    }
                }
            }
        }

        // If we were waiting for a second press, see if time expired
        if (waiting_for_second_press) {
            int64_t now_ms = esp_timer_get_time() / 1000;
            if ((now_ms - last_release_time_ms) > DOUBLE_PRESS_WINDOW_MS) {
                // Timed out => single press
                waiting_for_second_press = false;
                if (!double_press_detected) {
                    ESP_LOGI("BUTTON", "Single press detected");
                    button_callback(button_event_single_press, NULL);
                }
                press_count = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // small delay
    }
}

static void custom_button_init(void) {
    // Setup the button GPIO
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_ANYEDGE,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = 1, // active low button
        .pull_down_en = 0,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
    };
    gpio_config(&io_conf);

    // Create a queue & button_task
    button_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    // ISR service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void *)BUTTON_GPIO);
}

// =======================
// HomeKit Accessory Info
// =======================
#define DEVICE_NAME         "Programmable Switch"
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
    HOMEKIT_ACCESSORY(
        .id = 1,
        .category = homekit_accessory_category_programmable_switches,
        .services = (homekit_service_t*[]) {
            HOMEKIT_SERVICE(
                ACCESSORY_INFORMATION,
                .characteristics = (homekit_characteristic_t*[]) {
                    &name,
                    &manufacturer,
                    &serial,
                    &model,
                    &revision,
                    HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                    NULL
                }
            ),
            HOMEKIT_SERVICE(
                STATELESS_PROGRAMMABLE_SWITCH,
                .primary = true,
                .characteristics = (homekit_characteristic_t*[]) {
                    HOMEKIT_CHARACTERISTIC(NAME, "Programmable switch"),
                    &button_event,
                    NULL
                }
            ),
            NULL
        }
    ),
    NULL
};
#pragma GCC diagnostic pop

homekit_server_config_t config = {
    .accessories = accessories,
    .password = CONFIG_ESP_SETUP_CODE,
    .setupId  = CONFIG_ESP_SETUP_ID,
};

// Called once Wi-Fi is connected
void on_wifi_ready() {
    ESP_LOGI("INFORMATION", "Starting HomeKit server...");
    homekit_server_init(&config);
}

// ============
// app_main
// ============
void app_main(void) {
    // NVS init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW("WARNING", "NVS flash initialization failed, erasing...");
        CHECK_ERROR(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    CHECK_ERROR(ret);

    // Wi-Fi, GPIO, Button
    wifi_init();
    gpio_init();
    custom_button_init();  // Replaces button_create(...)

    // No more references to the old <button.h> library
}
