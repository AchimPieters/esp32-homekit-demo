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
#include <driver/ledc.h>
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

// RGB LED control
#define LED_RED_GPIO    21
#define LED_GREEN_GPIO  22
#define LED_BLUE_GPIO   23

void led_write(uint8_t red, uint8_t green, uint8_t blue) {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, red);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, green);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, blue);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
}

void gpio_init() {
    gpio_set_direction(LED_RED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GREEN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BLUE_GPIO, GPIO_MODE_OUTPUT);

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel[3] = {
        {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = LED_RED_GPIO,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .timer_sel  = LEDC_TIMER_0
        },
        {
            .channel    = LEDC_CHANNEL_1,
            .duty       = 0,
            .gpio_num   = LED_GREEN_GPIO,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .timer_sel  = LEDC_TIMER_0
        },
        {
            .channel    = LEDC_CHANNEL_2,
            .duty       = 0,
            .gpio_num   = LED_BLUE_GPIO,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .timer_sel  = LEDC_TIMER_0
        }
    };
    for (int i = 0; i < 3; i++) {
        ledc_channel_config(&ledc_channel[i]);
    }
}

void accessory_identify_task(void *args) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            led_write(255, 255, 255);
            vTaskDelay(pdMS_TO_TICKS(100));
            led_write(0, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    led_write(255, 255, 255);
    vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
    xTaskCreate(accessory_identify_task, "Accessory identify", 2048, NULL, 2, NULL);
}

// sRGB gamma correction
float gamma_correction(float value) {
    if (value <= 0.0031308) {
        return 12.92 * value;
    } else {
        return 1.055 * powf(value, 1.0 / 2.4) - 0.055;
    }
}

// Convert HSB (Hue, Saturation, Brightness) to RGB (Red, Green, Blue)
void hsb_to_rgb(float hue, float saturation, float brightness, uint8_t *red, uint8_t *green, uint8_t *blue) {
    float hue_prime = fmodf(hue / 360.0, 1.0) * 6.0;
    float chroma = brightness * saturation;
    float x = chroma * (1 - fabsf(fmodf(hue_prime, 2) - 1));

    float r1, g1, b1;

    if (hue_prime >= 0 && hue_prime < 1) {
        r1 = chroma;
        g1 = x;
        b1 = 0;
    } else if (hue_prime >= 1 && hue_prime < 2) {
        r1 = x;
        g1 = chroma;
        b1 = 0;
    } else if (hue_prime >= 2 && hue_prime < 3) {
        r1 = 0;
        g1 = chroma;
        b1 = x;
    } else if (hue_prime >= 3 && hue_prime < 4) {
        r1 = 0;
        g1 = x;
        b1 = chroma;
    } else if (hue_prime >= 4 && hue_prime < 5) {
        r1 = x;
        g1 = 0;
        b1 = chroma;
    } else {
        r1 = chroma;
        g1 = 0;
        b1 = x;
    }

    float m = brightness - chroma;

    *red = gamma_correction(r1 + m) * 255;
    *green = gamma_correction(g1 + m) * 255;
    *blue = gamma_correction(b1 + m) * 255;
}

// HomeKit characteristics for RGB color control
homekit_characteristic_t hue = HOMEKIT_CHARACTERISTIC_(HUE, 0);
homekit_characteristic_t saturation = HOMEKIT_CHARACTERISTIC_(SATURATION, 0);
homekit_characteristic_t brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 100);

void hue_setter(homekit_value_t value) {
    uint8_t red, green, blue;
    hsb_to_rgb(value.float_value, saturation.value.float_value, brightness.value.float_value, &red, &green, &blue);
    led_write(red, green, blue);
}

void saturation_setter(homekit_value_t value) {
    uint8_t red, green, blue;
    hsb_to_rgb(hue.value.float_value, value.float_value, brightness.value.float_value, &red, &green, &blue);
    led_write(red, green, blue);
}

void brightness_setter(homekit_value_t value) {
    uint8_t red, green, blue;
    hsb_to_rgb(hue.value.float_value, saturation.value.float_value, value.float_value, &red, &green, &blue);
    led_write(red, green, blue);
}

// HomeKit accessory definition
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_lighting, .services = (homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "HomeKit LED"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "StudioPieters®"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "NLDA4SQN1466"),
            HOMEKIT_CHARACTERISTIC(MODEL, "SD466NL/A"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.0.1"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "HomeKit LED"),
            &hue,
            &saturation,
            &brightness,
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
