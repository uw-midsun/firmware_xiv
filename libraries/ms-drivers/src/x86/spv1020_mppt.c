#include "spv1020_mppt.h"
#include "log.h"

// This file implements an x86 approximation of the stm32-specific functionality.
// See ../spv1020_mppt.c for the shared functions.

// TODO: ask Micah what "reasonable" voltages/currents/pwm are.
// These are in the middle of their ranges.
#define FIXED_CURRENT 0x200
#define FIXED_VIN 0x200
#define FIXED_PWM 0x100
#define FIXED_STATUS 0

static bool s_is_shut = false;

StatusCode spv1020_shut(SpiPort port) {
  if (port >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  
  // just log it and move on
  s_is_shut = true;
  LOG_DEBUG("SPV1020 MPPT command: SHUT\r\n");
  return STATUS_CODE_OK;
}

StatusCode spv1020_turn_on(SpiPort port) {
  if (port >= NUM_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  
  // this doesn't occur on stm32 but is useful for debugging
  if (!s_is_shut) {
    LOG_WARN("Issuing Turn ON command to SPV1020 MPPT before a SHUT command\r\n");
  }
  s_is_shut = false;
  
  LOG_DEBUG("SPV1020 MPPT command: Turn ON\r\n");
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_current(SpiPort port, uint16_t *current) {
  LOG_DEBUG("SPV1020 MPPT command: Read current\r\n");
  *current = FIXED_CURRENT;
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_voltage_in(SpiPort port, uint16_t *vin) {
  LOG_DEBUG("SPV1020 MPPT command: Read vin\r\n");
  *vin = FIXED_VIN;
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_pwm(SpiPort port, uint16_t *pwm) {
  LOG_DEBUG("SPV1020 MPPT command: Read pwm\r\n");
  *pwm = FIXED_PWM;
  return STATUS_CODE_OK;
}

StatusCode spv1020_read_status(SpiPort port, uint8_t *status) {
  LOG_DEBUG("SPV1020 MPPT command: Read status\r\n");
  *status = FIXED_STATUS;
  return STATUS_CODE_OK;
}
