#include "gpio.h"
#include "log.h"
#include "spi.h"

// This project is for smoke testing SPI.

// Fill in the parameters as needed.
// You can use hex with 0x... or binary with 0b...

// FILL IN THIS PACKAGE WITH THE BYTES TO SEND
static uint8_t tx_bytes[] = { 0x88 };

// FILL IN THIS VARIABLE WITH THE EXPECTED RESPONSE LENGTH
#define EXPECTED_RESPONSE_LENGTH 1

// FILL THIS VARIABLE WITH THE DESIRED SPI PORT
static SpiPort port_to_use = SPI_PORT_1;

const SpiSettings settings_to_use = {
  .baudrate = 6000000,
  .mode = SPI_MODE_1,
  // Adjust GPIO pins as needed
  .mosi = { .port = GPIO_PORT_A, 7 },
  .miso = { .port = GPIO_PORT_A, 6 },
  .sclk = { .port = GPIO_PORT_A, 5 },
  .cs = { .port = GPIO_PORT_A, 4 },
};

int main(void) {
  gpio_init();
  spi_init(port_to_use, &settings_to_use);

  // Calculate transmission length
  uint16_t tx_len = SIZEOF_ARRAY(tx_bytes);

  // Allocate space for response
  uint8_t response[EXPECTED_RESPONSE_LENGTH] = { 0 };

  // Do the exchange
  spi_exchange(port_to_use, tx_bytes, tx_len, response, EXPECTED_RESPONSE_LENGTH);

  // Log the output
  for (uint16_t i = 0; i < EXPECTED_RESPONSE_LENGTH; i++) {
    LOG_DEBUG("Byte %x of response: %x\n", i, response[i]);
  }
}
