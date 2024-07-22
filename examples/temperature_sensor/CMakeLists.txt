cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring for ESP32 variant: ${CONFIG_IDF_TARGET}")
if(CONFIG_IDF_TARGET STREQUAL "esp32")
  message(STATUS "Using partition table for ESP32")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32s2")
  message(STATUS "Using partition table for ESP32S2")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32s2.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32c3")
  message(STATUS "Using partition table for ESP32C3")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32c3.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32s3")
  message(STATUS "Using partition table for ESP32S3")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32s3.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32c2")
  message(STATUS "Using partition table for ESP32C2")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32c2.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32c6")
  message(STATUS "Using partition table for ESP32C6")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32c6.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32h2")
  message(STATUS "Using partition table for ESP32H2")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32h2.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32p4")
  message(STATUS "Using partition table for ESP32P4")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32p4.csv" CACHE FILE "Default partition table file")
elseif(CONFIG_IDF_TARGET STREQUAL "esp32c5")
  message(STATUS "Using partition table for ESP32C5")
  set(PARTITION_TABLE_DEFAULT "partitions/partitions_esp32c5.csv" CACHE FILE "Default partition table file")
endif()
message(STATUS "Partition table file: ${PARTITION_TABLE_DEFAULT}")

set(EXTRA_COMPONENT_DIRS
    ../../components
    ../../components/esp-idf
    ../../components/esp32-library/components
)

set(COMPONENTS main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(main)
