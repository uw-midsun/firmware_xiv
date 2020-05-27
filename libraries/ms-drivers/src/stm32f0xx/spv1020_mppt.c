#include "spv1020_mppt.h"
#include "spv1020_mppt_defs.h"

// Send a command to SPV1020 and get a response. |rx_len| must be <= 2.
// It's safe to pass NULL as |rx_data| if |rx_len| is 0.
static StatusCode prv_send_command(SpiPort port, uint8_t command, uint16_t *rx_data, size_t rx_len) {
  if (port >= NUM_SPI_PORTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // We don't use spi_exchange since that assumes 0x00 is NOP, whereas SPV1020 appears to use 0x01.
  spi_cs_set_state(port, GPIO_STATE_LOW);
  spi_tx(port, &command, 1);

  uint8_t rx_bytes[2];
  spi_rx(port, rx_bytes, rx_len, COMMAND_NOP);
  spi_cs_set_state(port, GPIO_STATE_LOW);

  if (rx_len > 0) {
    // pack the rx_bytes into a uint16_t in big-endian order
    *rx_data = 0;
    for (uint8_t i = 0; i < rx_len; i++) {
      *rx_data = (*rx_data << 8) | rx_bytes[i];
    }
  }

  return STATUS_CODE_OK;
}

StatusCode spv1020_shut(SpiPort port) {
  return prv_send_command(port, COMMAND_SHUT, NULL, 0);
}

StatusCode spv1020_turn_on(SpiPort port) {
  return prv_send_command(port, COMMAND_TURN_ON, NULL, 0);
}

StatusCode spv1020_read_current(SpiPort port, uint16_t *current) {
  return prv_send_command(port, COMMAND_READ_CURRENT, current, 2);
}

StatusCode spv1020_read_voltage_in(SpiPort port, uint16_t *vin) {
  return prv_send_command(port, COMMAND_READ_VIN, vin, 2);
}

StatusCode spv1020_read_pwm(SpiPort port, uint16_t *pwm) {
  return prv_send_command(port, COMMAND_READ_PWM, pwm, 2);
}

StatusCode spv1020_read_status(SpiPort port, uint8_t *status) {
  return prv_send_command(port, COMMAND_READ_STATUS, status, 1);
}

bool spv1020_is_overcurrent(uint8_t status, uint8_t *ovc_branches) {
  // this is the number of bits to shift to get the OVC bits of the bitmask as the LSBs
  const uint8_t ovc_bitshift = __builtin_ctz(OVC_BITMASK);
  
  ovc_branches = (status & OVC_BITMASK) >> ovc_bitshift;
  return ovc_branches != 0;
}

bool spv1020_is_overvoltage(uint8_t status) {
  return (status & OVV_BITMASK) != 0;
}

bool spv1020_is_overtemperature(uint8_t status) {
  return (status & OVT_BITMASK) != 0;
}

// What should we do about the CR bit (or bits)? Ask Micah.
