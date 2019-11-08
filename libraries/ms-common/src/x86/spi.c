#include "spi.h"

StatusCode spi_init(SpiPort spi, const SpiSettings *settings) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                        size_t rx_len) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
