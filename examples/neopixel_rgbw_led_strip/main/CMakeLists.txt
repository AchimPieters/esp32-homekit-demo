idf_component_register(
    SRCS "main.c"
    REQUIRES freertos esp_wifi nvs_flash driver esp32-homekit led_strip hsi2rgbw
)
