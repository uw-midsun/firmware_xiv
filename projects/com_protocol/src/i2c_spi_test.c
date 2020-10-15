#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "spi.h"

// Part 1: I2C

// Set this to true to perform an I2C write.
#define SHOULD_WRITE true
#define WRITE_I2C_PORT I2C_PORT_1  // or I2C_PORT_2
#define WRITE_I2C_ADDRESS 0x48

// write 0b0010010010100110 split into 0b00100100, 0b10100110
static const uint8_t bytes_to_write[] = { 0b00100100, 0b10100110 };

// Write to Config register 0b10010000
#define SHOULD_WRITE_REGISTER true
#define REGISTER_TO_WRITE 0b10010000

// ==== READ PARAMETERS ====

// Set this to true to perform an I2C read.
#define SHOULD_READ true
#define READ_I2C_PORT I2C_PORT_1  // or I2C_PORT_2
#define READ_I2C_ADDRESS 0x48

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 1

// Read Conversion register 0b10010001
#define SHOULD_READ_REGISTER true
#define REGISTER_TO_READ 0b10010001

// These are the SDA and SCL ports for the I2C ports 1 and 2.
#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 9 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 8 }
#define I2C2_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C2_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

static void prv_initialize(void) {
  gpio_init();

  I2CSettings i2c1_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C1_SDA,
    .scl = I2C1_SCL,
  };
  i2c_init(I2C_PORT_1, &i2c1_settings);

  I2CSettings i2c2_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C2_SDA,
    .scl = I2C2_SCL,
  };
  i2c_init(I2C_PORT_2, &i2c2_settings);
}
static void prv_write_read_i2c(void) {
  prv_initialize();
  if (SHOULD_WRITE) {
    // Calculate the write length
    uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);

    StatusCode status;
    if (SHOULD_WRITE_REGISTER) {
      // Perform the write to a register
      status = i2c_write_reg(WRITE_I2C_PORT, WRITE_I2C_ADDRESS, REGISTER_TO_WRITE, bytes_to_write,
                             tx_len);

      // Log a successful write
      if (status == STATUS_CODE_OK) {
        LOG_DEBUG("Successfully wrote %d bytes to register %x at I2C address %x on I2C_PORT_%d\n",
                  tx_len, REGISTER_TO_WRITE, WRITE_I2C_ADDRESS,
                  (WRITE_I2C_PORT == I2C_PORT_1) ? 1 : 2);
      }
    } else {
      // Perform the write normally
      status = i2c_write(WRITE_I2C_PORT, WRITE_I2C_ADDRESS, bytes_to_write, tx_len);

      // Log a successful write
      if (status == STATUS_CODE_OK) {
        LOG_DEBUG("Successfully wrote %d bytes to I2C address %x on I2C_PORT_%d\n", tx_len,
                  WRITE_I2C_ADDRESS, (WRITE_I2C_PORT == I2C_PORT_1) ? 1 : 2);
      }
    }

    // Log an unsuccessful write
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Write failed: status code %d\n", status);
    }
  }

  if (SHOULD_READ) {
    // Allocate space for the bytes we'll read
    uint8_t read_bytes[NUM_BYTES_TO_READ] = { 0 };

    StatusCode status;
    if (SHOULD_READ_REGISTER) {
      // Perform the read from a register
      status = i2c_read_reg(READ_I2C_PORT, READ_I2C_ADDRESS, REGISTER_TO_READ, read_bytes,
                            NUM_BYTES_TO_READ);

      // Log a successful read
      LOG_DEBUG("Successfully read %d bytes from register %x at I2C address %x on I2C_PORT_%d\n",
                NUM_BYTES_TO_READ, REGISTER_TO_READ, READ_I2C_ADDRESS,
                (READ_I2C_PORT == I2C_PORT_1) ? 1 : 2);
    } else {
      // Perform the read normally
      status = i2c_read(READ_I2C_PORT, READ_I2C_ADDRESS, read_bytes, NUM_BYTES_TO_READ);

      // Log a successful read
      LOG_DEBUG("Successfully read %d bytes from I2C address %x on I2C_PORT_%d\n",
                NUM_BYTES_TO_READ, READ_I2C_ADDRESS, (READ_I2C_PORT == I2C_PORT_1) ? 1 : 2);
    }

    if (status == STATUS_CODE_OK) {
      // Log the bytes we read
      for (uint16_t i = 0; i < NUM_BYTES_TO_READ; i++) {
        LOG_DEBUG("Byte %x read: %x\n", i, read_bytes[i]);
      }
    } else {
      // Log an unsucessful read
      LOG_DEBUG("Read failed: status code %d\n", status);
    }
  }
}
// Part 2: SPI
// first byte writes info to CANCTRL, second sends READ STATUS instruction
static uint8_t tx_bytes[] = { 0b01001001, 0b10100000 };

// respose is repeated twice, only need to look at first one
#define EXPECTED_RESPONSE_LENGTH 2

static SpiPort port_to_use = SPI_PORT_2;

const SpiSettings settings_to_use = {
  .baudrate = 60000,
  .mode = SPI_MODE_0,
  // Adjust GPIO pins as needed
  .mosi = { .port = GPIO_PORT_B, 15 },
  .miso = { .port = GPIO_PORT_B, 14 },
  .sclk = { .port = GPIO_PORT_B, 13 },
  .cs = { .port = GPIO_PORT_B, 12 },
};

static void prv_write_read_spi(void) {
  gpio_init();
  spi_init(port_to_use, &settings_to_use);

  // Calculate transmission length
  uint16_t tx_len = SIZEOF_ARRAY(tx_bytes);

  // Allocate space for response
  uint8_t response[EXPECTED_RESPONSE_LENGTH] = { 0 };

  // Do the exchange
  spi_exchange(port_to_use, tx_bytes, tx_len, response, EXPECTED_RESPONSE_LENGTH);

  // print the TXB1CNTRL[3] bit from the return.
  if ((response[0] & 0b00001000) == 0b00001000) {
    LOG_DEBUG("TXB1CNTRL[3] bit: 1\n");
  } else {
    LOG_DEBUG("TXB1CNTRL[3] bit: 0\n");
  }
}
