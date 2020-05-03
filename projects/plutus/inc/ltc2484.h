#pragma once
// LTC2484 ADC internal-only header
//
// Application code should not need anything here (use ltc_adc.h instead).

#include <stdint.h>

#include "status.h"

// Technically the maximum is 163.5ms, which occurs in 50Hz mode
// We just add 40ms to be safe (which is plenty, especially since soft-timers
// should never under-count the time duration
#define LTC2484_MAX_CONVERSION_TIME_MS (200)

// commands
// see datasheet Table 1 (p. 14) for these command values
#define LTC2484_ENABLE (1 << 7)
#define LTC2484_DISABLE (0 << 7)

#define LTC2484_EXTERNAL_INPUT (0 << 3)
#define LTC2484_TEMPERATURE_INPUT (1 << 3)

#define LTC2484_REJECTION_50HZ_60HZ 0x00
#define LTC2484_REJECTION_50HZ 0x02
#define LTC2484_REJECTION_60HZ 0x04

#define LTC2484_AUTO_CALIBRATION (0 << 0)
#define LTC2484_SPEED_2X (1 << 0)

// constants
// According to the datasheet, the maximum time that a conversion will take is
// 163.5ms, so we choose 200ms to be conservative
#define LTC2484_CONVERSION_TIMEOUT_MS 200

#define LTC2484_OVERRANGE_CODE ((1 << 5) | (1 << 4))
#define LTC2484_UNDERRANGE_CODE ((1 << 1) | (1 << 2) | (1 << 3))
// We mask to check bits EOC, DMY, SIG, MSB, B27, B26, B25
// (see Table 3 on p.16)
#define LTC2484_ERROR_CODE_MASK 0xFE
#define LTC2484_V_REF_MILLIVOLTS 4092

typedef union {
  uint8_t u8data[4];
  int32_t i32data;
} Ltc2484Response;

// Parse the raw 24-bit ADC reading to a voltage measurement in uV.
StatusCode ltc2484_raw_adc_to_uv(uint8_t *spi_data, int32_t *voltage);
