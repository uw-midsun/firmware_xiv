#include "log.h"
#include "spi.h"

#include "fw_104_spi.h"

StatusCode prv_write_read_spi_message() {
  StatusCode status = NUM_STATUS_CODES;

  SpiSettings settings = {
    // Setting the baud rate to 60000 as per the smoke_spi project
    .baudrate = 60000,
    .mode = SPI_MODE_0,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
  };

  // For settings outlined in FW 104: 0b01001001
  spi_init(SPI_PORT_2, &settings);
  uint8_t message = 0b10100000;

  uint8_t data = 0;

  status = spi_exchange(SPI_PORT_2, &message, sizeof(message), &data, sizeof(data));

  // Taking the 3rd bit from the data out (TXB0CNTRL)
  LOG_DEBUG("%d", data & 0b00000100);

  return status;
}
