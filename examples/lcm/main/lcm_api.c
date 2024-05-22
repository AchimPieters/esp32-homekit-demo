/* (c) 2022 HomeAccessoryKid
 * LCM API
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bootloader_common.h"

static uint8_t count=0,rtc_read_busy=1;
static void rtcr_task(void *arg) {
    rtc_retain_mem_t* rtcmem=bootloader_common_get_rtc_retain_mem(); //access to the memory struct
    if (bootloader_common_get_rtc_retain_mem_reboot_counter()) { //if zero, RTC CRC not valid
        count=rtcmem->custom[0]; //byte zero for count
    } else {
        count=0; //valid count values are > 0
    }
    bootloader_common_reset_rtc_retain_mem(); //this will clear RTC
    rtc_read_busy=0;
    vTaskDelete(NULL);
}

uint8_t lcm_read_count() {
    xTaskCreatePinnedToCore(rtcr_task,"rtcr",4096,NULL,1,NULL,0); //CPU_0 PRO_CPU needed for rtc operations
    while (rtc_read_busy) vTaskDelay(1);
    return count;
}


static void rtcw_task(void *arg) {
    rtc_retain_mem_t* rtcmem=bootloader_common_get_rtc_retain_mem(); //access to the memory struct
    bootloader_common_reset_rtc_retain_mem(); //this will clear RTC
    rtcmem->reboot_counter=1; //needed to make RTC reading valid
    rtcmem->custom[1]=1; //byte one for temp_boot signal (from app to bootloader)
    bootloader_common_update_rtc_retain_mem(NULL,false); //this will update the CRC only
    esp_restart();
    vTaskDelete(NULL); //should never get here
}

//if you want lcm_temp_boot to trigger lcm_main with a higher count value to e.g. reset Wi-Fi then use below 5 lines in your code
//  #include "nvs.h"
//  nvs_handle_t lcm_handle;
//  nvs_open("LCM", NVS_READWRITE, &lcm_handle);
//  nvs_set_u8(lcm_handle,"ota_count", your_count);
//  nvs_commit(lcm_handle);
//NOTE that this key will be erased when used by LCM
void    lcm_temp_boot() { //restart with RTC value of temp_boot active
    xTaskCreatePinnedToCore(rtcw_task,"rtcw",4096,NULL,1,NULL,0); //CPU_0 PRO_CPU needed for rtc operations
    while (true) vTaskDelay(100); //we should not return
}
