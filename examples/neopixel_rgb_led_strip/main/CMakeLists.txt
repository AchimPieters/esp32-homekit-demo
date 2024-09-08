set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")

idf_component_register(
    SRCS "main.c"
    REQUIRES freertos esp_wifi nvs_flash driver homekit led_strip
)
