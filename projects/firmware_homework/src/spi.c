#include "spi_104.h"

#include "gpio.h"
#include "log.h"
#include "spi.h"

StatusCode send_spi_message(void) {
  // SPI initialization
  static SpiPort port_to_use = SPI_PORT_2;
  // MS12 schematics for charger interface
  // do not exist anymore so I am just using MS14 pins
  const SpiSettings settings = { .baudrate = 1000000,  // 1 Mhz
                                 .mode = SPI_MODE_0,
                                 .mosi = { .port = GPIO_PORT_B, 15 },
                                 .miso = { .port = GPIO_PORT_B, 14 },
                                 .sclk = { .port = GPIO_PORT_B, 13 },
                                 .cs = { .port = GPIO_PORT_B, 12 } };

  spi_init(port_to_use, &settings);
  // Bits 7-5 are operation mode bits, 010 is loopback
  // Bit 4 is transmission bit, 0 terminates request
  // Bit 3 is one shot mode bit, 1 is enabled
  // Bit 2 is clock enable bit, 0 is disabled
  // Bits 1-0 are prescaler bits, 01 is clock/2
  uint8_t spi_message = 0b01001001;
  uint8_t rx_data = 0;

  // Send the SPI message
  status_ok_or_return(
      spi_exchange(port_to_use, &spi_message, sizeof(spi_message), &rx_data, sizeof(rx_data)));

  // Print control bit, bit three
  LOG_DEBUG("Control bit %d\n", (rx_data & 4) >> 2);

  return STATUS_CODE_OK;
}
