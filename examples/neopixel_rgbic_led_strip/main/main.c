#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ws2811_led_strip.h>

#define LED_STRIP_LENGTH 240

void app_main(void)
{
    led_strip_t *strip = led_strip_init(RMT_CHANNEL_0, GPIO_NUM_5, LED_STRIP_LENGTH);

    if (!strip) {
        printf("Failed to initialize LED strip\n");
        return;
    }

    while (1) {
        for (int i = 0; i < LED_STRIP_LENGTH; i++) {
            led_strip_set_pixel(strip, i, 255, 0, 0);  // Red
        }
        led_strip_refresh(strip);
        vTaskDelay(pdMS_TO_TICKS(1000));

        for (int i = 0; i < LED_STRIP_LENGTH; i++) {
            led_strip_set_pixel(strip, i, 0, 255, 0);  // Green
        }
        led_strip_refresh(strip);
        vTaskDelay(pdMS_TO_TICKS(1000));

        for (int i = 0; i < LED_STRIP_LENGTH; i++) {
            led_strip_set_pixel(strip, i, 0, 0, 255);  // Blue
        }
        led_strip_refresh(strip);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    led_strip_free(strip);
}
