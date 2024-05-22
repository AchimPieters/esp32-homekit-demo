set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")

idf_component_register(
    SRCS "main.c" "lcm_api.c"
    REQUIRES freertos lwip mbedtls fatfs homekit nvs_flash spi_flash esp_wifi driver esp_partition
)
