/* (c) 2022-2024 HomeAccessoryKid
 * LCM API
 */

#include "freertos/FreeRTOS.h"
#include "esp_partition.h"
#include "lcm_api.h"

static uint8_t count=0;
static int ii,lvv;
static uint32_t bytes=0,count_addr=0,word0=0x10000000,word1,word2;
uint8_t lcm_read_count() {
        const esp_partition_t *partition=NULL;
        // transfer count value and temp_boot flag in flash
        // 1->0000, 2567->0010, 389A->0100, 4BCD->0110, EFG->1000   and temp_boot->1100 in the way back
        int jj,vv;
        uint32_t val;
        partition=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_DATA_OTA,"otadata");
        if (partition && partition->size==0x2000) {
                esp_partition_read(partition,      0, &word1, 4);
                esp_partition_read(partition, 0x1000, &word2, 4);
                if (word2==UINT32_MAX) word2=0; //for uninitialized ota_data[1]
                count_addr=(word2>word1) ? 0x40 : 0x1040; //select the INactive part
        }
        else {UDPLUS("otadata not OK! ABORT\n");}

        // read 4 bytes at a time (32 bit words)
        do {bytes+=4; //first word can never fit the end sequence
            esp_partition_read(partition, count_addr+bytes, &word2, 4);} while ( !(word2==UINT32_MAX || (word2&0xF)==0xE) ); //all bits set or ends in 0b1110
        bytes-=4; //address the word before this as word1
        esp_partition_read(partition, count_addr+bytes, &word1, 4);
        //UDPLUS("xxxxxxxx %08lx %08lx\n", word1, word2);

        if (word2<UINT32_MAX-1) {word1=word2; word2=UINT32_MAX; bytes+=4;} //already started with bits in the last word, shift right

        val=0; ii=0;
        if ((word1&0xF)==0xE) { //word1 ends in 0xE
                ii=1; //skip last bit
        }
        // extract current value where we first evaluate word1 and conditionally word0
        while ( (word1>>ii)&1 ) ii++; //find bit number ii for right-most zero
        for (jj=ii+1,vv=1; jj<32&&vv<4; jj++,vv++) if (word1>>jj&1) val+=(1<<vv); // copy three bits to val with index vv
        lvv=vv; // store the val index for later
        if(vv<4) { // we need word0 to complete the reading
                esp_partition_read(partition, count_addr+bytes-4,  &word0, 4);
                for (jj=0; vv<4; jj++,vv++) if (word0>>jj&1) val+=(1<<vv);    // fill val up to 3 bits, left are lvv bits
        }
        UDPLUS("%08lx %08lx %08lx ", word0, word1, word2);
        UDPLUS("val=%ld, ii=%d, jj=%d, vv=%d, lvv=%d, bytes=%ld, count_addr=%0lx\n",val,ii,jj,vv,lvv,bytes,count_addr);
        count=val/2+1;

        return count;
}

void lcm_temp_boot() { //restart with RTC value of temp_boot active
        if (count==0) lcm_read_count(); //this is required to read the initial word values
        const esp_partition_t *partition=NULL;
        partition=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_DATA_OTA,"otadata");
        if (partition && partition->size==0x2000) {
                int jj,vv;
                uint32_t new;
                //reset bits on the left
                for (jj=ii; jj<ii+lvv; jj++) word1&=(~(1<<jj)); //put all lvv leftside bits to zero
                if (ii>28) word0=0; // wipe out previous word when less than 4 bits in this word

                new=0xC; //0b1100, to signal temp_boot
                vv=3; //need 3 bits beside the righthand zero
                for (jj=ii-1; jj>=0&&vv>=0; jj--,vv--) if (!(new&(1<<vv))) word1&=(~(1<<jj)); //transfer bits to word1
                if (vv>=0) for (jj=31; vv>=0; jj--,vv--) if (!(new&(1<<vv))) word2&=(~(1<<jj)); //if bits lvv,  to word2

                UDPLUS("%08lx %08lx %08lx  new=%ld\n", word0, word1, word2, new);
                //write words to flash
                if (word0==0         ) esp_partition_write(partition,count_addr+bytes-4,&word0,4);
                if (word1!=0x10000000) esp_partition_write(partition,count_addr+bytes,&word1,4); //stupid compare to shut up compiler
                if (word2!=UINT32_MAX) esp_partition_write(partition,count_addr+bytes+4,&word2,4);
        }
        else UDPLUS("otadata not OK! normal reboot\n");

        vTaskDelay(50); //allow e.g. UDPlog to flush output
        esp_restart();
}

