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
        count_addr=(word2>word1)?0x40:0x1040; //select the INactive part
    }
    else {UDPLGP("otadata not OK! ABORT\n");}

    // read 4 bytes at a time (32 bit words)
    do {bytes+=4; //first word can never fit the end sequence
        esp_partition_read(partition, count_addr+bytes, &word2, 4);
    } while ( !(word2==UINT32_MAX || (word2&0xF)==0xE) ); //all bits set or ends in 0b1110
    bytes-=4; //address the word before this as word1
    esp_partition_read(partition, count_addr+bytes, &word1, 4);
    //UDPLGP("xxxxxxxx %08lx %08lx\n", word1, word2);

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
        for (jj=0; vv<4; jj++,vv++)            if (word0>>jj&1) val+=(1<<vv); // fill val up to 3 bits, left are lvv bits
    }
    UDPLGP("%08lx %08lx %08lx ", word0, word1, word2);
    UDPLGP("val=%ld, ii=%d, jj=%d, vv=%d, lvv=%d, bytes=%ld, count_addr=%0lx\n",val,ii,jj,vv,lvv,bytes,count_addr);
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
        for (jj=ii;jj<ii+lvv;jj++) word1&=(~(1<<jj)); //put all lvv leftside bits to zero
        if (ii>28) word0=0; // wipe out previous word when less than 4 bits in this word

        new=0xC; //0b1100, to signal temp_boot
        vv=3; //need 3 bits beside the righthand zero
        for (jj=ii-1; jj>=0&&vv>=0; jj--,vv--) if (!(new&(1<<vv))) word1&=(~(1<<jj)); //transfer bits to word1
        if (vv>=0) for (jj=31;vv>=0;jj--,vv--) if (!(new&(1<<vv))) word2&=(~(1<<jj)); //if bits lvv,  to word2

        UDPLGP("%08lx %08lx %08lx  new=%ld\n", word0, word1, word2, new);
        //write words to flash
        if (word0==0         )  esp_partition_write(partition,count_addr+bytes-4,&word0,4);
        if (word1!=0x10000000)  esp_partition_write(partition,count_addr+bytes  ,&word1,4); //stupid compare to shut up compiler
        if (word2!=UINT32_MAX)  esp_partition_write(partition,count_addr+bytes+4,&word2,4);
    }
    else UDPLGP("otadata not OK! normal reboot\n");

    vTaskDelay(50); //allow e.g. UDPlog to flush output
    esp_restart();
    vTaskDelete(NULL); //should never get here
}
