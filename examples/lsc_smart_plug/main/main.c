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
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>     // For esp_restart()
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_timer.h>      // For esp_timer_get_time()
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// If you have defined these in Kconfig.projbuild, they're available via sdkconfig:
#define BUTTON_GPIO CONFIG_ESP_BUTTON_GPIO
#define LED_GPIO    CONFIG_ESP_LED_GPIO
#define RELAY_GPIO  CONFIG_ESP_RELAY_GPIO

// Provide a tag for logging
#define LOG_TAG "MAIN"

// A macro for checking and logging errors
#define CHECK_ERROR(x) do {                                 \
    esp_err_t __err_rc = (x);                               \
    if (__err_rc != ESP_OK) {                               \
        ESP_LOGE(LOG_TAG, "Error: %s", esp_err_to_name(__err_rc)); \
        handle_error(__err_rc);                             \
    }                                                       \
} while(0)

// For demonstration, we handle certain errors by restarting Wi-Fi or the device
static void handle_error(esp_err_t err) {
    switch (err) {
    case ESP_ERR_WIFI_NOT_STARTED:
    case ESP_ERR_WIFI_CONN:
        ESP_LOGI("INFORMATION", "Recoverable error occurred. Restarting Wi-Fi...");
        esp_wifi_stop();
        esp_wifi_start();
        break;
    default:
        ESP_LOGE("ERROR", "Critical error occurred. Restarting device...");
        esp_restart();
        break;
    }
}

////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////
static void on_wifi_ready(void);

////////////////////////////////////////////////////////////////
// Wi-Fi event handler
////////////////////////////////////////////////////////////////
static void event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI("INFORMATION", "Connecting to Wi-Fi...");
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI("INFORMATION", "Wi-Fi connected, IP obtained");
        on_wifi_ready();
    }
}

static void wifi_init(void) {
    CHECK_ERROR(esp_netif_init());
    CHECK_ERROR(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    CHECK_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    CHECK_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    CHECK_ERROR(esp_wifi_init(&wifi_init_config));
    CHECK_ERROR(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Grab SSID and password from Kconfig
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,          // from Kconfig
            .password = CONFIG_ESP_WIFI_PASSWORD,  // from Kconfig
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    CHECK_ERROR(esp_wifi_set_mode(WIFI_MODE_STA));
    CHECK_ERROR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    CHECK_ERROR(esp_wifi_start());
}

////////////////////////////////////////////////////////////////
// GPIO and Relay
////////////////////////////////////////////////////////////////
static bool led_on = false;

static void led_write(bool on) {
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

static void relay_write(bool on) {
    gpio_set_level(RELAY_GPIO, on ? 1 : 0);
}

////////////////////////////////////////////////////////////////
// HomeKit callback
////////////////////////////////////////////////////////////////
static void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);

homekit_characteristic_t switch_on = HOMEKIT_CHARACTERISTIC_(
    ON,
    false,
    .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback)
);

static void gpio_init(void) {
    // LED output
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    led_write(led_on);

    // Relay output
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    relay_write(switch_on.value.bool_value);
}

////////////////////////////////////////////////////////////////
// Button Handling (Single/Double/Long Press)
////////////////////////////////////////////////////////////////
static QueueHandle_t button_evt_queue = NULL;

// For multi-press logic
static volatile int64_t press_start_time_ms = 0;
static volatile bool button_pressed         = false;
static volatile int press_count             = 0;

// ISR handler
static void IRAM_ATTR button_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(button_evt_queue, &gpio_num, NULL);
}

