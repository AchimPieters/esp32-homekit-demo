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
#include <math.h>

// WiFi setup
void on_wifi_ready();

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
        if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
                ESP_LOGI("WIFI_EVENT", "STA start");
                esp_wifi_connect();
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                ESP_LOGI("IP_EVENT", "WiFI ready");
                on_wifi_ready();
        }
}

static void wifi_init() {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        wifi_config_t wifi_config = {
                .sta = {
                        .ssid = CONFIG_ESP_WIFI_SSID,
                        .password = CONFIG_ESP_WIFI_PASSWORD,
                },
        };

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
}

// LED control
#define LED_GPIO CONFIG_ESP_LED_GPIO
bool led_on = false;
uint32_t led_brightness = 100;  // Default brightness

// PWM Settings
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_RESOLUTION     LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY      (5000)

// Initialize PWM
static void pwm_init() {
        ledc_timer_config_t ledc_timer = {
                .duty_resolution = LEDC_RESOLUTION,
                .freq_hz = LEDC_FREQUENCY,
                .speed_mode = LEDC_MODE,
                .timer_num = LEDC_TIMER,
        };
        ledc_timer_config(&ledc_timer);

        ledc_channel_config_t ledc_channel = {
                .channel    = LEDC_CHANNEL,
                .duty       = 0,
                .gpio_num   = LED_GPIO,
                .speed_mode = LEDC_MODE,
                .timer_sel  = LEDC_TIMER,
        };
        ledc_channel_config(&ledc_channel);
}

// Map the brightness linearly to the PWM duty cycle
uint32_t map_brightness(uint32_t brightness) {
        return (brightness * (1 << LEDC_RESOLUTION)) / 100; // Map 0-100 to the entire range of duty cycle
}

void led_write(bool on, uint32_t brightness) {
        uint32_t mapped_brightness = on ? map_brightness(brightness) : 0;
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, mapped_brightness);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

// All GPIO Settings
void gpio_init() {
        pwm_init(); // Initialize PWM for LED control
        led_write(led_on, led_brightness);
}

// Accessory identification
void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 2; j++) {
                        led_write(true, led_brightness);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        led_write(false, led_brightness);
                        vTaskDelay(pdMS_TO_TICKS(100));
                }
                vTaskDelay(pdMS_TO_TICKS(250));
        }
        led_write(led_on, led_brightness);
        vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
        ESP_LOGI("ACCESSORY_IDENTIFY", "Accessory identify");
        xTaskCreate(accessory_identify_task, "Accessory identify", 2048, NULL, 2, NULL);
}

// HomeKit characteristics
homekit_value_t led_on_get() {
        return HOMEKIT_BOOL(led_on);
}

void led_on_set(homekit_value_t value) {
        if (value.format != homekit_format_bool) {
                ESP_LOGE("led_on_set", "Invalid value format: %d", value.format);
                return;
        }
        led_on = value.bool_value;
        led_write(led_on, led_brightness);
}

homekit_value_t led_brightness_get() {
        return HOMEKIT_INT(led_brightness);
}

void led_brightness_set(homekit_value_t value) {
        if (value.format != homekit_format_int) {
                ESP_LOGE("led_brightness_set", "Invalid value format: %d", value.format);
                return;
        }
        led_brightness = value.int_value;
        led_write(led_on, led_brightness);
}

#define DEVICE_NAME "HomeKit Light"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/A"
#define FW_VERSION "0.0.1"

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  DEVICE_MANUFACTURER);
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_lighting, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Light"),
                        HOMEKIT_CHARACTERISTIC(
                                ON, false,
                                .getter = led_on_get,
                                .setter = led_on_set
                                ),
                        HOMEKIT_CHARACTERISTIC(
                                BRIGHTNESS, 100,
                                .getter = led_brightness_get,
                                .setter = led_brightness_set
                                ),
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
        homekit_server_init(&config);
}

void app_main(void) {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        wifi_init();
        gpio_init();
}
