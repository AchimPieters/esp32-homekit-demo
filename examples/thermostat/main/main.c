#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <dht.h>

// Custom error handling macro
#define CHECK_ERROR(x) do {                        \
                esp_err_t __err_rc = (x);                  \
                if (__err_rc != ESP_OK) {                  \
                        ESP_LOGE("ERROR", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);                \
                }                                          \
} while(0)

static void handle_error(esp_err_t err) {
        // Custom error handling logic
        switch (err) {
        case ESP_ERR_WIFI_NOT_STARTED:
        case ESP_ERR_WIFI_CONN:
                ESP_LOGI("INFORMATION", "Restarting WiFi...");
                esp_wifi_stop();
                esp_wifi_start();
                break;
        default:
                ESP_LOGE("ERROR", "Critical error, restarting device...");
                esp_restart();
                break;
        }
}

static void on_wifi_ready();

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

#if defined(CONFIG_EXAMPLE_TYPE_DHT11)
#define SENSOR_TYPE DHT_TYPE_DHT11
#endif
#if defined(CONFIG_EXAMPLE_TYPE_AM2301)
#define SENSOR_TYPE DHT_TYPE_AM2301
#endif
#if defined(CONFIG_EXAMPLE_TYPE_SI7021)
#define SENSOR_TYPE DHT_TYPE_SI7021
#endif
#define TEMP_SENSOR_GPIO CONFIG_ESP_TEMP_SENSOR_GPIO

// GPIO control
#define LED_GPIO CONFIG_ESP_LED_GPIO
static bool led_on = false;
#define FAN_GPIO CONFIG_ESP_FAN_GPIO
static bool fan_on = false;
#define COOLER_GPIO CONFIG_ESP_COOLER_GPIO
static bool cooler_on = false;
#define HEATER_GPIO CONFIG_ESP_HEATER_GPIO
static bool heater_on = false;

#define TEMPERATURE_POLL_PERIOD 10000
#define HEATER_FAN_DELAY 3000
#define COOLER_FAN_DELAY 0

static void led_write(bool on) {
        gpio_set_level(LED_GPIO, on ? 1 : 0);
}
static void heater_write(bool on) {
        gpio_set_level(HEATER_GPIO, on ? 1 : 0);
}
static void cooler_write(bool on) {
        gpio_set_level(COOLER_GPIO, on ? 1 : 0);
}
static void fan_write(bool on) {
        gpio_set_level(FAN_GPIO, on ? 1 : 0);
}

// All GPIO Settings
static void gpio_init() {
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
        led_write(led_on);
        gpio_set_direction(FAN_GPIO, GPIO_MODE_OUTPUT);
        fan_write(fan_on);
        gpio_set_direction(HEATER_GPIO, GPIO_MODE_OUTPUT);
        heater_write(heater_on);
        gpio_set_direction(COOLER_GPIO, GPIO_MODE_OUTPUT);
        cooler_write(cooler_on);
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

static void update_state();

static void on_update(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
        update_state();
}

homekit_characteristic_t current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);
homekit_characteristic_t target_temperature  = HOMEKIT_CHARACTERISTIC_(TARGET_TEMPERATURE, 20, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t units = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);
homekit_characteristic_t current_state = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATING_COOLING_STATE, 0);
homekit_characteristic_t target_state = HOMEKIT_CHARACTERISTIC_(TARGET_HEATING_COOLING_STATE, 0, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t cooling_threshold = HOMEKIT_CHARACTERISTIC_(COOLING_THRESHOLD_TEMPERATURE, 25, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t heating_threshold = HOMEKIT_CHARACTERISTIC_(HEATING_THRESHOLD_TEMPERATURE, 15, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t current_humidity = HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 0);

static void update_state() {
        uint8_t state = target_state.value.int_value;
        if ((state == 1 && current_temperature.value.float_value < target_temperature.value.float_value) ||
            (state == 3 && current_temperature.value.float_value < heating_threshold.value.float_value)) {
                if (current_state.value.int_value != 1) {
                        current_state.value = HOMEKIT_UINT8(1);
                        homekit_characteristic_notify(&current_state, current_state.value);
                        heater_write(true);
                        cooler_write(false);
                        vTaskDelay(pdMS_TO_TICKS(HEATER_FAN_DELAY));
                        fan_write(true);
                        ESP_LOGI("INFORMATION", "Heater On");

                }
        } else if ((state == 2 && current_temperature.value.float_value > target_temperature.value.float_value) ||
                   (state == 3 && current_temperature.value.float_value > cooling_threshold.value.float_value)) {
                if (current_state.value.int_value != 2) {
                        current_state.value = HOMEKIT_UINT8(2);
                        homekit_characteristic_notify(&current_state, current_state.value);
                        cooler_write(true);
                        heater_write(false);
                        vTaskDelay(pdMS_TO_TICKS(COOLER_FAN_DELAY));
                        fan_write(true);
                        ESP_LOGI("INFORMATION", "Cooler On");
                }
        } else {
                if (current_state.value.int_value != 0) {
                        current_state.value = HOMEKIT_UINT8(0);
                        homekit_characteristic_notify(&current_state, current_state.value);
                        cooler_write(false);
                        heater_write(false);
                        fan_write(false);
                        ESP_LOGI("INFORMATION", "Off");
                }
        }
}

static void temperature_sensor_task(void *pvParameters) {

    #ifdef CONFIG_EXAMPLE_INTERNAL_PULLUP
        gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
    #endif

        float temperature_value, humidity_value;

        while (1) {
                if (dht_read_float_data(SENSOR_TYPE, CONFIG_ESP_TEMP_SENSOR_GPIO, &humidity_value, &temperature_value) == ESP_OK) {
                        ESP_LOGI("INFORMATION", "Humidity: %.1f%% Temperature: %.1fC", humidity_value, temperature_value);

                        current_temperature.value = HOMEKIT_FLOAT(temperature_value);
                        current_humidity.value = HOMEKIT_FLOAT(humidity_value);

                        homekit_characteristic_notify(&current_temperature, current_temperature.value);
                        homekit_characteristic_notify(&current_humidity, current_humidity.value);
                } else {
                        ESP_LOGE("ERROR", "Can not read data from sensor");

                }
                vTaskDelay(pdMS_TO_TICKS(2000));
        }
}

static void temperature_sensor_init() {
        xTaskCreate(temperature_sensor_task, "read data from sensor", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}

// HomeKit characteristics
#define DEVICE_NAME "HomeKit Thermostat"
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
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_thermostats, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(THERMOSTAT, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Thermostat"),
                        &current_temperature,
                        &target_temperature,
                        &current_state,
                        &target_state,
                        &cooling_threshold,
                        &heating_threshold,
                        &units,
                        &current_humidity,
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
        temperature_sensor_init();
}
