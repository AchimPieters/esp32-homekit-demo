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
#include <math.h>
#include <ws2811.h> // Include the esp32-ws2811 library

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

#define LED_STRIP_GPIO CONFIG_ESP_LED_GPIO
#define LED_STRIP_LENGTH CONFIG_ESP_STRIP_LENGTH
#define COLD_WHITE_GPIO CONFIG_ESP_COLD_WHITE_GPIO
#define WARM_WHITE_GPIO CONFIG_ESP_WARM_WHITE_GPIO

#define LEDC_COLD_WHITE_CHANNEL LEDC_CHANNEL_0
#define LEDC_WARM_WHITE_CHANNEL LEDC_CHANNEL_1
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY 5000 // 5 kHz PWM frequency

static const char *TAG = "WS2811";
led_strip_t *led_strip;

bool led_on = false;
float led_brightness = 50;
float led_hue = 180; // hue is scaled 0 to 360
float led_saturation = 50; // saturation is scaled 0 to 100
int led_color_temperature = 140; // Color temperature in mirek (6500K = 153, 2700K = 370)

#define DEG_TO_RAD(X) (M_PI * (X) / 180)

void hsi2rgbw(float H, float S, float I, int *rgbw) {
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

void set_white_leds(int mirek) {
    int temperature = 1000000 / mirek; // Convert mirek to Kelvin
    int cold_white_duty = 0;
    int warm_white_duty = 0;

    if (temperature < 2700) {
        temperature = 2700;
    } else if (temperature > 6500) {
        temperature = 6500;
    }

    if (temperature <= 4000) {
        warm_white_duty = (int)(8191 * (led_brightness / 100.0));
        cold_white_duty = warm_white_duty * (temperature - 2700) / 1300;
    } else {
        cold_white_duty = (int)(8191 * (led_brightness / 100.0));
        warm_white_duty = cold_white_duty * (6500 - temperature) / 2500;
    }

    ledc_set_duty(LEDC_MODE, LEDC_COLD_WHITE_CHANNEL, cold_white_duty);
    ledc_update_duty(LEDC_MODE, LEDC_COLD_WHITE_CHANNEL);
    ledc_set_duty(LEDC_MODE, LEDC_WARM_WHITE_CHANNEL, warm_white_duty);
    ledc_update_duty(LEDC_MODE, LEDC_WARM_WHITE_CHANNEL);
}

void led_write(bool on) {
    if (on) {
        if (led_saturation == 0) { // Check if the selected color is white (saturation is 0)
            set_white_leds(led_color_temperature);
            for (int i = 0; i < LED_STRIP_LENGTH; i++) {
                led_strip_set_pixel(led_strip, i, 0, 0, 0); // Turn off RGB LEDs
            }
        } else {
            // Ensure white LEDs are off when color other than white is selected
            ledc_set_duty(LEDC_MODE, LEDC_COLD_WHITE_CHANNEL, 0);
            ledc_update_duty(LEDC_MODE, LEDC_COLD_WHITE_CHANNEL);
            ledc_set_duty(LEDC_MODE, LEDC_WARM_WHITE_CHANNEL, 0);
            ledc_update_duty(LEDC_MODE, LEDC_WARM_WHITE_CHANNEL);

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

                led_strip_set_pixel(led_strip, i, blue, red, green);
            }
        }
    } else {
        // Turn off all LEDs
        for (int i = 0; i < LED_STRIP_LENGTH; i++) {
            led_strip_set_pixel(led_strip, i, 0, 0, 0);
        }
        ledc_set_duty(LEDC_MODE, LEDC_COLD_WHITE_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_COLD_WHITE_CHANNEL);
        ledc_set_duty(LEDC_MODE, LEDC_WARM_WHITE_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_WARM_WHITE_CHANNEL);
    }
    led_strip_refresh(led_strip);
}

static void init_led_strip() {
    rmt_channel_t channel = RMT_CHANNEL_0;
    gpio_num_t gpio = LED_STRIP_GPIO;
    uint16_t length = LED_STRIP_LENGTH;

    led_strip = led_strip_init(channel, gpio, length);
    assert(led_strip != NULL);
}

static void init_white_leds() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel_cold_white = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_COLD_WHITE_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = COLD_WHITE_GPIO,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_cold_white);

    ledc_channel_config_t ledc_channel_warm_white = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_WARM_WHITE_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = WARM_WHITE_GPIO,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_warm_white);
}

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
    ESP_LOGI(TAG, "Accessory identify");
    xTaskCreate(accessory_identify_task, "Accessory identify", 2048, NULL, 2, NULL);
}

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

homekit_value_t led_color_temperature_get() {
    return HOMEKIT_UINT32(led_color_temperature);
}

void led_color_temperature_set(homekit_value_t value) {
    if (value.format != homekit_format_uint32) {
        return;
    }
    led_color_temperature = value.uint32_value;
    led_write(led_on);
}

#define DEVICE_NAME "HomeKit RGBIC Light"
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
            HOMEKIT_CHARACTERISTIC(NAME, "HomeKit RGBIC Light"),
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
            HOMEKIT_CHARACTERISTIC(
                COLOR_TEMPERATURE, 140,
                .getter = led_color_temperature_get,
                .setter = led_color_temperature_set
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
    init_led_strip();
    init_white_leds();
}
