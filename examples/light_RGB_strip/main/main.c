#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
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

#define LPF_SHIFT 4  // divide by 16
#define LPF_INTERVAL 10  // in milliseconds

#define RED_PWM_PIN 21
#define GREEN_PWM_PIN 22
#define BLUE_PWM_PIN 23
#define LED_RGB_SCALE 255       // this is the scaling factor used for color conversion

typedef union {
    struct {
        uint16_t blue;
        uint16_t green;
        uint16_t red;
        uint16_t white;
    };
    uint64_t color;
} rgb_color_t;

// Color smoothing variables
rgb_color_t current_color = { { 0, 0, 0, 0 } };
rgb_color_t target_color = { { 0, 0, 0, 0 } };

// Global variables
float led_hue = 0;              // hue is scaled 0 to 360
float led_saturation = 59;      // saturation is scaled 0 to 100
float led_brightness = 100;     // brightness is scaled 0 to 100
bool led_on = false;            // on is boolean on or off

//http://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
static void hsi2rgb(float h, float s, float i, rgb_color_t* rgb) {
    int r, g, b;

    while (h < 0) { h += 360.0F; }; // cycle h around to 0-360 degrees
    while (h >= 360) { h -= 360.0F; };
    h = 3.14159F * h / 180.0F;        // convert to radians.
    s /= 100.0F;                      // from percentage to ratio
    i /= 100.0F;                      // from percentage to ratio
    s = s > 0 ? (s < 1 ? s : 1) : 0;  // clamp s and i to interval [0,1]
    i = i > 0 ? (i < 1 ? i : 1) : 0;  // clamp s and i to interval [0,1]

    if (h < 2.09439) {
        r = LED_RGB_SCALE * i / 3 * (1 + s * cos(h) / cos(1.047196667 - h));
        g = LED_RGB_SCALE * i / 3 * (1 + s * (1 - cos(h) / cos(1.047196667 - h)));
        b = LED_RGB_SCALE * i / 3 * (1 - s);
    } else if (h < 4.188787) {
        h = h - 2.09439;
        g = LED_RGB_SCALE * i / 3 * (1 + s * cos(h) / cos(1.047196667 - h));
        b = LED_RGB_SCALE * i / 3 * (1 + s * (1 - cos(h) / cos(1.047196667 - h)));
        r = LED_RGB_SCALE * i / 3 * (1 - s);
    } else {
        h = h - 4.188787;
        b = LED_RGB_SCALE * i / 3 * (1 + s * cos(h) / cos(1.047196667 - h));
        r = LED_RGB_SCALE * i / 3 * (1 + s * (1 - cos(h) / cos(1.047196667 - h)));
        g = LED_RGB_SCALE * i / 3 * (1 - s);
    }

    rgb->red = (uint8_t) r;
    rgb->green = (uint8_t) g;
    rgb->blue = (uint8_t) b;
}

void led_identify_task(void *_args) {
    printf("LED identify\n");

    rgb_color_t color = target_color;
    rgb_color_t black_color = { { 0, 0, 0, 0 } };
    rgb_color_t white_color = { { 128, 128, 128, 128 } };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            target_color = white_color;
            vTaskDelay(100 / portTICK_PERIOD_MS);

            target_color = black_color;
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    target_color = color;

    vTaskDelete(NULL);
}

void led_identify(homekit_value_t _value) {
    xTaskCreate(led_identify_task, "LED identify", 128, NULL, 2, NULL);
}

homekit_value_t led_on_get() {
    return HOMEKIT_BOOL(led_on);
}

void led_on_set(homekit_value_t value) {
    if (value.format != homekit_format_bool) {
        return;
    }

    led_on = value.bool_value;
}

homekit_value_t led_brightness_get() {
    return HOMEKIT_INT(led_brightness);
}

void led_brightness_set(homekit_value_t value) {
    if (value.format != homekit_format_int) {
        return;
    }
    led_brightness = value.int_value;
}

homekit_value_t led_hue_get() {
    return HOMEKIT_FLOAT(led_hue);
}

void led_hue_set(homekit_value_t value) {
    if (value.format != homekit_format_float) {
        return;
    }
    led_hue = value.float_value;
}

homekit_value_t led_saturation_get() {
    return HOMEKIT_FLOAT(led_saturation);
}

void led_saturation_set(homekit_value_t value) {
    if (value.format != homekit_format_float) {
        return;
    }
    led_saturation = value.float_value;
}

// HomeKit characteristics
#define DEVICE_NAME "HomeKit RGB Strip"
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
            HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "HomeKit RGB Strip"),
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

void ledc_task(void *pvParameters) {
    const TickType_t xPeriod = pdMS_TO_TICKS(LPF_INTERVAL);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    ledc_channel_config_t ledc_channel[3] = {
        {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = RED_PWM_PIN,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        },
        {
            .channel    = LEDC_CHANNEL_1,
            .duty       = 0,
            .gpio_num   = GREEN_PWM_PIN,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        },
        {
            .channel    = LEDC_CHANNEL_2,
            .duty       = 0,
            .gpio_num   = BLUE_PWM_PIN,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        }
    };

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ledc_timer_config(&ledc_timer);
    for (int ch = 0; ch < 3; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    while (1) {
        if (led_on) {
            hsi2rgb(led_hue, led_saturation, led_brightness, &target_color);
        } else {
            target_color.red = 0;
            target_color.green = 0;
            target_color.blue = 0;
        }

        current_color.red += ((target_color.red * 256) - current_color.red) >> LPF_SHIFT;
        current_color.green += ((target_color.green * 256) - current_color.green) >> LPF_SHIFT;
        current_color.blue += ((target_color.blue * 256) - current_color.blue) >> LPF_SHIFT;

        ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, current_color.red >> 8);
        ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
        ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, current_color.green >> 8);
        ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
        ledc_set_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel, current_color.blue >> 8);
        ledc_update_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

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
    xTaskCreate(ledc_task, "ledc_task", 4096, NULL, 2, NULL);
}
