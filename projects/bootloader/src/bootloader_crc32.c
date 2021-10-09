#include "bootloader_crc32.h"

#include "bootloader_mcu.h"
#include "crc32.h"
#include "flash.h"
#include "stdio.h"

// calculates the crc32 for the full address, with 2048 byte increments
// at a time, returning the final crc32 code
uint32_t calculate_application_crc32() {
  // code array for holding crc32 codes per 2048 bytes
  // size of array is BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1
  uint32_t crc32_codes[BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1];
  uint8_t crc32_code_number = 0;
  size_t curr_size = 0;        // memory size that has been read
  uint8_t buffer[BUFFER_LEN];  // buffer for holding 2048 bytes of flash
  uint32_t crc_temp;

  while (curr_size < BOOTLOADER_APPLICATION_SIZE) {
    // read from flash
    flash_read((uintptr_t)BOOTLOADER_APPLICATION_START + curr_size, sizeof(buffer), buffer, sizeof(buffer));
    // calculate crc32
    crc_temp = crc32_arr(buffer, sizeof(buffer));

    crc32_codes[crc32_code_number] = crc_temp;
    crc32_code_number++;
    curr_size += sizeof(buffer);
  }

  // calculate the final crc32 and return it
  return crc32_arr((uint8_t *)crc32_codes, sizeof(crc32_codes));
}
