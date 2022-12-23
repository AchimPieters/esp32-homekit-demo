idf_component_register(
    SRCS toggle.c button.c port.c
    REQUIRES freertos lwip bt mbedtls fatfs homekit openthread nvs_flash spi_flash esp_wifi driver esp_partition
    INCLUDE_DIRS .
)
target_compile_options(${COMPONENT_LIB} PRIVATE -DESP_IDF)
