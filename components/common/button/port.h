#pragma once

#include <stdint.h>

#ifdef ESP_IDF
    #include <freertos/FreeRTOS.h>
    #include <freertos/semphr.h>
    #include <freertos/timers.h>
#else
    #include <FreeRTOS.h>
    #include <semphr.h>
    #include <timers.h>
#endif

void my_gpio_enable(uint8_t gpio);
void my_gpio_pullup(uint8_t gpio);
void my_gpio_pulldown(uint8_t gpio);
uint8_t my_gpio_read(uint8_t gpio);
