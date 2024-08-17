/* (c) 2022-2024 HomeAccessoryKid
 * LCM API
 * used with LCM4ESP32
 */
#pragma once

#ifndef UDPLGP
    #define UDPLGP printf
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
