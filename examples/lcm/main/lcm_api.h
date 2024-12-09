/* (c) 2022-2024 HomeAccessoryKid
 * LCM API
 * used with LCM4ESP32
 */
#pragma once

#ifndef UDPLUS
    #define UDPLUS printf
#endif
uint8_t lcm_read_count(); //read the restart count value from RTC
void    lcm_temp_boot(); //restart with RTC value of temp_boot active
//if you want lcm_temp_boot to trigger lcm_main with a higher count value to e.g. reset Wi-Fi, use below 5 lines in your code
//  #include "nvs.h"
//  nvs_handle_t lcm_handle;
//  nvs_open("LCM", NVS_READWRITE, &lcm_handle);
//  nvs_set_u8(lcm_handle,"ota_count", your_count);
//  nvs_commit(lcm_handle);
//NOTE that this key will be erased when used by LCM


//ota-api extension for esp32-homekit
#include <homekit/characteristics.h>
#ifndef __HOMEKIT_CUSTOM_CHARACTERISTICS__
#define __HOMEKIT_CUSTOM_CHARACTERISTICS__

#define HOMEKIT_CUSTOM_UUID(value) (value "-0e36-4a42-ad11-745a73b84f2b")

#define HOMEKIT_SERVICE_CUSTOM_SETUP HOMEKIT_CUSTOM_UUID("000000FF")

#define HOMEKIT_CHARACTERISTIC_CUSTOM_OTA_TRIGGER HOMEKIT_CUSTOM_UUID("F0000001")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_OTA_TRIGGER(_value, ...) \
        .type = HOMEKIT_CHARACTERISTIC_CUSTOM_OTA_TRIGGER, \
        .description = "}FirmwareUpdate", \
        .format = homekit_format_bool, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_paired_write \
                       | homekit_permissions_notify, \
        .value = HOMEKIT_BOOL_(_value), \
        ## __VA_ARGS__

unsigned int  ota_read_sysparam(char **manufacturer,char **serial,char **model,char **revision);

void ota_update(void *arg);

void ota_set(homekit_value_t value);

#define API_OTA_TRIGGER HOMEKIT_CHARACTERISTIC_(CUSTOM_OTA_TRIGGER, false, .setter=ota_set)

#endif
