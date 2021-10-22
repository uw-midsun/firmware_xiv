#include "bootloader_crc32.h"

#include "bootloader_mcu.h"
#include "crc32.h"
#include "flash.h"
#include "log.h"
#include "stdio.h"

// calculates the crc32 for the full address, with 2048 byte increments
// at a time, returning the final crc32 code
uint32_t calculate_application_crc32() {
  // code array for holding crc32 codes per 2048 bytes
  // size of array is BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1
  uint8_t crc32_codes[(BOOTLOADER_APPLICATION_SIZE / BUFFER_LEN + 1) * 4];
  uint8_t buffer_bytes[BUFFER_LEN * 4];
  uint8_t buffer_bytes_size = 0;
  uint8_t crc32_code_number = 0;
  size_t curr_size = 0;        // memory size that has been read
  uint8_t buffer[BUFFER_LEN];  // buffer for holding 2048 bytes of flash
  uint32_t crc_temp = 0;
  uint8_t firstByte = 0, secondByte = 0, thirdByte = 0, fourthByte = 0;

  while (curr_size < BOOTLOADER_APPLICATION_SIZE) {
    // read from flash
    flash_read((uintptr_t)BOOTLOADER_APPLICATION_START + curr_size, sizeof(buffer[0]) * BUFFER_LEN,
               buffer, sizeof(buffer[0]) * BUFFER_LEN);

    // extact bytes of each buffer value in little endian form
    for (uint16_t i = 0; i < BUFFER_LEN; i++) {
      firstByte = (buffer[i] << 24) >> 24;
      buffer_bytes[buffer_bytes_size] = firstByte;

      secondByte = (buffer[i] << 16) >> 24;
      buffer_bytes[buffer_bytes_size + 1] = secondByte;

      thirdByte = (buffer[i] << 8) >> 24;
      buffer_bytes[buffer_bytes_size + 2] = thirdByte;

      fourthByte = buffer[i] >> 24;
      buffer_bytes[buffer_bytes_size + 3] = fourthByte;

      buffer_bytes_size += 4;
    }
    // uncomment this to see if array vals are consistent
    printf("number at index 2047 of buffer: %d\n", buffer[2047]);

    // calculate crc32
    crc_temp = crc32_arr(buffer_bytes, sizeof(buffer_bytes[0]) * BUFFER_LEN * 4);
    // printf("crc32 is %d\n", crc_temp);

    // extact bytes of each buffer value in little endian form
    firstByte = (crc_temp << 24) >> 24;
    crc32_codes[crc32_code_number] = firstByte;

    secondByte = (crc_temp << 16) >> 24;
    crc32_codes[crc32_code_number + 1] = secondByte;

    thirdByte = (crc_temp << 8) >> 24;
    crc32_codes[crc32_code_number + 2] = thirdByte;

    fourthByte = (crc_temp) >> 24;
    crc32_codes[crc32_code_number + 3] = fourthByte;

    crc32_code_number += 4;
    curr_size += sizeof(buffer);
  }
  // printf("bytes are:\n\tfirst: %d\n\tsecond: %d\n\tthird: %d\n\tfourth: %d\n",
  // firstByte,secondByte,thirdByte,fourthByte);

  // calculate the final crc32 and return it
  return crc32_arr(crc32_codes, sizeof(crc32_codes[0]) * crc32_code_number);
}
