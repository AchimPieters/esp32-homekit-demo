# CMakeLists.txt
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_wifi nvs_flash driver homekit led_strip esp32-sht3x esp32-button
)
