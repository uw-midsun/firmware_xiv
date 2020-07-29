#pragma once

// Contains enums used for distinguishing the solar boards.

#include <stdint.h>

typedef enum SolarMpptCount {
  SOLAR_BOARD_5_MPPTS = 5,
  SOLAR_BOARD_6_MPPTS = 6,
  MAX_SOLAR_BOARD_MPPTS = 6,
} SolarMpptCount;

// The minimum number of bits of a valid MPPT number, equal to ceil(log2(MAX_SOLAR_BOARD_MPPTS)).
#define MAX_MPPT_BIT_WIDTH 3

// This should be used to distinguish between the MPPTs.
typedef uint8_t Mppt;
