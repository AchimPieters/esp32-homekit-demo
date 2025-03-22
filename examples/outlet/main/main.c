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
#include <esp_system.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_timer.h> // Needed for esp_timer_get_time()
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// Removed #include <button.h> and replaced with ESP-IDF logic.

// Custom error handling macro
#define CHECK_ERROR(x) do {                        \
    esp_err_t __err_rc = (x);                      \
    if (__err_rc != ESP_OK) {                      \
        ESP_LOGE("ERROR", "Error: %s", esp_err_to_name(__err_rc)); \
        handle_error(__err_rc);                    \
    }                                              \
} while(0)

static void handle_error(esp_err_t err) {
    // Handle recoverable Wi-Fi errors
    if (err == ESP_ERR_WIFI_NOT_STARTED || err == ESP_ERR_WIFI_CONN) {
        ESP_LOGI("INFORMATION", "Attempting to restart WiFi...");
        esp_wifi_stop();
        esp_wifi_start();
    } else {
        // Handle critical errors
        ESP_LOGE("ERROR", "Critical error, restarting device...");
        esp_restart();
    }
}

static void on_wifi_ready(void);

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT &&
       (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        ESP_LOGI("INFORMATION", "Connecting to WiFi...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI("INFORMATION", "WiFi connected, IP obtained");
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

// =====================
//  GPIO Definitions
// =====================
#define BUTTON_1_GPIO CONFIG_ESP_BUTTON_1_GPIO
#define BUTTON_2_GPIO CONFIG_ESP_BUTTON_2_GPIO
#define BUTTON_3_GPIO CONFIG_ESP_BUTTON_3_GPIO

#define LED_GPIO      CONFIG_ESP_LED_GPIO
#define RELAY_1_GPIO  CONFIG_ESP_RELAY_1_GPIO
#define RELAY_2_GPIO  CONFIG_ESP_RELAY_2_GPIO
#define RELAY_3_GPIO  CONFIG_ESP_RELAY_3_GPIO

static bool led_on = false;
static void led_write(bool on) {
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

static void relay_1_write(bool on) {
    gpio_set_level(RELAY_1_GPIO, on ? 1 : 0);
}
static void relay_2_write(bool on) {
    gpio_set_level(RELAY_2_GPIO, on ? 1 : 0);
}
static void relay_3_write(bool on) {
    gpio_set_level(RELAY_3_GPIO, on ? 1 : 0);
}

void outlet_1_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void outlet_2_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void outlet_3_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);

// HomeKit characteristics
homekit_characteristic_t outlet_on_1 =
    HOMEKIT_CHARACTERISTIC_(ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(outlet_1_on_callback));
homekit_characteristic_t outlet_on_2 =
    HOMEKIT_CHARACTERISTIC_(ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(outlet_2_on_callback));
homekit_characteristic_t outlet_on_3 =
    HOMEKIT_CHARACTERISTIC_(ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(outlet_3_on_callback));

// Initialize GPIO directions
static void gpio_init(void) {
    // LED
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    led_write(led_on);

    // Relays
    gpio_set_direction(RELAY_1_GPIO, GPIO_MODE_OUTPUT);
    relay_1_write(outlet_on_1.value.bool_value);

    gpio_set_direction(RELAY_2_GPIO, GPIO_MODE_OUTPUT);
    relay_2_write(outlet_on_2.value.bool_value);

    gpio_set_direction(RELAY_3_GPIO, GPIO_MODE_OUTPUT);
    relay_3_write(outlet_on_3.value.bool_value);
}

// ==============================
// Accessory Identification Task
// ==============================
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
    xTaskCreate(accessory_identify_task, "Accessory identify",
                configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// ==============================
// HomeKit Callbacks
// ==============================
void outlet_1_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_1_write(outlet_on_1.value.bool_value);
}

void outlet_2_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_2_write(outlet_on_2.value.bool_value);
}

void outlet_3_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_3_write(outlet_on_3.value.bool_value);
}

// ==============================
// Replacing 'button.h' logic
// ==============================

// Define your button events
typedef enum {
    button_event_single_press,
    button_event_double_press,
    button_event_long_press
} button_event_t;

// Original user callbacks for each button (unchanged)
extern void button_1_callback(button_event_t event, void *context);
extern void button_2_callback(button_event_t event, void *context);
extern void button_3_callback(button_event_t event, void *context);

// We’ll reuse these same callbacks but call them from below logic

