#include "spi.h"

#include "gpio.h"
#include "log.h"

static SpiPort port_to_use = SPI_PORT_2;

static const uint8_t tx_bytes[] = { 0b01001001 };

const SpiSettings settings_to_use = {
  .baudrate = 6000000,
  .mode = SPI_MODE_0,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

int main(void) {
  gpio_init();
  spi_init(port_to_use, &settings_to_use);

  StatusCode status;
  const uint16_t tx_len = SIZEOF_ARRAY(tx_bytes);

  status = spi_tx(port_to_use, tx_bytes, tx_len);

  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Successfully wrote %d byte(s) to spi port %d\n", tx_len,
              (port_to_use == SPI_PORT_1) ? 1 : 2);
  } else {
    LOG_DEBUG("Error: Could not write to spi port %d\n", port_to_use);
  }
}