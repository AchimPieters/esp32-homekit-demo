#include <stdio.h>
#include <stdlib.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>

#include <button.h>

#ifndef BUTTON1_GPIO
#error BUTTON1_GPIO just be defined
#endif
#ifndef BUTTON2_GPIO
#error BUTTON2_GPIO just be defined
#endif

void idle_task(void* arg) {
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}


void button_callback(button_event_t event, void* context) {
    int button_idx = *((uint8_t*) context);

    switch (event) {
        case button_event_single_press:
            printf("button %d single press\n", button_idx);
            break;
        case button_event_double_press:
            printf("button %d double press\n", button_idx);
            break;
        case button_event_tripple_press:
            printf("button %d tripple press\n", button_idx);
            break;
        case button_event_long_press:
            printf("button %d long press\n", button_idx);
            break;
        default:
            printf("unexpected button %d event: %d\n", button_idx, event);
    }
}


uint8_t button_idx1 = 1;
uint8_t button_idx2 = 2;


void user_init(void) {
    uart_set_baud(0, 115200);

    printf("Button example\n");

    button_config_t button_config = BUTTON_CONFIG(
        button_active_low,
        .long_press_time = 1000,
        .max_repeat_presses = 3,
    );

    int r;
    r = button_create(BUTTON1_GPIO, button_config, button_callback, &button_idx1);
    if (r) {
        printf("Failed to initialize button %d (code %d)\n", button_idx1, r);
    }

    r = button_create(BUTTON2_GPIO, button_config, button_callback, &button_idx2);
    if (r) {
        printf("Failed to initialize button %d (code %d)\n", button_idx1, r);
    }

    xTaskCreate(idle_task, "Idle task", 256, NULL, tskIDLE_PRIORITY, NULL);
}
