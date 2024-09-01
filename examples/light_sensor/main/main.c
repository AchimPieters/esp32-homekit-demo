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
 #include <bh1750.h>
 #include <string.h>
 #include <i2cdev.h>

// Custom error handling macro
 #define CHECK_ERROR(x) do {                            \
                esp_err_t __err_rc = (x);                          \
                if (__err_rc != ESP_OK) {                          \
                        ESP_LOGE("INFORMATION", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);                        \
                }                                                  \
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

 #if defined(CONFIG_ESP_I2C_ADDRESS_LO)
 #define ADDR BH1750_ADDR_LO
 #elif defined(CONFIG_ESP_I2C_ADDRESS_HI)
 #define ADDR BH1750_ADDR_HI
 #endif

 #define I2C_SCL_PIN CONFIG_ESP_I2C_MASTER_SCL
 #define I2C_SDA_PIN CONFIG_ESP_I2C_MASTER_SDA

 #define LED_GPIO CONFIG_ESP_LED_GPIO
static bool led_on = false;

static void led_write(bool on) {
        gpio_set_level(LED_GPIO, on ? 1 : 0);
}

static void gpio_init() {
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
        led_write(led_on);
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

static homekit_characteristic_t currentAmbientLightLevel = HOMEKIT_CHARACTERISTIC_(CURRENT_AMBIENT_LIGHT_LEVEL, 0);

static void light_sensor_task(void *_args) {
        i2c_dev_t dev;
        memset(&dev, 0, sizeof(i2c_dev_t)); // Zero descriptor

        CHECK_ERROR(bh1750_init_desc(&dev, ADDR, 0, I2C_SDA_PIN, I2C_SCL_PIN));
        CHECK_ERROR(bh1750_setup(&dev, BH1750_MODE_CONTINUOUS, BH1750_RES_HIGH));

        while (1) {
                uint16_t lux;
                if (bh1750_read(&dev, &lux) != ESP_OK) {
                        ESP_LOGE("ERROR", "Could not read lux data");
                } else {
                        ESP_LOGI("INFORMATION", "Lux: %d", lux);
                        currentAmbientLightLevel.value.float_value = (float)lux;
                        homekit_characteristic_notify(&currentAmbientLightLevel, HOMEKIT_FLOAT((float)lux));
                }
                vTaskDelay(pdMS_TO_TICKS(3000));
        }
}

static void light_sensor_init() {
        xTaskCreate(light_sensor_task, "Light Sensor", 2024, NULL, 2, NULL);
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
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
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
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_LOGW("WARNING", "NVS flash initialization failed, erasing...");
                CHECK_ERROR(nvs_flash_erase());
                ret = nvs_flash_init();
        }
        CHECK_ERROR(ret);

        wifi_init();
        gpio_init();
        CHECK_ERROR(i2cdev_init());
        light_sensor_init();
}
