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
#include <led_strip.h>
#include <sht3x.h>
#include <driver/i2c.h>

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

// LED Strip setup
#define LED_STRIP_GPIO CONFIG_ESP_LED_GPIO
#define LED_STRIP_LENGTH 1

// LED control
static led_strip_handle_t led_strip = NULL;

static bool led_on = false;

static void led_write(bool on) {
        if (led_strip) {
                int r = 0, g = 0, b = 0;
                if (on) {
                        r = 0;
                        g = 0;
                        b = 255; // Blue color
                }
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, r, g, b));
                ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
}

// Fan control
#define FAN_GPIO 33
static bool fan_on = false;

static void fan_write(bool on) {
        gpio_set_level(FAN_GPIO, on ? 1 : 0);
}

// All GPIO Settings
static void led_strip_init(void) {
        led_strip_config_t strip_config = {
                .strip_gpio_num = LED_STRIP_GPIO,
                .max_leds = LED_STRIP_LENGTH,
                .led_model = LED_MODEL_WS2812,
                .flags = {
                        .invert_out = false,
                },
        };

        led_strip_rmt_config_t rmt_config = {
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = 10 * 1000 * 1000,
        };

        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

static void fan_init(void) {
        gpio_set_direction(FAN_GPIO, GPIO_MODE_OUTPUT);
        fan_write(fan_on);
}

// Accessory identification
void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                led_write(true);
                vTaskDelay(pdMS_TO_TICKS(100));
                led_write(false);
                vTaskDelay(pdMS_TO_TICKS(100));
        }
        led_write(led_on);
        vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
        ESP_LOGI("INFORMATION", "Accessory identify");
        xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

homekit_value_t fan_on_get() {
        return HOMEKIT_BOOL(fan_on);
}

void fan_on_set(homekit_value_t value) {
        if (value.format != homekit_format_bool) {
                ESP_LOGE("ERROR", "Invalid value format: %d", value.format);
                return;
        }
        fan_on = value.bool_value;
        fan_write(fan_on);
}

// Temperature and Humidity Sensor
#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

static float temperature = 0.0f, humidity = 0.0f;

homekit_value_t temperature_get() {
        return HOMEKIT_FLOAT(temperature);
}

homekit_value_t humidity_get() {
        return HOMEKIT_FLOAT(humidity);
}

homekit_characteristic_t current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0, .getter = temperature_get);
homekit_characteristic_t current_humidity = HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 0, .getter = humidity_get);

void i2c_master_init(void) {
        i2c_config_t conf = {
                .mode = I2C_MODE_MASTER,
                .sda_io_num = I2C_MASTER_SDA_IO,
                .sda_pullup_en = GPIO_PULLUP_ENABLE,
                .scl_io_num = I2C_MASTER_SCL_IO,
                .scl_pullup_en = GPIO_PULLUP_ENABLE,
                .master.clk_speed = I2C_MASTER_FREQ_HZ,
        };
        i2c_param_config(SHT3X_I2C_PORT, &conf);
        i2c_driver_install(SHT3X_I2C_PORT, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void temperature_humidity_task(void *pvParameters) {
        uint8_t sht3x_address = CONFIG_SHT3X_I2C_ADDRESS;

        if (sht3x_init(sht3x_address) == ESP_OK) {
                ESP_LOGI("INFORMATION", "SHT3X sensor initialized successfully");
        } else {
                ESP_LOGE("ERROR", "Failed to initialize SHT3X sensor");
                vTaskDelete(NULL);
        }

        while (1) {
                if (sht3x_read_temperature_humidity(sht3x_address, &temperature, &humidity) == ESP_OK) {
                        ESP_LOGI("INFORMATION", "Temperature: %.2f C, Humidity: %.2f %%RH", temperature, humidity);

                        // Notify HomeKit about the temperature and humidity update
                        homekit_characteristic_notify(&current_temperature, HOMEKIT_FLOAT(temperature));
                        homekit_characteristic_notify(&current_humidity, HOMEKIT_FLOAT(humidity));
                } else {
                        ESP_LOGE("ERROR", "Failed to read data from SHT3X sensor");
                }
                vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
}

// HomeKit characteristics
#define DEVICE_NAME "HomeKit Fan"
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
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_fans, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(FAN, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Fan"),
                        HOMEKIT_CHARACTERISTIC(ON, false, .getter = fan_on_get, .setter = fan_on_set),
                        NULL
                }),
                HOMEKIT_SERVICE(TEMPERATURE_SENSOR, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "Temperature Sensor"),
                        &current_temperature,
                        NULL
                }),
                HOMEKIT_SERVICE(HUMIDITY_SENSOR, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "Humidity Sensor"),
                        &current_humidity,
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

void on_wifi_ready() {
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
        led_strip_init();
        fan_init();
        i2c_master_init();
        xTaskCreate(temperature_humidity_task, "Temperature and Humidity Task", 4096, NULL, 5, NULL);
}
