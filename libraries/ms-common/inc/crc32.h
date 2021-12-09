#pragma once
// CRC32 module
// Polynomial 0x04C11DB7, initial value 0xFFFFFFFF - standard CRC32 model
//
// Cyclic redundancy check - error detecting code commonly used as a quick hash
// function. Note that this is designed to be used to verify data integrity, and
// should not be used to protect against intentional alteration of data.
#include <stddef.h>
#include <stdint.h>

#include "status.h"

StatusCode crc32_init(void);

uint32_t crc32_arr(const uint8_t *buffer, size_t buffer_len);

// continue calculating crc32 from a
uint32_t crc32_append_arr(const uint8_t *buffer, size_t buffer_len, uint32_t crc_state);
