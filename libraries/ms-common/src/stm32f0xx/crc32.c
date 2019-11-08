#include "crc32.h"
#include "log.h"
#include "stm32f0xx.h"

StatusCode crc32_init(void) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
  CRC_ReverseInputDataSelect(CRC_ReverseInputData_32bits);
  CRC_ReverseOutputDataCmd(ENABLE);

  return STATUS_CODE_OK;
}

uint32_t crc32_arr(const uint8_t *buffer, size_t buffer_len) {
  CRC_ResetDR();

  // The CRC32 peripheral consumes words (u32) by default - split into u32 and
  // remaining bytes so we can process the remaining bytes separately
  size_t num_u32 = buffer_len / sizeof(uint32_t);
  size_t remaining_bytes = buffer_len % sizeof(uint32_t);
  uint32_t crc = CRC_CalcBlockCRC((const uint32_t *)buffer, num_u32);

  // Consume remaining bytes that did not take up a whole word
  const uint8_t *data = buffer + (num_u32 * sizeof(uint32_t));
  for (size_t i = 0; i < remaining_bytes; i++) {
    crc = CRC_CalcCRC8bits(data[i]);
  }

  return ~crc;
}
