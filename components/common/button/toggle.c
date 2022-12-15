#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "toggle.h"
#include "port.h"


#define MAX_TOGGLE_VALUE 4
#define MIN(a, b) (((b) < (a)) ? (b) : (a))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))


typedef struct _toggle {
    uint8_t gpio_num;
    toggle_callback_fn callback;
    void* context;

    // Implementation inspired by
    // https://github.com/RavenSystem/esp-homekit-devices/blob/master/libs/adv_button/adv_button.c
    // and
    // https://github.com/pcsaito/esp-homekit-demo/blob/LPFToggle/examples/sonoff_basic_toggle/toggle.c
    int8_t value;
    bool last_high;

    struct _toggle *next;
} toggle_t;


static SemaphoreHandle_t toggles_lock = NULL;
static toggle_t *toggles = NULL;
static TimerHandle_t toggle_timer = NULL;
static bool toggles_initialized = false;


static toggle_t *toggle_find_by_gpio(const uint8_t gpio_num) {
    toggle_t *toggle = toggles;
    while (toggle && toggle->gpio_num != gpio_num)
        toggle = toggle->next;

    return toggle;
}


static void toggle_timer_callback(TimerHandle_t timer) {
    if (xSemaphoreTake(toggles_lock, 0) != pdTRUE)
        return;

    toggle_t *toggle = toggles;

    while (toggle) {
        if (my_gpio_read(toggle->gpio_num) == 1) {
            toggle->value = MIN(toggle->value + 1, MAX_TOGGLE_VALUE);
            if (toggle->value == MAX_TOGGLE_VALUE && !toggle->last_high) {
                toggle->last_high = true;
                toggle->callback(true, toggle->context);
            }
        } else {
            toggle->value = MAX(toggle->value - 1, 0);
            if (toggle->value == 0 && toggle->last_high) {
                toggle->last_high = false;
                toggle->callback(false, toggle->context);
            }
        }

        toggle = toggle->next;
    }

    xSemaphoreGive(toggles_lock);
}


static int toggles_init() {
    if (!toggles_initialized) {
        toggles_lock = xSemaphoreCreateBinary();
        xSemaphoreGive(toggles_lock);

        toggle_timer = xTimerCreate(
            "Toggle timer", pdMS_TO_TICKS(10), pdTRUE, NULL, toggle_timer_callback
        );

        toggles_initialized = true;
    }

    return 0;
}


int toggle_create(const uint8_t gpio_num, toggle_callback_fn callback, void* context) {
    if (!toggles_initialized)
        toggles_init();

    toggle_t *toggle = toggle_find_by_gpio(gpio_num);
    if (toggle)
        return -1;

    toggle = malloc(sizeof(toggle_t));
    memset(toggle, 0, sizeof(*toggle));
    toggle->gpio_num = gpio_num;
    toggle->callback = callback;
    toggle->context = context;
    toggle->last_high = my_gpio_read(toggle->gpio_num) == 1;

    my_gpio_enable(toggle->gpio_num);

    xSemaphoreTake(toggles_lock, portMAX_DELAY);

    toggle->next = toggles;
    toggles = toggle;

    xSemaphoreGive(toggles_lock);

    if (!xTimerIsTimerActive(toggle_timer)) {
        xTimerStart(toggle_timer, 1);
    }

    return 0;
}


void toggle_delete(const uint8_t gpio_num) {
    if (!toggles_initialized)
        toggles_init();

    xSemaphoreTake(toggles_lock, portMAX_DELAY);

    if (!toggles) {
        xSemaphoreGive(toggles_lock);
        return;
    }

    toggle_t *toggle = NULL;
    if (toggles->gpio_num == gpio_num) {
        toggle = toggles;
        toggles = toggles->next;
    } else {
        toggle_t *b = toggles;
        while (b->next) {
            if (b->next->gpio_num == gpio_num) {
                toggle = b->next;
                b->next = b->next->next;
                break;
            }
        }
    }

    if (!toggles) {
        xTimerStop(toggle_timer, 1);
    }

    xSemaphoreGive(toggles_lock);

    if (!toggle)
        return;

    free(toggle);
}
