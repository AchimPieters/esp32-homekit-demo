#include "port.h"

#ifdef ESP_IDF

// ESP-IDF part
#include <driver/gpio.h>

void my_gpio_enable(uint8_t gpio) {
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
}

void my_gpio_pullup(uint8_t gpio) {
    gpio_set_pull_mode(gpio, GPIO_PULLUP_ONLY);
}

void my_gpio_pulldown(uint8_t gpio) {
    gpio_set_pull_mode(gpio, GPIO_PULLDOWN_ONLY);
}

uint8_t my_gpio_read(uint8_t gpio) {
    return gpio_get_level(gpio);
}

#else

// ESP-OPEN-RTOS part
#include <esp/gpio.h>

void my_gpio_enable(uint8_t gpio) {
    gpio_enable(gpio, GPIO_INPUT);
}

void my_gpio_pullup(uint8_t gpio) {
    gpio_set_pullup(gpio, true, true);
}

void my_gpio_pulldown(uint8_t gpio) {
    gpio_set_pullup(gpio, false, false);
}

uint8_t my_gpio_read(uint8_t gpio) {
    return gpio_read(gpio);
}


#endif
