
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <homekit/homekit.h>
#include "custom_characteristics.h"

homekit_characteristic_t custom_ampere = HOMEKIT_CHARACTERISTIC_(
        CUSTOM_AMPS, 0.0
        );

homekit_characteristic_t custom_volt = HOMEKIT_CHARACTERISTIC_(
        CUSTOM_VOLTS, 0.0
        );

homekit_characteristic_t custom_watt = HOMEKIT_CHARACTERISTIC_(
        CUSTOM_WATTS, 0.0
        );
