#include "bootloader_crc32.h"

#include "bootloader_mcu.h"
#include "crc32.h"
#include "flash.h"
#include "log.h"
#include "stdio.h"

#define BUFFER_LEN 2048

// calculates the crc32 for the full address, with 2048 byte increments
// at a time, returning the final crc32 code
uint32_t calculate_application_crc32() {
  // code array for holding crc32 codes per 2048 bytes
  // size of array is BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1
  uint8_t crc32_codes[(BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1) * 4];
  uint8_t buffer_bytes_size = 0;
  uint8_t crc32_code_number = 0;
  size_t curr_size = 0;        // memory size that has been read
  uint8_t buffer[BUFFER_LEN];  // buffer for holding 2048 bytes of flash
  uint32_t crc_temp = 0;
  uint8_t first_byte = 0, second_byte = 0, third_byte = 0, fourth_byte = 0;

  while (curr_size < BOOTLOADER_APPLICATION_SIZE) {
    // read from flash
    flash_read((uintptr_t)BOOTLOADER_APPLICATION_START + curr_size, sizeof(buffer[0]) * BUFFER_LEN,
               buffer, sizeof(buffer[0]) * BUFFER_LEN);

    // calculate crc32
    crc_temp = crc32_arr(buffer, sizeof(buffer[0]) * BUFFER_LEN);

    // extact bytes of each buffer value in little endian form
    first_byte = (crc_temp << 24) >> 24;
    crc32_codes[crc32_code_number] = first_byte;

    second_byte = (crc_temp << 16) >> 24;
    crc32_codes[crc32_code_number + 1] = second_byte;

    third_byte = (crc_temp << 8) >> 24;
    crc32_codes[crc32_code_number + 2] = third_byte;

    fourth_byte = (crc_temp) >> 24;
    crc32_codes[crc32_code_number + 3] = fourth_byte;

    crc32_code_number += 4;
    curr_size += sizeof(buffer);
  }

  // calculate the final crc32 and return it
  return crc32_arr(crc32_codes, sizeof(crc32_codes[0]) * crc32_code_number);
}
