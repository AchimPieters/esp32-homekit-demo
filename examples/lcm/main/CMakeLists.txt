idf_component_register(
    SRCS "main.c"
    REQUIRES freertos esp_wifi nvs_flash driver esp32-captive_portal esp32-homekit
)