// Each queue item is simply the GPIO number that triggered the interrupt
static QueueHandle_t button_evt_queue = NULL;

// We maintain separate state for each of the 3 buttons
// so we can do single/double/long press detection
typedef struct {
    bool button_pressed;
    int press_count;
    bool waiting_for_second_press;
    bool double_press_detected;
    int64_t press_start_time_ms;
    int64_t last_release_time_ms;
} button_state_t;

// Indices: 0 => BUTTON_1_GPIO, 1 => BUTTON_2_GPIO, 2 => BUTTON_3_GPIO
static button_state_t button_states[3];

// Helper to map pin -> index
static int get_button_index(uint32_t gpio_num) {
    if (gpio_num == BUTTON_1_GPIO) return 0;
    if (gpio_num == BUTTON_2_GPIO) return 1;
    if (gpio_num == BUTTON_3_GPIO) return 2;
    return -1; // unknown
}

// Helper to call the correct user callback
static void call_user_callback(int idx, button_event_t event) {
    switch (idx) {
    case 0:
        button_1_callback(event, NULL);
        break;
    case 1:
        button_2_callback(event, NULL);
        break;
    case 2:
        button_3_callback(event, NULL);
        break;
    default:
        ESP_LOGW("BUTTON", "Invalid button index %d", idx);
    }
}

// ISR handler for each button
static void IRAM_ATTR button_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t) arg;
    // Push pin number into the queue
    xQueueSendFromISR(button_evt_queue, &gpio_num, NULL);
}

