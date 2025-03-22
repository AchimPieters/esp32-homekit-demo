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
#include <bh1750.h>
#include <string.h>

// Custom error handling macro
#define CHECK_ERROR(x) do { \
    esp_err_t __err_rc = (x); \
    if (__err_rc != ESP_OK) { \
        ESP_LOGE("ERROR", "Error: %s", esp_err_to_name(__err_rc)); \
        handle_error(__err_rc); \
    } \
} while(0)

static void handle_error(esp_err_t err) {
    if (err == ESP_ERR_WIFI_NOT_STARTED || err == ESP_ERR_WIFI_CONN) {
        ESP_LOGI("INFORMATION", "Restarting WiFi...");
        CHECK_ERROR(esp_wifi_stop());
        CHECK_ERROR(esp_wifi_start());
    } else {
        ESP_LOGE("ERROR", "Critical error, restarting device...");
        esp_restart();
    }
}

static void on_wifi_ready();

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        ESP_LOGI("INFORMATION", "Connecting to WiFi...");
        CHECK_ERROR(esp_wifi_connect());
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

#define I2C_SCL_PIN CONFIG_ESP_I2C_MASTER_SCL
#define I2C_SDA_PIN CONFIG_ESP_I2C_MASTER_SDA
#define I2C_ADDRESS (CONFIG_ESP_I2C_ADDRESS_LO ? BH1750_ADDR_LO : BH1750_ADDR_HI)

#define LED_GPIO CONFIG_ESP_LED_GPIO
static bool led_on = false;

static void led_write(bool on) {
    gpio_set_level(LED_GPIO, on ? 1 : 0);
}

static void gpio_init() {
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    led_write(led_on);
}

static homekit_characteristic_t currentAmbientLightLevel = HOMEKIT_CHARACTERISTIC_(CURRENT_AMBIENT_LIGHT_LEVEL, 0);

// BH1750 Sensor Variables
static i2c_dev_t bh1750_dev;

static void light_sensor_task(void *arg) {
    uint16_t lux_value;

    // Initialize BH1750 sensor with dynamic I2C address
    CHECK_ERROR(bh1750_init_desc(&bh1750_dev, I2C_ADDRESS, I2C_NUM_0, I2C_SDA_PIN, I2C_SCL_PIN));
    CHECK_ERROR(bh1750_power_on(&bh1750_dev));
    CHECK_ERROR(bh1750_setup(&bh1750_dev, BH1750_MODE_CONTINUOUS, BH1750_RES_HIGH));

    while (1) {
        if (bh1750_read(&bh1750_dev, &lux_value) == ESP_OK) {
            ESP_LOGI("SENSOR", "Light Intensity: %d lux", lux_value);
            currentAmbientLightLevel.value.float_value = lux_value;
            homekit_characteristic_notify(&currentAmbientLightLevel, currentAmbientLightLevel.value);
        } else {
            ESP_LOGE("SENSOR", "Failed to read light intensity.");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));  // Update every second
    }
}

static void light_sensor_init() {
    xTaskCreate(light_sensor_task, "Light Sensor", 4096, NULL, 2, NULL);
}

#define DEVICE_NAME "HomeKit Light Sensor"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/A"
#define FW_VERSION "0.0.1"

static homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
static homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, DEVICE_MANUFACTURER);
static homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
static homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
static homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
static homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_sensors, .services = (homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
            &name, &manufacturer, &serial, &model, &revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, NULL),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHT_SENSOR, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Light Sensor"),
            &currentAmbientLightLevel,
            NULL
        }),
        NULL
    }),
    NULL
};
#pragma GCC diagnostic pop

static homekit_server_config_t config = {
    .accessories = accessories,
    .password = CONFIG_ESP_SETUP_CODE,
    .setupId = CONFIG_ESP_SETUP_ID,
};

static void on_wifi_ready() {
    ESP_LOGI("INFORMATION", "Starting HomeKit server...");
    homekit_server_init(&config);
}

void app_main(void) {
    CHECK_ERROR(nvs_flash_init());
    wifi_init();
    gpio_init();
    light_sensor_init();
}
