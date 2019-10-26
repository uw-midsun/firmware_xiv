#include "flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "critical_section.h"
#include "log.h"

#define FLASH_DEFAULT_FILENAME "x86_flash"
#define FLASH_USER_ENV "MIDSUN_X86_FLASH_FILE"

static FILE *s_flash_fp = NULL;

StatusCode flash_init(void) {
  if (s_flash_fp != NULL) {
    fclose(s_flash_fp);
  }

  char *flash_filename = getenv(FLASH_USER_ENV);
  if (flash_filename == NULL) {
    flash_filename = FLASH_DEFAULT_FILENAME;
  }
  LOG_DEBUG("Using flash file: %s\n", flash_filename);

  s_flash_fp = fopen(flash_filename, "r+b");
  if (s_flash_fp == NULL) {
    LOG_DEBUG("Setting up new flash file\n");
    s_flash_fp = fopen(flash_filename, "w+b");
    if (s_flash_fp == NULL) {
      LOG_DEBUG("Error: could not open flash file\n");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_FLASH_PAGES; i++) {
      flash_erase((FlashPage)i);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode flash_read(uintptr_t address, size_t read_bytes, uint8_t *buffer, size_t buffer_len) {
  if (buffer_len < read_bytes || address < FLASH_BASE_ADDR ||
      (address + read_bytes) > FLASH_END_ADDR || (intptr_t)address < 0) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  fseek(s_flash_fp, (intptr_t)address, SEEK_SET);
  size_t ret = fread(buffer, 1, read_bytes, s_flash_fp);
  (void)ret;

  return STATUS_CODE_OK;
}

StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (address < FLASH_BASE_ADDR || (address + buffer_len) > FLASH_END_ADDR ||
      (intptr_t)address < 0) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  } else if (buffer_len % FLASH_WRITE_BYTES != 0 || address % FLASH_WRITE_BYTES != 0) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t *read_buffer = malloc(buffer_len);

  fseek(s_flash_fp, (intptr_t)address, SEEK_SET);
  size_t read = fread(read_buffer, 1, buffer_len, s_flash_fp);
  (void)read;

  for (size_t i = 0; i < buffer_len; i++) {
    // STM32 does not overwriting at all - emulate behavior
    if (read_buffer[i] != 0xFF) {
      free(read_buffer);
      return status_msg(STATUS_CODE_INTERNAL_ERROR,
                        "Flash: Attempted to write to already written flash");
    }
  }

  free(read_buffer);

  fseek(s_flash_fp, (intptr_t)address, SEEK_SET);
  fwrite(buffer, 1, buffer_len, s_flash_fp);
  fflush(s_flash_fp);

  return STATUS_CODE_OK;
}

StatusCode flash_erase(FlashPage page) {
  if (page >= NUM_FLASH_PAGES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  char buffer[FLASH_PAGE_BYTES];
  memset(buffer, 0xFF, sizeof(buffer));

  fseek(s_flash_fp, (intptr_t)FLASH_PAGE_TO_ADDR(page), SEEK_SET);
  fwrite(buffer, 1, sizeof(buffer), s_flash_fp);
  fflush(s_flash_fp);

  return STATUS_CODE_OK;
}
