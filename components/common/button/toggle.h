#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef void (*toggle_callback_fn)(bool high, void* context);

int toggle_create(uint8_t gpio_num, toggle_callback_fn callback, void* context);
void toggle_delete(uint8_t gpio_num);
