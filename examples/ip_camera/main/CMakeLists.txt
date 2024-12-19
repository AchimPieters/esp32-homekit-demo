# Add a custom flag for wolfSSL settings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_wifi nvs_flash driver homekit esp_h264 esp32-camera
)
