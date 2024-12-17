#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <led_strip.h>
#include <sht3x.h>
#include <driver/i2c.h>
#include <button.h>

// Custom error handling macro
#define CHECK_ERROR(x) do {                                \
                esp_err_t __err_rc = (x);                  \
                if (__err_rc != ESP_OK) {                  \
                        ESP_LOGE("INFORMATION", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);            \
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
static bool manual_override = false;
static bool homekit_active = false;

static void led_write(bool on, int r, int g, int b) {
        if (led_strip) {
                if (on) {
                        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, r, g, b));
                } else {
                        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 0, 0));
                }
                ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
}

// Fan control
#define FAN_GPIO 33
static float fan_speed = 0.0f;
static bool fan_on = false;

static void fan_write(float speed) {
        uint32_t duty = (uint32_t)(speed * 2.55f); // Convert percentage (0-100) to duty (0-255)
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
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
        ledc_timer_config_t ledc_timer = {
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .timer_num = LEDC_TIMER_0,
                .duty_resolution = LEDC_TIMER_8_BIT,
                .freq_hz = 25000,
                .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&ledc_timer);

        ledc_channel_config_t ledc_channel = {
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .channel = LEDC_CHANNEL_0,
                .timer_sel = LEDC_TIMER_0,
                .intr_type = LEDC_INTR_DISABLE,
                .gpio_num = FAN_GPIO,
                .duty = 0,
                .hpoint = 0
        };
        ledc_channel_config(&ledc_channel);
}

// Accessory identification
void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                led_write(true, 0, 0, 255);
                vTaskDelay(pdMS_TO_TICKS(100));
                led_write(false, 0, 0, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
        }
        led_write(led_on, 0, 0, 255);
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
        manual_override = fan_on;
        homekit_active = fan_on;
        fan_write(fan_on ? fan_speed : 0.0f);
        if (fan_on) {
                led_write(true, 128, 0, 128); // Purple for HomeKit control
        } else {
                led_write(false, 0, 0, 0);
                manual_override = false;
                homekit_active = false;
        }
}

homekit_value_t fan_speed_get() {
        return HOMEKIT_FLOAT(fan_speed);
}

void fan_speed_set(homekit_value_t value) {
        if (value.format != homekit_format_float) {
                ESP_LOGE("ERROR", "Invalid value format: %d", value.format);
                return;
        }
        fan_speed = value.float_value;
        if (fan_speed < 0.0f) fan_speed = 0.0f;
        if (fan_speed > 100.0f) fan_speed = 100.0f;
        fan_write(fan_on ? fan_speed : 0.0f);
        manual_override = true;
        homekit_active = true;
        led_write(true, 128, 0, 128); // Purple for HomeKit control
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

                        // Fan control based on temperature if not in manual override mode
                        if (!manual_override && !homekit_active) {
                                if (temperature > 60.0f) {
                                        fan_speed = 20.0f;
                                        led_write(true, 0, 255, 0); // Green LED
                                } else if (temperature > 50.0f) {
                                        fan_speed = 40.0f;
                                        led_write(true, 255, 255, 0); // Yellow LED
                                } else if (temperature > 40.0f) {
                                        fan_speed = 60.0f;
                                        led_write(true, 255, 165, 0); // Orange LED
                                } else if (temperature > 30.0f) {
                                        fan_speed = 80.0f;
                                        led_write(true, 255, 0, 0); // Red LED
                                } else if (temperature > 25.0f) {
                                        fan_speed = 10.0f;
                                        led_write(false, 0, 0, 0); // Turn off LED
                                } else {
                                        fan_speed = 0.0f;
                                        led_write(false, 0, 0, 0); // Turn off LED
                                }
                                fan_write(fan_speed);
                                fan_on = (fan_speed > 0.0f);
                        }

                } else {
                        ESP_LOGE("ERROR", "Failed to read data from SHT3X sensor");
                }
                vTaskDelay(300000 / portTICK_PERIOD_MS); // Update every 5 minutes as recommended for HomeKit temperature sensor
        }
}

#define BUTTON_GPIO CONFIG_ESP_BUTTON_GPIO

void button_callback(button_event_t event, void *context) {
        if (event == button_event_single_press) {
                if (homekit_active) {
                        homekit_active = false;
                        manual_override = false;
                        fan_on = false;
                        fan_write(0.0f);
                        led_write(false, 0, 0, 0); // Turn off LED
                } else {
                        fan_speed += 20.0f;
                        if (fan_speed > 100.0f) {
                                fan_speed = 0.0f;
                                fan_on = false;
                                fan_write(0.0f);
                                led_write(false, 0, 0, 0);
                                manual_override = false;
                        } else {
                                fan_on = true;
                                fan_write(fan_speed);
                                manual_override = true;

                                if (fan_speed == 20.0f) {
                                        led_write(true, 0, 255, 0); // Green
                                } else if (fan_speed == 40.0f) {
                                        led_write(true, 255, 255, 0); // Yellow
                                } else if (fan_speed == 60.0f) {
                                        led_write(true, 255, 165, 0); // Orange
                                } else if (fan_speed == 80.0f) {
                                        led_write(true, 255, 0, 0); // Red
                                } else if (fan_speed == 100.0f) {
                                        led_write(true, 0, 0, 255); // Blue (Max speed)
                                }
                        }
                }
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
                        HOMEKIT_CHARACTERISTIC(ROTATION_SPEED, 0.0f, .getter = fan_speed_get, .setter = fan_speed_set),
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

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Woverride-init"
        button_config_t button_config = BUTTON_CONFIG(
                button_active_high,
                .max_repeat_presses=1,
                .long_press_time=1000,
                );
        #pragma GCC diagnostic pop

        if (button_create(BUTTON_GPIO, button_config, button_callback, NULL)) {
                ESP_LOGE("INITIALIZE_BUTTON", "Failed to initialize button");
        }
}
