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
#include <led_strip_rmt.h>
#include <math.h>

// Macros and Definitions
#define CHECK_ERROR(x) do { \
                esp_err_t __err_rc = (x); \
                if (__err_rc != ESP_OK) { \
                        ESP_LOGE("ERROR", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc); \
                } \
} while(0)

#define LED_STRIP_GPIO CONFIG_ESP_LED_GPIO
#define LED_STRIP_LENGTH CONFIG_ESP_STRIP_LENGTH
#define DEG_TO_RAD(X) (M_PI*(X)/180)

// Static Declarations
static led_strip_handle_t led_strip;
static bool led_on = false;
static float led_brightness = 50;
static float led_hue = 180; // 0 to 360
static float led_saturation = 50; // 0 to 100

// Error Handling
static void handle_error(esp_err_t err) {
        if (err == ESP_ERR_WIFI_NOT_STARTED || err == ESP_ERR_WIFI_CONN) {
                ESP_LOGI("INFORMATION", "Restarting WiFi...");
                esp_wifi_stop();
                esp_wifi_start();
        } else {
                ESP_LOGE("ERROR", "Critical error, restarting device...");
                esp_restart();
        }
}

// Event Handler
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
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

// WiFi Initialization
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

// LED Control
static void hsi2rgbw(float H, float S, float I, int* rgbw) {
        int r = 0, g = 0, b = 0, w = 0;
        float cos_h, cos_1047_h;

        H = fmod(H, 360); // Cycle H around to 0-360 degrees
        H = DEG_TO_RAD(H); // Convert to radians.
        S = fminf(fmaxf(S, 0), 1); // Clamp S and I to interval [0,1]
        I = fminf(fmaxf(I, 0), 1);

        if (H < 2.09439) {
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                r = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                g = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                w = 255 * (1 - S) * I;
        } else if (H < 4.188787) {
                H -= 2.09439;
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                g = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                b = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                w = 255 * (1 - S) * I;
        } else {
                H -= 4.188787;
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                b = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                r = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                w = 255 * (1 - S) * I;
        }

        rgbw[0] = r;
        rgbw[1] = g;
        rgbw[2] = b;
        rgbw[3] = w;
}

static void led_write(bool on) {
        if (led_strip) {
                for (int i = 0; i < LED_STRIP_LENGTH; i++) {
                        int rgbw[4] = {0};
                        if (on) {
                                hsi2rgbw(led_hue, led_saturation / 100.0, led_brightness / 100.0, rgbw);
                        }
                        ESP_ERROR_CHECK(led_strip_set_pixel_rgbw(led_strip, i, rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
                }
                ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
}

static void led_strip_init() {
        led_strip_config_t strip_config = {
                .strip_gpio_num = LED_STRIP_GPIO,
                .max_leds = LED_STRIP_LENGTH,
                .led_pixel_format = LED_PIXEL_FORMAT_GRBW,
                .led_model = LED_MODEL_SK6812,
                .flags.invert_out = false,
        };

        led_strip_rmt_config_t rmt_config = {
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = 10 * 1000 * 1000,
                .flags.with_dma = false,
        };

        CHECK_ERROR(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

// Accessory Identification
static void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                led_write(true);
                vTaskDelay(pdMS_TO_TICKS(100));
                led_write(false);
                vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(250));
        led_write(led_on);
        vTaskDelete(NULL);
}

static void accessory_identify(homekit_value_t _value) {
        ESP_LOGI("INFORMATION", "Accessory identify");
        xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// HomeKit Characteristics
static homekit_value_t led_on_get() {
        return HOMEKIT_BOOL(led_on);
}

static void led_on_set(homekit_value_t value) {
        if (value.format == homekit_format_bool) {
                led_on = value.bool_value;
                led_write(led_on);
        }
}

static homekit_value_t led_brightness_get() {
        return HOMEKIT_INT(led_brightness);
}

static void led_brightness_set(homekit_value_t value) {
        if (value.format == homekit_format_int) {
                led_brightness = value.int_value;
                led_write(led_on);
        }
}

static homekit_value_t led_hue_get() {
        return HOMEKIT_FLOAT(led_hue);
}

static void led_hue_set(homekit_value_t value) {
        if (value.format == homekit_format_float) {
                led_hue = value.float_value;
                led_write(led_on);
        }
}

static homekit_value_t led_saturation_get() {
        return HOMEKIT_FLOAT(led_saturation);
}

static void led_saturation_set(homekit_value_t value) {
        if (value.format == homekit_format_float) {
                led_saturation = value.float_value;
                led_write(led_on);
        }
}

// HomeKit Accessory Configuration
#define DEVICE_NAME "HomeKit RGBW Light"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/A"
#define FW_VERSION "0.0.1"

static homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
static homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, DEVICE_MANUFACTURER);
static homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
static homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
static homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

static homekit_accessory_t *accessories[] = {
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
                        HOMEKIT_CHARACTERISTIC(NAME, "HomeKit RGBW Light"),
                        HOMEKIT_CHARACTERISTIC(
                                ON, true,
                                .getter = led_on_get,
                                .setter = led_on_set
                                ),
                        HOMEKIT_CHARACTERISTIC(
                                BRIGHTNESS, 100,
                                .getter = led_brightness_get,
                                .setter = led_brightness_set
                                ),
                        HOMEKIT_CHARACTERISTIC(
                                HUE, 0,
                                .getter = led_hue_get,
                                .setter = led_hue_set
                                ),
                        HOMEKIT_CHARACTERISTIC(
                                SATURATION, 0,
                                .getter = led_saturation_get,
                                .setter = led_saturation_set
                                ),
                        NULL
                }),
                NULL
        }),
        NULL
};

static homekit_server_config_t config = {
        .accessories = accessories,
        .password = CONFIG_ESP_SETUP_CODE,
        .setupId = CONFIG_ESP_SETUP_ID,
};

// WiFi Ready Handler
static void on_wifi_ready() {
        ESP_LOGI("INFORMATION", "Starting HomeKit server...");
        homekit_server_init(&config);
}

// Main Application
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
}
