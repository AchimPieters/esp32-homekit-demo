#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toggle.h"
#include "button.h"
#include "port.h"


typedef struct _button {
    uint8_t gpio_num;
    button_config_t config;
    button_callback_fn callback;
    void* context;

    uint8_t press_count;
    TimerHandle_t long_press_timer;
    TimerHandle_t repeat_press_timeout_timer;

    struct _button *next;
} button_t;

static SemaphoreHandle_t buttons_lock = NULL;
static button_t *buttons = NULL;

static void button_fire_event(button_t *button) {
    button_event_t event = button_event_single_press;

    switch (button->press_count) {
        case 1: event = button_event_single_press; break;
        case 2: event = button_event_double_press; break;
        case 3: event = button_event_tripple_press; break;
    }

    button->callback(event, button->context);
    button->press_count = 0;
}

static void button_toggle_callback(bool high, void *context) {
    if (!context)
        return;

    button_t *button = (button_t*) context;
    if (high == (button->config.active_level == button_active_high)) {
        // pressed
        button->press_count++;
        if (button->config.long_press_time && button->press_count == 1) {
            xTimerStart(button->long_press_timer, 1);
        }
    } else {
        // released
        if (!button->press_count)
            return;

        if (button->long_press_timer
                && xTimerIsTimerActive(button->long_press_timer)) {
            xTimerStop(button->long_press_timer, 1);
        }

        if (button->press_count >= button->config.max_repeat_presses
                || !button->config.repeat_press_timeout) {
            if (button->repeat_press_timeout_timer
                    && xTimerIsTimerActive(button->repeat_press_timeout_timer)) {
                xTimerStop(button->repeat_press_timeout_timer, 1);
            }

            button_fire_event(button);
        } else {
            xTimerStart(button->repeat_press_timeout_timer, 1);
        }
    }
}

static void button_long_press_timer_callback(TimerHandle_t timer) {
    button_t *button = (button_t*) pvTimerGetTimerID(timer);

    button->callback(button_event_long_press, button->context);
    button->press_count = 0;
}

static void button_repeat_press_timeout_timer_callback(TimerHandle_t timer) {
    button_t *button = (button_t*) pvTimerGetTimerID(timer);

    button_fire_event(button);
}

static void button_free(button_t *button) {
    if (button->long_press_timer) {
        xTimerStop(button->long_press_timer, 1);
        xTimerDelete(button->long_press_timer, 1);
    }

    if (button->repeat_press_timeout_timer) {
        xTimerStop(button->repeat_press_timeout_timer, 1);
        xTimerDelete(button->repeat_press_timeout_timer, 1);
    }

    free(button);
}

static int buttons_init() {
    if (!buttons_lock) {
        buttons_lock = xSemaphoreCreateBinary();
        xSemaphoreGive(buttons_lock);
    }

    return 0;
}

int button_create(const uint8_t gpio_num,
                  button_config_t config,
                  button_callback_fn callback,
                  void* context)
{
    if (!buttons_lock) {
        buttons_init();
    }

    xSemaphoreTake(buttons_lock, portMAX_DELAY);
    button_t *button = buttons;
    while (button && button->gpio_num != gpio_num)
        button = button->next;

    bool exists = button != NULL;
    xSemaphoreGive(buttons_lock);

    if (exists)
        return -1;

    button = malloc(sizeof(button_t));
    memset(button, 0, sizeof(*button));
    button->gpio_num = gpio_num;
    button->config = config;
    button->callback = callback;
    button->context = context;
    if (config.long_press_time) {
        button->long_press_timer = xTimerCreate(
            "Button Long Press Timer", pdMS_TO_TICKS(config.long_press_time),
            pdFALSE, button, button_long_press_timer_callback
        );
        if (!button->long_press_timer) {
            button_free(button);
            return -2;
        }
    }
    if (config.max_repeat_presses > 1) {
        button->repeat_press_timeout_timer = xTimerCreate(
            "Button Repeat Press Timeout Timer", pdMS_TO_TICKS(config.repeat_press_timeout),
            pdFALSE, button, button_repeat_press_timeout_timer_callback
        );
        if (!button->repeat_press_timeout_timer) {
            button_free(button);
            return -3;
        }
    }

    my_gpio_enable(button->gpio_num);
    if (config.active_level == button_active_low) {
        my_gpio_pullup(button->gpio_num);
    } else {
        my_gpio_pulldown(button->gpio_num);
    }

    int r = toggle_create(gpio_num, button_toggle_callback, button);
    if (r) {
        button_free(button);
        return -4;
    }

    xSemaphoreTake(buttons_lock, portMAX_DELAY);

    button->next = buttons;
    buttons = button;

    xSemaphoreGive(buttons_lock);

    return 0;
}

void button_destroy(const uint8_t gpio_num) {
    if (!buttons_lock) {
        buttons_init();
    }

    xSemaphoreTake(buttons_lock, portMAX_DELAY);

    if (!buttons) {
        xSemaphoreGive(buttons_lock);
        return;
    }

    button_t *button = NULL;
    if (buttons->gpio_num == gpio_num) {
        button = buttons;
        buttons = buttons->next;
    } else {
        button_t *b = buttons;
        while (b->next) {
            if (b->next->gpio_num == gpio_num) {
                button = b->next;
                b->next = b->next->next;
                break;
            }
        }
    }

    if (button) {
        toggle_delete(button->gpio_num);
        button_free(button);
    }

    xSemaphoreGive(buttons_lock);
}
