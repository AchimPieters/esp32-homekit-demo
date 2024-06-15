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

#include <button.h> // Only for onboard button

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

//  CEN  ADC  P8  P7  P6   ||   3v3  GND  RX1  TX1  P24  P26
//           led  btn                     942  942  swt  rel
//  EN    6   7   10  18   ||   vcc  GND  RXD  TXD   5    4


// BL0942_TX -> TX1  GPIO1
// BL0942_RX -> RX1  GPIO3

#define LED_GPIO      13   // ONBOARD LED (verified)
#define BUTTON_GPIO   17    // RESET BUTTON
#define RELAY_GPIO    14   // RELAY (verified)
#define SWITCH_GPIO   12   // WALL SWITCH (verified)

void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);

bool led_on = false;

void led_write(bool on) {
    gpio_set_level(LED_GPIO, on ? 0 : 1);
}

void relay_write(bool on) {
    gpio_set_level(RELAY_GPIO, on ? 1 : 0);
}

homekit_characteristic_t switch_on = HOMEKIT_CHARACTERISTIC_(ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback));

void wall_switch_callback(uint8_t gpio) {
    if (gpio == SWITCH_GPIO) {
        int state = 0;
        state = gpio_get_level(SWITCH_GPIO);
        switch_on.value = HOMEKIT_BOOL(state);
        homekit_characteristic_notify(&switch_on, HOMEKIT_BOOL(state));
        if (state == 1) {
            ESP_LOGI("WALL_SWITCH_STATE", "Wall Switch off");
            relay_write(switch_on.value.bool_value);
        } else {
            ESP_LOGI("WALL_SWITCH_STATE", "Wall Switch on");
            relay_write(switch_on.value.bool_value);
        }
    }
}

// ISR handler
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    wall_switch_callback(gpio_num);
}

void gpio_init() {
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    led_write(led_on);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    relay_write(switch_on.value.bool_value);
    gpio_set_direction(SWITCH_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(SWITCH_GPIO, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add(SWITCH_GPIO, gpio_isr_handler, (void*) SWITCH_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
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
    led_write(led_on);
    vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
    ESP_LOGI("ACCESSORY_IDENTIFY", "Accessory identify");
    xTaskCreate(accessory_identify_task, "Accessory identify", 2048, NULL, 2, NULL);
}

// switch
void switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write(switch_on.value.bool_value);
}

void button_callback(button_event_t event, void *context) {
    switch (event) {
    case button_event_single_press:
        ESP_LOGI("SINGLE_PRESS", "single press");
        break;
    case button_event_double_press:
        ESP_LOGI("DOUBLE_PRESS", "Double press");
        break;
    case button_event_long_press:
        ESP_LOGI("LONG_PRESS", "Long press");
        break;
    default:
        ESP_LOGI("UNKNOWN_BUTTON_EVENT", "unknown button event: %d", event);
    }
}

#define DEVICE_NAME "Switch with power monitoring"
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
    HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_switches, .services = (homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
            &name,
            &manufacturer,
            &serial,
            &model,
            &revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(SWITCH, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "HomeKit Aubess Switch"),
            &switch_on,
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

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Woverride-init"
    button_config_t button_config = BUTTON_CONFIG(
        button_active_low,
        .max_repeat_presses=2,
        .long_press_time=1000,
    );
    #pragma GCC diagnostic pop

    if (button_create(BUTTON_GPIO, button_config, button_callback, NULL)) {
        ESP_LOGI("FAILED_TO_INITIALIZE_BUTTON", "Failed to initialize button");
    }
}
