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
#include <driver/ledc.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <led_strip.h>
#include <led_strip_rmt.h>
#include <math.h>

// Custom error handling macro with specific error codes
#define CHECK_ERROR(x) do { \
                esp_err_t __err_rc = (x); \
                if (__err_rc != ESP_OK) { \
                        ESP_LOGE("INFORMATION", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc); \
                } \
} while(0)

static void handle_error(esp_err_t err) {
        switch(err) {
        case ESP_ERR_WIFI_NOT_STARTED:
        case ESP_ERR_WIFI_CONN:
                ESP_LOGI("INFORMATION", "Attempting to restart WiFi...");
                esp_wifi_stop();
                vTaskDelay(pdMS_TO_TICKS(1000)); // Adding delay before retry
                esp_wifi_start();
                break;
        default:
                ESP_LOGE("ERROR", "Critical error, restarting device...");
                esp_restart();
        }
}

// WiFi event handler and initialization
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
        if (event_base == WIFI_EVENT) {
                switch(event_id) {
                case WIFI_EVENT_STA_START:
                case WIFI_EVENT_STA_DISCONNECTED:
                        ESP_LOGI("INFORMATION", "Connecting to WiFi...");
                        esp_wifi_connect();
                        break;
                }
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

// LED strip configuration and initialization
#define LED_STRIP_GPIO CONFIG_ESP_LED_GPIO
#define LED_STRIP_LENGTH CONFIG_ESP_STRIP_LENGTH
#define GPIO_WARM_WHITE 18
#define GPIO_COLD_WHITE 19

static led_strip_handle_t led_strip;
static bool led_on = false;
static float led_brightness = 50;
static float led_hue = 180;               // hue is scaled 0 to 360
static float led_saturation = 50;         // saturation is scaled 0 to 100
static uint32_t color_temperature = 2700; // default color temperature in Kelvin

// Kelvin to Mirek conversion
static uint32_t kelvin_to_mirek(uint32_t kelvin) {
        return 1000000 / kelvin;
}

// Adjust LED temperature function
static void adjust_led_temperature(uint32_t kelvin) {
        uint32_t warm_duty = 0;
        uint32_t cold_duty = 0;

        if (kelvin <= 4000) {
                warm_duty = (4000 - kelvin) * 1023 / 1300;
        } else {
                cold_duty = (kelvin - 4000) * 1023 / 2500;
        }

        CHECK_ERROR(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, warm_duty));
        CHECK_ERROR(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, cold_duty));
        CHECK_ERROR(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0));
        CHECK_ERROR(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1));
}

// Convert HSI to RGBW
static void hsi2rgbw(float H, float S, float I, int *rgbw) {
        int r = 0, g = 0, b = 0, w = 0;
        float cos_h, cos_1047_h;
        H = fmod(H, 360) * M_PI / 180; // Convert to radians and normalize H
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

// Write to LED strip
static void led_write(bool on) {
        if (led_strip) {
                for (int i = 0; i < LED_STRIP_LENGTH; i++) {
                        int rgbw[4] = {0, 0, 0, 0};
                        if (on) {
                                hsi2rgbw(led_hue, led_saturation / 100.0, led_brightness / 100.0, rgbw);
                        }
                        CHECK_ERROR(led_strip_set_pixel_rgbw(led_strip, i, rgbw[0], rgbw[1], rgbw[2], rgbw[3]));
                }
                CHECK_ERROR(led_strip_refresh(led_strip));
        }
}

// Initialize the LED strip
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

// Initialize LEDC for warm and cold white control
static void ledc_init() {
        ledc_timer_config_t ledc_timer = {
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .duty_resolution = LEDC_TIMER_10_BIT,
                .timer_num = LEDC_TIMER_0,
                .freq_hz = 5000,
                .clk_cfg = LEDC_AUTO_CLK
        };
        CHECK_ERROR(ledc_timer_config(&ledc_timer));

        ledc_channel_config_t warm_channel = {
                .channel = LEDC_CHANNEL_0,
                .duty = 0,
                .gpio_num = GPIO_WARM_WHITE,
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .hpoint = 0,
                .timer_sel = LEDC_TIMER_0
        };
        CHECK_ERROR(ledc_channel_config(&warm_channel));

        ledc_channel_config_t cold_channel = {
                .channel = LEDC_CHANNEL_1,
                .duty = 0,
                .gpio_num = GPIO_COLD_WHITE,
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .hpoint = 0,
                .timer_sel = LEDC_TIMER_0
        };
        CHECK_ERROR(ledc_channel_config(&cold_channel));
}

// Accessory identification task
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

// HomeKit characteristics
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

static homekit_value_t color_temperature_get() {
        return HOMEKIT_UINT32(kelvin_to_mirek(color_temperature));
}

static void color_temperature_set(homekit_value_t value) {
        if (value.format == homekit_format_uint32) {
                uint32_t mirek_value = value.uint32_value;
                color_temperature = 1000000 / mirek_value;
                adjust_led_temperature(color_temperature);
        }
}

// Accessory configuration
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
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
                        HOMEKIT_CHARACTERISTIC(ON, true, .getter = led_on_get, .setter = led_on_set),
                        HOMEKIT_CHARACTERISTIC(BRIGHTNESS, 100, .getter = led_brightness_get, .setter = led_brightness_set),
                        HOMEKIT_CHARACTERISTIC(HUE, 0, .getter = led_hue_get, .setter = led_hue_set),
                        HOMEKIT_CHARACTERISTIC(SATURATION, 0, .getter = led_saturation_get, .setter = led_saturation_set),
                        HOMEKIT_CHARACTERISTIC(COLOR_TEMPERATURE, 140, .getter = color_temperature_get, .setter = color_temperature_set),
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

// Main function
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
        led_strip_init();
        ledc_init();
}
