

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <FreeRTOS.h>

extern homekit_characteristic_t custom_ampere;
extern homekit_characteristic_t custom_volt;
extern homekit_characteristic_t custom_watt;

#ifndef __HOMEKIT_DBB_CUSTOM_CHARACTERISTICS__
#define __HOMEKIT_DBB_CUSTOM_CHARACTERISTICS__

#define HOMEKIT_CUSTOM_UUID_DBB(value) (value "-4772-4466-80fd-a6ea3d5bcd55")

#define HOMEKIT_CHARACTERISTIC_CUSTOM_LPG_LEVEL HOMEKIT_CUSTOM_UUID_DBB("F0000003")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_LPG_LEVEL(_value, ...) .type = HOMEKIT_CHARACTERISTIC_CUSTOM_LPG_LEVEL, \
        .description = "LPG Level", \
        .format = homekit_format_float, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {0}, \
        .max_value = (float[]) {10000}, \
        .min_step = (float[]) {1}, \
        .value = HOMEKIT_FLOAT_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_METHANE_LEVEL HOMEKIT_CUSTOM_UUID_DBB("F0000004")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_METHANE_LEVEL(_value, ...) .type = HOMEKIT_CHARACTERISTIC_CUSTOM_METHANE_LEVEL, \
        .description = "Methane Level", \
        .format = homekit_format_float, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {0}, \
        .max_value = (float[]) {10000}, \
        .min_step = (float[]) {1}, \
        .value = HOMEKIT_FLOAT_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_AMMONIUM_LEVEL HOMEKIT_CUSTOM_UUID_DBB("F0000005")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_AMMONIUM_LEVEL(_value, ...) .type = HOMEKIT_CHARACTERISTIC_CUSTOM_AMMONIUM_LEVEL, \
        .description = "Amonium Level", \
        .format = homekit_format_float, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {0}, \
        .max_value = (float[]) {10000}, \
        .min_step = (float[]) {1}, \
        .value = HOMEKIT_FLOAT_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_WATTS HOMEKIT_CUSTOM_UUID_DBB("F0000017")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_WATTS(_value, ...) \
        .type = HOMEKIT_CHARACTERISTIC_CUSTOM_WATTS, \
        .description = "WATTS", \
        .format = homekit_format_uint16, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {0}, \
        .max_value = (float[]) {3120}, \
        .min_step = (float[]) {1}, \
        .value = HOMEKIT_UINT16_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_VOLTS HOMEKIT_CUSTOM_UUID_DBB("F0000018")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_VOLTS(_value, ...) \
        .type = HOMEKIT_CHARACTERISTIC_CUSTOM_VOLTS, \
        .description = "VOLTS", \
        .format = homekit_format_uint16, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {0}, \
        .max_value = (float[]) {240}, \
        .min_step = (float[]) {1}, \
        .value = HOMEKIT_UINT16_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_AMPS HOMEKIT_CUSTOM_UUID_DBB("F0000019")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_AMPS(_value, ...) \
        .type = HOMEKIT_CHARACTERISTIC_CUSTOM_AMPS, \
        .description = "AMPS", \
        .format = homekit_format_float, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {0}, \
        .max_value = (float[]) {13}, \
        .min_step = (float[]) {0.01}, \
        .value = HOMEKIT_FLOAT_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_CALIBRATE_POW HOMEKIT_CUSTOM_UUID_DBB("F000001A")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_CALIBRATE_POW(_value, ...) \
        .type = HOMEKIT_CHARACTERISTIC_CUSTOM_CALIBRATE_POW, \
        .description = "Calibrate POW", \
        .format = homekit_format_bool, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_paired_write \
                       | homekit_permissions_notify, \
        .value = HOMEKIT_BOOL_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_CALIBRATE_VOLTS HOMEKIT_CUSTOM_UUID_DBB("F000001B")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_CALIBRATE_VOLTS(_value, ...) \
        .type = HOMEKIT_CHARACTERISTIC_CUSTOM_CALIBRATE_VOLTS, \
        .description = "Calibrate Volts", \
        .format = homekit_format_uint16, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_paired_write \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {1}, \
        .max_value = (float[]) {240}, \
        .min_step = (float[]) {1}, \
        .value = HOMEKIT_UINT16_(_value), \
        ## __VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_CALIBRATE_WATTS HOMEKIT_CUSTOM_UUID_DBB("F000001C")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_CALIBRATE_WATTS(_value, ...) \
        .type = HOMEKIT_CHARACTERISTIC_CUSTOM_CALIBRATE_WATTS, \
        .description = "Calibrate WATTS", \
        .format = homekit_format_uint16, \
        .permissions = homekit_permissions_paired_read \
                       | homekit_permissions_paired_write \
                       | homekit_permissions_notify, \
        .min_value = (float[]) {1}, \
        .max_value = (float[]) {3120}, \
        .min_step = (float[]) {1}, \
        .value = HOMEKIT_UINT16_(_value), \
        ## __VA_ARGS__

#endif
