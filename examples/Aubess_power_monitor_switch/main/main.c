/*
   Copyright 2025 Achim Pieters | StudioPieters®

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

   For more information, visit https://www.studiopieters.nl
 */

#include <stdio.h>
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "custom_characteristics.h"

// GPIO Configuration
#define LED_GPIO GPIO_NUM_13
#define RELAY_GPIO GPIO_NUM_14
#define SWITCH_GPIO GPIO_NUM_12
#define BUTTON_GPIO GPIO_NUM_17

#define UART_TXD_PIN GPIO_NUM_1
#define UART_RXD_PIN GPIO_NUM_3
#define UART_PORT UART_NUM_1

// Logging Tag
static const char *TAG = "AUBESS_SWITCH";

// BL0942 Register Addresses
#define REG_I_RMS 0x03
#define REG_V_RMS 0x04
#define REG_WATT 0x06

// Utility Functions
static void handle_error(esp_err_t err) {
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error: %s", esp_err_to_name(err));
                esp_restart();
        }
}

// UART Initialization for BL0942
static void bl0942_uart_init() {
        const uart_config_t uart_config = {
                .baud_rate = 9600,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        };
        handle_error(uart_param_config(UART_PORT, &uart_config));
        handle_error(uart_set_pin(UART_PORT, UART_TXD_PIN, UART_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        handle_error(uart_driver_install(UART_PORT, 256, 0, 0, NULL, 0));
}

// BL0942 Reading Function
static int32_t read_bl0942_register(uint8_t reg) {
        uint8_t command[] = {0x5A, reg}; // Read command
        uint8_t response[4] = {0};
        uart_write_bytes(UART_PORT, (const char *)command, sizeof(command));
        int len = uart_read_bytes(UART_PORT, response, sizeof(response), pdMS_TO_TICKS(500));
        if (len < 4) {
                ESP_LOGE(TAG, "BL0942 Read Error");
                return -1;
        }
        return (response[1] << 16) | (response[2] << 8) | response[3];
}

// Update Power Data
static void update_power_data() {
        int32_t i_rms = read_bl0942_register(REG_I_RMS);
        int32_t v_rms = read_bl0942_register(REG_V_RMS);
        int32_t watt = read_bl0942_register(REG_WATT);

        if (i_rms < 0 || v_rms < 0 || watt < 0) {
                ESP_LOGE(TAG, "Failed to read BL0942 registers");
                return;
        }

        float current = i_rms * 0.001; // Convert to Amps
        float voltage = v_rms * 0.01; // Convert to Volts
        float power = watt * 0.01; // Convert to Watts

        ESP_LOGI(TAG, "Current: %.3f A, Voltage: %.3f V, Power: %.3f W", current, voltage, power);

        custom_ampere.value = HOMEKIT_FLOAT(current);
        custom_volt.value = HOMEKIT_FLOAT(voltage);
        custom_watt.value = HOMEKIT_FLOAT(power);

        homekit_characteristic_notify(&custom_ampere, custom_ampere.value);
        homekit_characteristic_notify(&custom_volt, custom_volt.value);
        homekit_characteristic_notify(&custom_watt, custom_watt.value);
}

// GPIO and Relay Functions
static void relay_write(bool on) {
        gpio_set_level(RELAY_GPIO, on ? 1 : 0);
}

// GPIO Initialization
void gpio_init() {
        gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_direction(SWITCH_GPIO, GPIO_MODE_INPUT);
        relay_write(false);
}

// LED Control Function
static void led_write(bool on) {
        gpio_set_level(LED_GPIO, on ? 1 : 0);
}

// Accessory Identify Task
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
        vTaskDelete(NULL);
}

static void accessory_identify(homekit_value_t _value) {
        ESP_LOGI(TAG, "Accessory identify");
        xTaskCreate(accessory_identify_task, "Accessory identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// HomeKit Characteristics
#define DEVICE_NAME "Aubess Power Monitor"
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
                        HOMEKIT_CHARACTERISTIC(NAME, "Power Switch"),
                        &custom_ampere,
                        &custom_volt,
                        &custom_watt,
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

// Main Function
void app_main(void) {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ESP_ERROR_CHECK(nvs_flash_init());
        }

        gpio_init();
        bl0942_uart_init();

        ESP_LOGI(TAG, "Starting HomeKit server...");
        homekit_server_init(&config);

        while (1) {
                update_power_data();
                vTaskDelay(pdMS_TO_TICKS(1000));
        }
}