//ota-api extension for esp32-homekit
#include <string.h>
#include "nvs_flash.h"
#include "esp_mac.h"
#include "driver/gpio.h"

// the first function is the ONLY thing needed for a repo to support ota after having started with ota-boot
// in ota-boot the user gets to set the wifi and the repository details and it then installs the ota-main binary

void ota_update(void *arg) {  //arg not used
        //TODO: make a distinct light pattern or other feedback to the user = call identify routine
        //to get a clean reboot, we disable any GPIO output functions, this does not happen by itself
        gpio_config_t io_conf = {}; //zero-initialize the config structure.
        //for C3 use GPIO-0 - GPIO-11 + GPIO-18-GPIO21
        //for 32 use GPIO-0 - GPIO-5 + GPIO-12-GPIO36 + GPIO-39  etc.
        io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_32|1ULL<<GPIO_NUM_33); //bit mask of the pins for now
        io_conf.mode = GPIO_MODE_INPUT;
        gpio_config(&io_conf); //configure GPIO with the given settings
        //ready to reboot
        vTaskDelay(500/portTICK_PERIOD_MS);
        lcm_temp_boot(); //select the OTA main routine
        vTaskDelete(NULL); //should never get here
}

// this function is optional to couple Homekit parameters to the nvs variables and github parameters
unsigned int  ota_read_sysparam(char **manufacturer,char **serial,char **model,char **revision) {
        esp_err_t status;
        nvs_handle_t lcm_handle;
        char *value;
        size_t size;

        status = nvs_open("LCM", NVS_READONLY, &lcm_handle);

        if (!status && nvs_get_str(lcm_handle, "ota_repo", NULL, &size) == ESP_OK) {
                value = malloc(size);
                nvs_get_str(lcm_handle, "ota_repo", value, &size);
                strchr(value,'/')[0]=0;
                *manufacturer=value;
                *model=value+strlen(value)+1;
        } else {
                *manufacturer="manuf_unknown"; //TODO: will this be a valid pointer outside this function?
                *model="model_unknown";
        }

        if (!status && nvs_get_str(lcm_handle, "ota_version", NULL, &size) == ESP_OK) {
                value = malloc(size);
                nvs_get_str(lcm_handle, "ota_version", value, &size);
                *revision=value;
        } else *revision="0.0.0";

        uint8_t macaddr[6];
        if (esp_read_mac(macaddr, ESP_MAC_WIFI_STA) == ESP_OK) {
                *serial=malloc(18);
                sprintf(*serial,"%02X:%02X:%02X:%02X:%02X:%02X",macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
        } else {
                *serial="_serial__unknown_";
                UDPLUS("issue with ota_version, expect trouble\n");
                return 1; //c_hash==0 is illegal in homekit
        }

        unsigned int c_hash=0;
        char version[16];
        char* rev=version;
        char* dot;
        strncpy(rev,*revision,16);
        if ((dot=strchr(rev,'.'))) {dot[0]=0; c_hash=            atoi(rev); rev=dot+1;}
        if ((dot=strchr(rev,'.'))) {dot[0]=0; c_hash=c_hash*1000+atoi(rev); rev=dot+1;}
        c_hash=c_hash*1000+atoi(rev);
        //c_hash=c_hash*10  +configuration_variant; //possible future extension
        UDPLUS("manuf=\'%s\' serial=\'%s\' model=\'%s\' revision=\'%s\' c#=%d\n",*manufacturer,*serial,*model,*revision,c_hash);
        return c_hash;
}


#include <homekit/characteristics.h>
void ota_set(homekit_value_t value) {
        if (value.format != homekit_format_bool) {
                UDPLUS("Invalid ota-value format: %d\n", value.format);
                return;
        }
        if (value.bool_value) {
                xTaskCreate(ota_update,"ota", 3072, NULL, 1, NULL);
        }
}
