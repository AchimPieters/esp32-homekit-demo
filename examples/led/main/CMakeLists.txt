idf_component_register(
    SRCS "led.c"
    REQUIRES freertos lwip bt mbedtls fatfs homekit openthread nvs_flash spi_flash esp_wifi driver esp_partition
)
