#include "bootloader_crc32.h"

#include "stdio.h"

#include "bootloader_mcu.h"
#include "crc32.h"
#include "flash.h"

// calculates the crc32 for the full address, with 2048 byte increments
// at a time, returning the final crc32 code
uint32_t calculated_application_crc32(uintptr_t address, size_t size) {
  // code array for holding crc32 codes per 2048 bytes
  uint32_t crc32_codes[crc32_code_buffer_size];
  uint8_t crc32_code_number = 0;
  size_t curr_size = 0;  // memory size that has been read
  uint8_t buffer[2048];  // buffer for holding 2048 bytes of flash
  uint32_t crc_temp;

  while (curr_size <= size) {
    // read from flash
    flash_read(address + curr_size, sizeof(buffer), (uint8_t *)&buffer, sizeof(buffer));
    // claculate crc32
    crc_temp = crc32_arr((uint8_t *)&buffer, sizeof(buffer));

    crc32_codes[crc32_code_number] = crc_temp;
    crc32_code_number++;
    curr_size += sizeof(buffer);
  }

  // calculate the final crc32 and return it
  return crc32_arr((uint8_t *)&crc32_codes, sizeof(crc32_codes));
}
