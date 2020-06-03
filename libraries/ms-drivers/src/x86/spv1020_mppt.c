#include "spv1020_mppt.h"
#include "log.h"

// This file implements an x86 approximation of the stm32-specific functionality.
// See ../spv1020_mppt.c for the shared functions.

// These are "reasonable" values of the inputs.
#define FIXED_CURRENT 512  // middle of the 10-bit current range
#define FIXED_VIN 720      // 70% of max (2^10), about the maximum power point
#define FIXED_PWM 500      // a duty cycle of 50%
#define FIXED_STATUS 0

static bool s_is_shut = false;

StatusCode spv1020_shut(SpiPort port) {
  if (port >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // this doesn't occur on stm32 but is useful for debugging
  if (s_is_shut) {
    LOG_WARN("Issuing SHUT command to SPV1020 MPPT when it's already shut\n");
  }
  s_is_shut = true;

  // just log it and move on
  LOG_DEBUG("SPV1020 MPPT command: SHUT\n");
  return STATUS_CODE_OK;
}

StatusCode spv1020_turn_on(SpiPort port) {
  if (port >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_is_shut) {
    LOG_WARN("Issuing Turn ON command to SPV1020 MPPT before a SHUT command\n");
  }
  s_is_shut = false;

  LOG_DEBUG("SPV1020 MPPT command: Turn ON\n");
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_current(SpiPort port, uint16_t *current) {
  LOG_DEBUG("SPV1020 MPPT command: Read current\n");
  *current = FIXED_CURRENT;
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_voltage_in(SpiPort port, uint16_t *vin) {
  LOG_DEBUG("SPV1020 MPPT command: Read vin\n");
  *vin = FIXED_VIN;
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_pwm(SpiPort port, uint16_t *pwm) {
  LOG_DEBUG("SPV1020 MPPT command: Read pwm\n");
  *pwm = FIXED_PWM;
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_status(SpiPort port, uint8_t *status) {
  LOG_DEBUG("SPV1020 MPPT command: Read status\n");
  *status = FIXED_STATUS;
  return STATUS_CODE_OK;
}
