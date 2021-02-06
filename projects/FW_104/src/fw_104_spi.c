#include "fw_104_spi.h"

#include "controller_board_pins.h"
#include "log.h"
#include "spi.h"

StatusCode write_read_spi_message(void) {
  StatusCode status = NUM_STATUS_CODES;

  SpiSettings settings = {
    // Setting the baud rate to 60000 as per the smoke_spi project
    // ^ The smoke_spi project uses a different board, the maximum clock frequency for MCP2515 is
    // 10MHz
    .baudrate = 10000000,
    .mode = SPI_MODE_0,
    .mosi = CONTROLLER_BOARD_ADDR_SPI2_MOSI,
    .miso = CONTROLLER_BOARD_ADDR_SPI2_MISO,
    .sclk = CONTROLLER_BOARD_ADDR_SPI2_SCK,
    .cs = CONTROLLER_BOARD_ADDR_SPI2_NSS,
  };

  // For settings outlined in FW 104: 0b01001001
  spi_init(SPI_PORT_2, &settings);
  uint8_t message = 0b01001001;

  uint8_t data = 0;

  status = spi_exchange(SPI_PORT_2, &message, sizeof(message), &data, sizeof(data));

  // Taking the 3rd bit from the data out (TXB0CNTRL)
  LOG_DEBUG("%d\n", (data & 0b00000100) >> 2);

  return status;
}
