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

// LED Strip setup
#define LED_STRIP_GPIO CONFIG_ESP_LED_GPIO
#define LED_STRIP_LENGTH CONFIG_ESP_STRIP_LENGTH

// LED control
led_strip_handle_t led_strip;

bool led_on = false;
float led_brightness = 50;
float led_hue = 180;              // hue is scaled 0 to 360
float led_saturation = 50;      // saturation is scaled 0 to 100

// Function to convert HSI to RGBW
// https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
#define DEG_TO_RAD(X) (M_PI*(X)/180)

void hsi2rgbw(float H, float S, float I, int* rgbw) {
        int r, g, b, w;
        float cos_h, cos_1047_h;
        H = fmod(H, 360); // cycle H around to 0-360 degrees
        H = M_PI * H / 180; // Convert to radians.
        S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
        I = I > 0 ? (I < 1 ? I : 1) : 0;

        if (H < 2.09439) {
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                r = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                g = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                b = 0;
                w = 255 * (1 - S) * I;
        } else if (H < 4.188787) {
                H = H - 2.09439;
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                g = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                b = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                r = 0;
                w = 255 * (1 - S) * I;
        } else {
                H = H - 4.188787;
                cos_h = cos(H);
                cos_1047_h = cos(1.047196667 - H);
                b = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
                r = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
                g = 0;
                w = 255 * (1 - S) * I;
        }

        rgbw[0] = r;
        rgbw[1] = g;
        rgbw[2] = b;
        rgbw[3] = w;
}

void led_write(bool on) {
        if (led_strip) {
                if (on) {
                        for (int i = 0; i < LED_STRIP_LENGTH; i++) {
                                // Initialize variables
                                float h = led_hue;
                                float s = led_saturation / 100.0;
                                float v = led_brightness / 100.0;

                                int rgbw[4];
                                hsi2rgbw(h, s, v, rgbw);

                                uint8_t red = rgbw[0];
                                uint8_t green = rgbw[1];
                                uint8_t blue = rgbw[2];

                                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
                        }
                } else {
                        // Turn off all LEDs
                        for (int i = 0; i < LED_STRIP_LENGTH; i++) {
                                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 0, 0, 0));
                        }
                }
                ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
}

// All GPIO Settings
static void led_strip_init() {
        led_strip_config_t strip_config = {
                .strip_gpio_num = LED_STRIP_GPIO,
                .max_leds = LED_STRIP_LENGTH,
                .led_pixel_format = LED_PIXEL_FORMAT_GRB,
                .led_model = LED_MODEL_WS2812,
                .flags.invert_out = false,
        };

        led_strip_rmt_config_t rmt_config = {
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = 10 * 1000 * 1000,
                .flags.with_dma = false,
        };

        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

// Accessory identification
void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 2; j++) {
                        led_write(true);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        led_write(false);
                        vTaskDelay(pdMS_TO_TICKS(100));
                }
                vTaskDelay(pdMS_TO_TICKS(250));
        }
        led_write(false);
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
                return;
        }

        led_on = value.bool_value;
        led_write(led_on);
}

homekit_value_t led_brightness_get() {
        return HOMEKIT_INT(led_brightness);
}

void led_brightness_set(homekit_value_t value) {
        if (value.format != homekit_format_int) {
                return;
        }
        led_brightness = value.int_value;
        led_write(led_on);
}

homekit_value_t led_hue_get() {
        return HOMEKIT_FLOAT(led_hue);
}

void led_hue_set(homekit_value_t value) {
        if (value.format != homekit_format_float) {
                return;
        }
        led_hue = value.float_value;
        led_write(led_on);
}

homekit_value_t led_saturation_get() {
        return HOMEKIT_FLOAT(led_saturation);
}

void led_saturation_set(homekit_value_t value) {
        if (value.format != homekit_format_float) {
                return;
        }
        led_saturation = value.float_value;
        led_write(led_on);
}

#define DEVICE_NAME "HomeKit Light"
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
        led_strip_init();
}
