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
#include "lcm_api.h"
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

// LED control
#define BUTTON_GPIO CONFIG_ESP_BUTTON_GPIO
#define LED_GPIO CONFIG_ESP_LED_GPIO
#define RELAY_GPIO CONFIG_ESP_RELAY_GPIO

static bool led_on = false;

static void led_write(bool on) {
        gpio_set_level(LED_GPIO, on ? 1 : 0);
}

static void relay_write(bool on) {
        gpio_set_level(RELAY_GPIO, on ? 1 : 0);
}

void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);

homekit_characteristic_t switch_on = HOMEKIT_CHARACTERISTIC_(
        ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback)
        );

static void gpio_init() {
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
        led_write(led_on);

        gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
        relay_write(switch_on.value.bool_value);
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
        xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

static void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
        relay_write(switch_on.value.bool_value);
}

static void button_callback(button_event_t event, void *context) {
        switch (event) {
        case button_event_single_press:
                ESP_LOGI("SINGLE_PRESS", "single press");
                switch_on.value.bool_value = !switch_on.value.bool_value;
                relay_write(switch_on.value.bool_value);
                homekit_characteristic_notify(&switch_on, switch_on.value);
                break;
        case button_event_double_press:
                ESP_LOGI("DOUBLE_PRESS", "Double press");
                break;
        case button_event_long_press:
                ESP_LOGI("LONG_PRESS", "Long press");
                break;
        default:
                ESP_LOGI("UNKNOWN_BUTTON_EVENT", "unknown button event: %d", event);
        }
}

#define DEVICE_NAME "HomeKit Switch"
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
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_switches, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(SWITCH, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Switch"),
                        &switch_on,
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

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Woverride-init"
        button_config_t button_config = BUTTON_CONFIG(
                button_active_low,
                .max_repeat_presses=2,
                .long_press_time=1000,
                );
    #pragma GCC diagnostic pop

        if (button_create(BUTTON_GPIO, button_config, button_callback, NULL)) {
                ESP_LOGI("FAILED_TO_INITIALIZE_BUTTON", "Failed to initialize button");
        }
}

void app_main(void) {


        // The code in this function would be the setup for any app that uses Wi-Fi set by LCM
        esp_err_t err = nvs_flash_init(); // Initialize NVS
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_ERROR_CHECK(nvs_flash_erase()); // NVS partition truncated and must be erased
                err = nvs_flash_init(); // Retry nvs_flash_init
        }
        ESP_ERROR_CHECK(err);

        // Initialize Wi-Fi with the lowest amount of effort, and based on FLASH
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
        esp_netif_config.route_prio = 128;
        esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
        esp_wifi_set_default_wifi_sta_handlers();
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH)); // Key difference
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_connect());

        // Create the main task
        xTaskCreate(main_task, "main", 4096, NULL, 1, NULL);

        // Keep the main function running indefinitely
        while (true) {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}