static void button_task(void *pvParameter) {
    uint32_t io_num;
    // Time thresholds for press logic
    const int DOUBLE_PRESS_WINDOW_MS  = 500;
    const int LONG_PRESS_THRESHOLD_MS = 1000;

    int64_t last_release_time_ms     = 0;
    bool waiting_for_second_press    = false;
    bool double_press_detected       = false;

    for (;;) {
        if (xQueueReceive(button_evt_queue, &io_num, portMAX_DELAY)) {
            int64_t now_ms = esp_timer_get_time() / 1000;
            bool level     = gpio_get_level(io_num);  // Active low

            if (level == 0) {
                // Press
                button_pressed = true;
                press_start_time_ms = now_ms;
                press_count++;
                double_press_detected    = false;
                waiting_for_second_press = false;
            } else {
                // Release
                button_pressed = false;
                int64_t press_duration_ms = now_ms - press_start_time_ms;

                if (press_duration_ms >= LONG_PRESS_THRESHOLD_MS) {
                    // Long press
                    ESP_LOGI("INFORMATION", "Long press detected");
                    // Additional long-press logic here...
                    press_count = 0;
                } else {
                    // Short press
                    if (press_count == 1) {
                        // Possibly single press => wait for second press
                        waiting_for_second_press = true;
                        last_release_time_ms     = now_ms;
                    } else if (press_count == 2) {
                        // Possibly double press
                        int64_t time_between_presses_ms = now_ms - last_release_time_ms;
                        if (time_between_presses_ms <= DOUBLE_PRESS_WINDOW_MS) {
                            ESP_LOGI("INFORMATION", "Double press detected");
                            // Additional double-press logic...
                            double_press_detected = true;
                        }
                        waiting_for_second_press = false;
                        press_count = 0;
                    }
                }
            }
        }

        // If we're waiting for a second press but time expires => single press
        if (waiting_for_second_press) {
            int64_t now_ms = esp_timer_get_time() / 1000;
            if ((now_ms - last_release_time_ms) > DOUBLE_PRESS_WINDOW_MS) {
                waiting_for_second_press = false;
                if (!double_press_detected) {
                    ESP_LOGI("INFORMATION", "Single press detected");
                    // Single press => toggle the relay
                    switch_on.value.bool_value = !switch_on.value.bool_value;
                    relay_write(switch_on.value.bool_value);
                    homekit_characteristic_notify(&switch_on, switch_on.value);
                }
                press_count = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void button_init(void) {
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_ANYEDGE, // Trigger on both edges
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = 1,                 // Use internal pull-up
        .pull_down_en = 0,
        .pin_bit_mask = (1ULL << BUTTON_GPIO)
    };
    gpio_config(&io_conf);

    button_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    if (!button_evt_queue) {
        ESP_LOGE(LOG_TAG, "Failed to create button queue");
        return;
    }

    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    // Install ISR service and attach our ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void *)BUTTON_GPIO);
}

////////////////////////////////////////////////////////////////
// HomeKit Accessory Setup
////////////////////////////////////////////////////////////////

// Called by HomeKit for "Identify Accessory" functionality
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
    // Restore LED to prior state
    led_write(led_on);
    vTaskDelete(NULL);
}

static void accessory_identify(homekit_value_t _value) {
    ESP_LOGI("INFORMATION", "Accessory identify");
    xTaskCreate(
        accessory_identify_task,
        "Accessory identify",
        configMINIMAL_STACK_SIZE,
        NULL,
        2,
        NULL
    );
}

// Switch callback
static void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write(switch_on.value.bool_value);
}

// Device constants
#define DEVICE_NAME         "HomeKit Smart Plug"
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
        .category = homekit_accessory_category_switches,
        .services = (homekit_service_t*[]) {
            HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                &name,
                &manufacturer,
                &serial,
                &model,
                &revision,
                HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                NULL
            }),
            HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
                HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
                &switch_on,
                NULL
            }),
            NULL
        }
    ),
    NULL
};
#pragma GCC diagnostic pop

// Use your Kconfig values for HomeKit server config
homekit_server_config_t config = {
    .accessories = accessories,
    .password    = CONFIG_ESP_SETUP_CODE,  // from Kconfig
    .setupId     = CONFIG_ESP_SETUP_ID,    // from Kconfig
};

// Once Wi-Fi is connected, start the HomeKit server
static void on_wifi_ready(void) {
    ESP_LOGI("INFORMATION", "Starting HomeKit server...");
    homekit_server_init(&config);
}

////////////////////////////////////////////////////////////////
// app_main
////////////////////////////////////////////////////////////////
void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW("WARNING", "NVS flash init failed, erasing...");
        CHECK_ERROR(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    CHECK_ERROR(ret);

    // Wi-Fi, GPIOs, and button
    wifi_init();
    gpio_init();
    button_init();

    // At this point, once Wi-Fi connects, HomeKit server will start in on_wifi_ready().
}