static void button_task(void *pvParameter) {
    // Tweak these as needed
    const int DOUBLE_PRESS_WINDOW_MS  = 500;
    const int LONG_PRESS_THRESHOLD_MS = 1000;

    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(button_evt_queue, &io_num, portMAX_DELAY)) {
            // Identify which button
            int idx = get_button_index(io_num);
            if (idx < 0) {
                // Not recognized
                continue;
            }

            button_state_t *bs = &button_states[idx];
            int64_t now_ms = esp_timer_get_time() / 1000;
            bool level     = gpio_get_level(io_num); // active low

            if (level == 0) {
                // Button pressed
                bs->button_pressed         = true;
                bs->press_start_time_ms    = now_ms;
                bs->press_count++;
                bs->double_press_detected  = false;
                bs->waiting_for_second_press = false;

            } else {
                // Button released
                bs->button_pressed = false;
                int64_t press_duration_ms = now_ms - bs->press_start_time_ms;

                if (press_duration_ms >= LONG_PRESS_THRESHOLD_MS) {
                    // Long press
                    ESP_LOGI("BUTTON", "Long press (idx=%d)", idx);
                    call_user_callback(idx, button_event_long_press);
                    bs->press_count = 0;
                } else {
                    // Short press
                    if (bs->press_count == 1) {
                        // Possibly single press => wait for second press
                        bs->waiting_for_second_press = true;
                        bs->last_release_time_ms     = now_ms;

                    } else if (bs->press_count == 2) {
                        // Possibly double press
                        int64_t diff = now_ms - bs->last_release_time_ms;
                        if (diff <= DOUBLE_PRESS_WINDOW_MS) {
                            ESP_LOGI("BUTTON", "Double press (idx=%d)", idx);
                            call_user_callback(idx, button_event_double_press);
                            bs->double_press_detected = true;
                        }
                        bs->waiting_for_second_press = false;
                        bs->press_count = 0;
                    }
                }
            }
        }

        // Check for single press timing out
        for (int i = 0; i < 3; i++) {
            button_state_t *bs = &button_states[i];
            if (bs->waiting_for_second_press) {
                int64_t now_ms = esp_timer_get_time() / 1000;
                if (now_ms - bs->last_release_time_ms > DOUBLE_PRESS_WINDOW_MS) {
                    // Timed out => single press
                    bs->waiting_for_second_press = false;
                    if (!bs->double_press_detected) {
                        ESP_LOGI("BUTTON", "Single press (idx=%d)", i);
                        call_user_callback(i, button_event_single_press);
                    }
                    bs->press_count = 0;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void button_init(void) {
    // Common input config
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_ANYEDGE, // triggers on rising/falling
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = 1,
        .pull_down_en = 0,
    };

    // For all three buttons
    uint64_t pins_mask = (1ULL << BUTTON_1_GPIO) |
                         (1ULL << BUTTON_2_GPIO) |
                         (1ULL << BUTTON_3_GPIO);
    io_conf.pin_bit_mask = pins_mask;
    gpio_config(&io_conf);

    // Create queue & task
    button_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);

    // Install ISR & attach each pin
    gpio_install_isr_service(0);

    // For each button: attach the same ISR but pass the corresponding pin number
    gpio_isr_handler_add(BUTTON_1_GPIO, button_isr_handler, (void *) BUTTON_1_GPIO);
    gpio_isr_handler_add(BUTTON_2_GPIO, button_isr_handler, (void *) BUTTON_2_GPIO);
    gpio_isr_handler_add(BUTTON_3_GPIO, button_isr_handler, (void *) BUTTON_3_GPIO);
}

// ==========================
// Original user callbacks
// ==========================
// (No longer tied to the old "button_create" calls; they're invoked from our new logic.)

void button_1_callback(button_event_t event, void *context) {
    switch (event) {
        case button_event_single_press:
            ESP_LOGI("BUTTON_1", "Single press");
            outlet_on_1.value.bool_value = !outlet_on_1.value.bool_value;
            relay_1_write(outlet_on_1.value.bool_value);
            homekit_characteristic_notify(&outlet_on_1, outlet_on_1.value);
            break;
        case button_event_double_press:
            ESP_LOGI("BUTTON_1", "Double press");
            break;
        case button_event_long_press:
            ESP_LOGI("BUTTON_1", "Long press");
            break;
        default:
            ESP_LOGI("BUTTON_1", "Unknown event: %d", event);
    }
}

void button_2_callback(button_event_t event, void *context) {
    switch (event) {
        case button_event_single_press:
            ESP_LOGI("BUTTON_2", "Single press");
            outlet_on_2.value.bool_value = !outlet_on_2.value.bool_value;
            relay_2_write(outlet_on_2.value.bool_value);
            homekit_characteristic_notify(&outlet_on_2, outlet_on_2.value);
            break;
        case button_event_double_press:
            ESP_LOGI("BUTTON_2", "Double press");
            break;
        case button_event_long_press:
            ESP_LOGI("BUTTON_2", "Long press");
            break;
        default:
            ESP_LOGI("BUTTON_2", "Unknown event: %d", event);
    }
}

void button_3_callback(button_event_t event, void *context) {
    switch (event) {
        case button_event_single_press:
            ESP_LOGI("BUTTON_3", "Single press");
            outlet_on_3.value.bool_value = !outlet_on_3.value.bool_value;
            relay_3_write(outlet_on_3.value.bool_value);
            homekit_characteristic_notify(&outlet_on_3, outlet_on_3.value);
            break;
        case button_event_double_press:
            ESP_LOGI("BUTTON_3", "Double press");
            break;
        case button_event_long_press:
            ESP_LOGI("BUTTON_3", "Long press");
            break;
        default:
            ESP_LOGI("BUTTON_3", "Unknown event: %d", event);
    }
}

// =======================
// HomeKit Accessory Setup
// =======================
#define DEVICE_NAME         "HomeKit Triple Outlet"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL       "NLDA4SQN1466"
#define DEVICE_MODEL        "SD466NL/A"
#define FW_VERSION          "0.0.1"

homekit_characteristic_t name         = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  DEVICE_MANUFACTURER);
homekit_characteristic_t serial       = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model        = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision     = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_outlets,
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
            // Outlet 1
            HOMEKIT_SERVICE(OUTLET, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Outlet 1"),
                &outlet_on_1,
                HOMEKIT_CHARACTERISTIC(OUTLET_IN_USE, true),
                NULL
            }),
            // Outlet 2
            HOMEKIT_SERVICE(OUTLET, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Outlet 2"),
                &outlet_on_2,
                HOMEKIT_CHARACTERISTIC(OUTLET_IN_USE, true),
                NULL
            }),
            // Outlet 3
            HOMEKIT_SERVICE(OUTLET, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Outlet 3"),
                &outlet_on_3,
                HOMEKIT_CHARACTERISTIC(OUTLET_IN_USE, true),
                NULL
            }),
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

void on_wifi_ready(void) {
    ESP_LOGI("INFORMATION", "Starting HomeKit server...");
    homekit_server_init(&config);
}

// ====================
// Main Entry Point
// ====================
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
    button_init(); // <-- replaced old button library calls

    // No longer calling button_create(...) from <button.h>.
    // Our new button_init() and button_task() do the same job.
}
