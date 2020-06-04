#include "spi.h"
#include "log.h"
#include "spi_mcu.h"

static GpioState cs_curr_state = GPIO_STATE_HIGH;

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  LOG_DEBUG("Note this is an x86 version of SPI\n");
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  } else if (settings->mode >= NUM_SPI_MODES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI mode.");
  }
  return STATUS_CODE_OK;
}

StatusCode spi_tx(SpiPort spi, uint8_t *tx_data, size_t tx_len) {
  LOG_DEBUG("Sending Data...\n");
  for (size_t i = 0; i < tx_len; i++) {
    LOG_DEBUG("%d\n", tx_data[i]);
  }
  return STATUS_CODE_OK;
}

StatusCode spi_rx(SpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder) {
  LOG_DEBUG("Recieving Data...\n");
  for (size_t i = 0; i < rx_len; i++) {
    // Insert dummy data
    if (i % 2 == 0) {
      rx_data[i] = 1;
    }
    LOG_DEBUG("%d\n", rx_data[i]);
  }
  return STATUS_CODE_OK;
}
StatusCode spi_cs_set_state(SpiPort spi, GpioState state) {
  cs_curr_state = state;
  if (state == GPIO_STATE_LOW) {
    LOG_DEBUG("CS state set to LOW\n");
  } else {
    LOG_DEBUG("CS state set to HIGH\n");
  }
  return STATUS_CODE_OK;
}

StatusCode spi_cs_get_state(SpiPort spi, GpioState *input_state) {
  if (cs_curr_state == GPIO_STATE_LOW) {
    LOG_DEBUG("CS state is LOW\n");
  } else {
    LOG_DEBUG("CS state is HIGH\n");
  }
  *input_state = cs_curr_state;
  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                        size_t rx_len) {
  if (spi >= NUM_SPI_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid SPI port.");
  }
  spi_cs_set_state(spi, GPIO_STATE_LOW);

  spi_tx(spi, tx_data, tx_len);

  spi_rx(spi, rx_data, rx_len, 0x00);

  spi_cs_set_state(spi, GPIO_STATE_HIGH);

  return STATUS_CODE_OK;
}
