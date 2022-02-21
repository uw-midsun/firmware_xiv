#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "log.h"

// Part 1.
// Based on I2C smoketest.

/*
For my reference:

"
Based on the MSXII pedal board schematics,
write a function that writes 0b0010010010100110
to the configuration register,
then reads the conversion register.

Hint: you’ll need to look at the schematics for the pedal board and the datasheet for the ADC.
"

// https://www.ti.com/lit/ds/symlink/ads1013.pdf
*/

#define ADC_I2C_ADDRESS 0x48
static const uint8_t bytes_to_write[] = { 0b00100100, 0b10100110 };
#define REGISTER_TO_WRITE 0x01  // ADC Configuration register
#define NUM_BYTES_TO_READ 2
uint8_t read_bytes[NUM_BYTES_TO_READ] = { 0 };
#define REGISTER_TO_READ 0x00  // ADC Conversion register

// These are the SDA and SCL ports for the I2C port 2.
#define I2C2_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C2_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

static void prv_initialize(void) {
  gpio_init();

  I2CSettings i2c2_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C2_SDA,
    .scl = I2C2_SCL,
  };
  i2c_init(I2C_PORT_2, &i2c2_settings);
}

void i2c_func(void) {
  prv_initialize();

  // Calculate the write length
  uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);

  StatusCode status;
  // Perform the write to a register
  status = i2c_write_reg(I2C_PORT_2, ADC_I2C_ADDRESS, REGISTER_TO_WRITE, bytes_to_write, tx_len);

  // Log a successful write
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Successfully wrote %d bytes to register %x at I2C address %x on I2C_PORT_%d\n",
              tx_len, REGISTER_TO_WRITE, ADC_I2C_ADDRESS, (I2C_PORT_2 == I2C_PORT_1) ? 1 : 2);
  }

  // Log an unsuccessful write
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Write failed: status code %d\n", status);
  }

  StatusCode status;
  // Perform the read from a register
  status =
      i2c_read_reg(I2C_PORT_2, ADC_I2C_ADDRESS, REGISTER_TO_READ, read_bytes, NUM_BYTES_TO_READ);

  // Log a successful read
  LOG_DEBUG("Successfully read %d bytes from register %x at I2C address %x on I2C_PORT_%d\n",
            NUM_BYTES_TO_READ, REGISTER_TO_READ, ADC_I2C_ADDRESS,
            (I2C_PORT_2 == I2C_PORT_1) ? 1 : 2);

  if (status == STATUS_CODE_OK) {
    // Log the bytes we read
    for (uint16_t i = 0; i < NUM_BYTES_TO_READ; i++) {
      LOG_DEBUG("Byte %x read: %x\n", i, read_bytes[i]);
    }
  } else {
    // Log an unsucessful read
    LOG_DEBUG("Read failed: status code %d\n", status);
  }

  return 0;
}

  // Part 2.
  // Based on SPI smoketest.

#include "spi.h"  // Normally included at top of file.
// I included it here to keep part 2 items together
#include "mcp2515_defs.h"

/*
Based on the MSXII charger interface board schematics,
write a function that initializes SPI then writes the following information to the CANCTRL
register of the CAN controller IC:
- Set loopback mode
- Do not request abort of transmit buffers
- Enable one shot mode
- Disable CLKOUT pin
- Set the CLKOUT prescaler to System Clock / 2.
Then, send the READ STATUS instruction and print the TXB1CNTRL[3] bit from the return
*/
// CAN Controller IC datasheet:
// https://ww1.microchip.com/downloads/en/DeviceDoc/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf

// Send WRITE cmd, the addr to write to, and the data to write
static uint8_t tx_bytes_ctrl[] = { MCP2515_CMD_WRITE, MCP2515_CTRL_REG_CANCTRL, 0b01010010 };

// Send READ_STATUS cmd
static uint8_t tx_bytes_read_status[] = { MCP2515_CMD_READ_STATUS };
#define EXPECTED_RESPONSE_LENGTH 1
static SpiPort port_to_use = SPI_PORT_2;

const SpiSettings settings_to_use = {
  .baudrate = 6000000,
  .mode = SPI_MODE_0,
  // Adjust GPIO pins as needed
  .mosi = { .port = GPIO_PORT_B, 15 },
  .miso = { .port = GPIO_PORT_B, 14 },
  .sclk = { .port = GPIO_PORT_B, 13 },
  .cs = { .port = GPIO_PORT_B, 12 },
};

void spi_func() {
  gpio_init();
  spi_init(port_to_use, &settings_to_use);

  // Set CANCTRL
  uint16_t tx_len = SIZEOF_ARRAY(tx_bytes_ctrl);
  uint8_t response[EXPECTED_RESPONSE_LENGTH] = { 0 };
  StatusCode status =
      spi_exchange(port_to_use, tx_bytes_ctrl, tx_len, response, EXPECTED_RESPONSE_LENGTH);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Successfully set CANCTRL");
  }

  // READ_STATUS
  tx_len = SIZEOF_ARRAY(tx_bytes_read_status);
  response[0] = 0;
  status =
      spi_exchange(port_to_use, tx_bytes_read_status, tx_len, response, EXPECTED_RESPONSE_LENGTH);
  if (status == STATUS_CODE_OK) {
    uint8_t txb1_cntrl = (response[0] >> 4) & 0x01;
    LOG_DEBUG("Successfully read, TXB1CNTRL[3] bit = %d/n", txb1_cntrl);
  }
}
