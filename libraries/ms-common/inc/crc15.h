#pragma once
// crc15 implementation for the LTC6811
#include <stddef.h>
#include <stdint.h>

void crc15_init_table(void);

uint16_t crc15_calculate(uint8_t *data, size_t len);
