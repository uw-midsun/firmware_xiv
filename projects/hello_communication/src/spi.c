/**
 * Part 1: I2C

Based on the MSXII pedal board schematics, write a function that writes 0b0010010010100110 to
the configuration register, then reads the conversion register.

Hint: you’ll need to look at the schematics for the pedal board and the datasheet for the ADC.
Part 2: SPI

Based on the MSXII charger interface board schematics, write a function that initializes SPI
then writes the following information to the CANCTRL
register of the CAN controller IC:

    Set loopback mode

    Do not request abort of transmit buffers

    Enable one shot mode

    Disable CLKOUT pin

    Set the CLKOUT prescaler to System Clock / 2.

Then, send the READ STATUS instruction and print the TXB1CNTRL[3] bit from the return.
Part 3: CAN Communication

Write a function that periodically sends a CAN message with id 0xA and a random uint16_t
as the body, and requests an ACK with an OK status.
Write another function that periodically sends a CAN message with id 0xB and a different
random uint16_t as the body.

Then, register callbacks for both that print the data, but only ACK the 0xA message.

Run the program in two terminals at the same time, and send a screenshot of the output
to your lead.
**/

#include "spi.h"
#include "gpio.h"
#include "log.h"

// This project is for smoke testing SPI.

// Fill in the parameters as needed.
// You can use hex with 0x... or binary with 0b...

#define READ_STATUS = 0b10100000

// FILL IN THIS PACKAGE WITH THE BYTES TO SEND
static uint8_t tx_bytes[] = { 0b01010010, 0b10100000 };

// FILL IN THIS VARIABLE WITH THE EXPECTED RESPONSE LENGTH
#define EXPECTED_RESPONSE_LENGTH 2

// FILL THIS VARIABLE WITH THE DESIRED SPI PORT
static SpiPort port_to_use = 3;

const SpiSettings settings_to_use = {
  .baudrate = 60000,
  .mode = SPI_MODE_0,
  // Adjust GPIO pins as needed
  .mosi = { .port = 3, .pin = 7 },
  .miso = { .port = 3, .pin = 8 },
  .sclk = { .port = 3, .pin = 9 },
  .cs = { .port = 3, .pin = 10 },
};

int main(void) {
  gpio_init();
  spi_init(port_to_use, &settings_to_use);

  // Calculate transmission length
  uint16_t tx_len = SIZEOF_ARRAY(tx_bytes);

  // Allocate space for response
  uint8_t response[EXPECTED_RESPONSE_LENGTH] = { 0 };

  uint8_t cntrl_response[EXPECTED_RESPONSE_LENGTH] = { 0 };

  // Do the exchange
  spi_exchange(port_to_use, tx_bytes[0], tx_len, response, EXPECTED_RESPONSE_LENGTH);

  // send READ STATUS instruction
  spi_exchange(port_to_use, tx_bytes[1], tx_len, cntrl_response, EXPECTED_RESPONSE_LENGTH);

  uint8_t txbit = cntrl_response[6];

  // Log the output
  for (uint16_t i = 0; i < EXPECTED_RESPONSE_LENGTH; i++) {
    LOG_DEBUG("Byte %x of response: %x\n", i, response[i]);
  }
}
