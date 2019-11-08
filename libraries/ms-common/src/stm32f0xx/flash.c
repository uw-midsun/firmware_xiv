#include "flash.h"
#include <string.h>
#include "critical_section.h"
#include "stm32f0xx.h"

StatusCode flash_init(void) {
  // Flash prefetch and latency are set up in system init
  return STATUS_CODE_OK;
}

StatusCode flash_read(uintptr_t address, size_t read_bytes, uint8_t *buffer, size_t buffer_len) {
  if (buffer_len < read_bytes || address < FLASH_BASE_ADDR ||
      (address + read_bytes) > FLASH_END_ADDR) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // may want to move all flash-related functions into RAM so we don't block the
  // CPU
  memcpy(buffer, (void *)address, read_bytes);

  return STATUS_CODE_OK;
}

StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (address < FLASH_BASE_ADDR || (address + buffer_len) > FLASH_END_ADDR) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  } else if (buffer_len % FLASH_WRITE_BYTES != 0 || address % FLASH_WRITE_BYTES != 0) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t *data = (uint32_t *)buffer;
  size_t data_len = buffer_len / FLASH_WRITE_BYTES;

  bool disabled = critical_section_start();

  FLASH_Unlock();
  FLASH_Status status = FLASH_COMPLETE;
  for (size_t i = 0; status == FLASH_COMPLETE && i < data_len; i++) {
    status = FLASH_ProgramWord(address + (i * FLASH_WRITE_BYTES), data[i]);
  }
  FLASH_Lock();

  critical_section_end(disabled);

  if (status != FLASH_COMPLETE) {
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  return STATUS_CODE_OK;
}

StatusCode flash_erase(FlashPage page) {
  if (page >= NUM_FLASH_PAGES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  bool disabled = critical_section_start();

  FLASH_Unlock();
  FLASH_Status status = FLASH_ErasePage(FLASH_PAGE_TO_ADDR(page));
  FLASH_Lock();

  critical_section_end(disabled);

  if (status != FLASH_COMPLETE) {
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  return STATUS_CODE_OK;
}
