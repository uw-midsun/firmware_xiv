#include "spv1020_mppt.h"
#include "spv1020_mppt_defs.h"

// This file implements stm32-specific functionality. See ../spv1020_mppt.c for the rest.

// Send a command to SPV1020 and get a response. |rx_len| must be <= 2.
// It's safe to pass NULL as |rx_data| if |rx_len| is 0.
static StatusCode prv_send_command(SpiPort port, uint8_t command, uint8_t *rx_data, size_t rx_len) {
  if (port >= NUM_SPI_PORTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // We don't use spi_exchange since that assumes 0x00 is NOP, whereas SPV1020 appears to use 0x01.
  spi_cs_set_state(port, GPIO_STATE_LOW);
  spi_tx(port, &command, 1);

  if (rx_len > 0) {
    spi_rx(port, rx_data, rx_len, COMMAND_NOP);
  }

  spi_cs_set_state(port, GPIO_STATE_HIGH);

  return STATUS_CODE_OK;
}

StatusCode spv1020_shut(SpiPort port) {
  return prv_send_command(port, COMMAND_SHUT, NULL, 0);
}

StatusCode spv1020_turn_on(SpiPort port) {
  return prv_send_command(port, COMMAND_TURN_ON, NULL, 0);
}

StatusCode spv1020_read_current(SpiPort port, uint16_t *current) {
  return prv_send_command(port, COMMAND_READ_CURRENT, (uint8_t *)current, 2);
}

StatusCode spv1020_read_voltage_in(SpiPort port, uint16_t *vin) {
  return prv_send_command(port, COMMAND_READ_VIN, (uint8_t *)vin, 2);
}

StatusCode spv1020_read_pwm(SpiPort port, uint16_t *pwm) {
  return prv_send_command(port, COMMAND_READ_PWM, (uint8_t *)pwm, 2);
}

StatusCode spv1020_read_status(SpiPort port, uint8_t *status) {
  return prv_send_command(port, COMMAND_READ_STATUS, status, 1);
}
