idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_wifi nvs_flash driver esp32-homekit esp_h264 esp32-camera
)
