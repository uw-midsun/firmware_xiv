#include "spv1020_mppt.h"
#include "spv1020_mppt_defs.h"

// This file implements stm32-specific functionality. See ../spv1020_mppt.c for the rest.

// Send a command to SPV1020 and get a response.
// It's safe to pass NULL as |rx_data| if |rx_len| is 0.
static StatusCode prv_send_command(SpiPort port, uint8_t command, uint8_t *rx_data, size_t rx_len) {
  if (port >= NUM_SPI_PORTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Ensure rx_data can be NULL for rx_len==0 (spi_exchange should be safe, but this is more robust)
  uint8_t placeholder;
  if (rx_len == 0) {
    rx_data = &placeholder;
  }

  // Note: it's possible that the SPV1020 uses 0x01 as the NOP instead of 0x00 as used by
  // spi_exchange, in which case we need a weird custom SPI implementation to use 0x01.
  // The command table gives the NOP command as 0x01, but the SPI implementaiton section says 0x00.

  return spi_exchange(port, &command, 1, rx_data, rx_len);
}

// Helper to send a command that expects a 16-bit reply.
static StatusCode prv_send_command_uint16_reply(SpiPort port, uint8_t command, uint16_t *reply) {
  uint8_t rx_data[2] = { 0 };
  StatusCode code = prv_send_command(port, command, rx_data, 2);
  *reply = (rx_data[0] << 8) | rx_data[1];
  return code;
}

StatusCode spv1020_shut(SpiPort port) {
  return prv_send_command(port, SPV1020_CMD_SHUT, NULL, 0);
}

StatusCode spv1020_turn_on(SpiPort port) {
  return prv_send_command(port, SPV1020_CMD_TURN_ON, NULL, 0);
}

StatusCode spv1020_read_current(SpiPort port, uint16_t *current) {
  return prv_send_command_uint16_reply(port, SPV1020_CMD_READ_CURRENT, current);
}

StatusCode spv1020_read_voltage_in(SpiPort port, uint16_t *vin) {
  return prv_send_command_uint16_reply(port, SPV1020_CMD_READ_VIN, vin);
}

StatusCode spv1020_read_pwm(SpiPort port, uint16_t *pwm) {
  uint16_t raw_pwm;
  StatusCode code = prv_send_command_uint16_reply(port, SPV1020_CMD_READ_PWM, &raw_pwm);

  // Convert the raw 9-bit value to a permille. The application note says that the PWM duty cycle
  // ranges from 5% to 90% with a step of 0.2%, giving 425 values; we assume that the raw value
  // read from the SPV1020 is the step number, where 0 is 5% and 425 is 90%, and each change of 1
  // represents 0.2%. Then duty cycle = (raw value * 0.2%) + 5%, probably.
  *pwm = (raw_pwm * 2) + 50;

  return code;
}

StatusCode spv1020_read_status(SpiPort port, uint8_t *status) {
  return prv_send_command(port, SPV1020_CMD_READ_STATUS, status, 1);
}
