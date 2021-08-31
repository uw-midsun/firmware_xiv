#include "bootloader_crc32.h"

#include "crc32.h"
#include "stdio.h"

#include "flash.h"
#include "bootloader_mcu.h"


uint32_t single_code_crc32(){
    uint8_t buffer[2048];
    uint32_t crc_temp;

    flash_read((uintptr_t)BOOTLOADER_APPLICATION_START, sizeof(buffer), (uint8_t *)&buffer, sizeof(buffer));
    crc_temp = crc32_arr((uint8_t *)&buffer, sizeof(buffer));

    return crc_temp;
}

void bootloader_crc32(uintptr_t address, size_t size, uint32_t *crc32_codes){
    uint8_t code_number = 0;
    size_t curr_size = 0;
    uint8_t buffer[2048];
    uint32_t crc_temp;

    while(curr_size <= size){
        printf("address is %ld\n",address + curr_size);
        printf("CURRENT SIZE VS ACTUAL SIZE:\t%ld, %ld\n", curr_size, size);
        
        flash_read(address + curr_size, sizeof(buffer), (uint8_t *)&buffer, sizeof(buffer));
        crc_temp = crc32_arr((uint8_t *)&buffer, sizeof(buffer));
        crc32_codes[code_number] = crc_temp;
        code_number++;
        curr_size += sizeof(buffer);
    }

}