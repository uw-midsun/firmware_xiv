#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "log.h"

// General I2C smoketest.

// To perform a write, set SHOULD_WRITE to true and fill in the write parameters.
// To perform a read, set SHOULD_READ to true and fill in the read parameters.
// If both SHOULD_WRITE and SHOULD_READ are true, the write will be performed before the read.

// You can use hex with 0x... or binary with 0b...
// If you get an error like "Read/Write failed with status code X", go to
// libraries/libcore/inc/status.h and look up the code by its position in the StatusCode enum;
// e.g. status code 8 is STATUS_CODE_UNIMPLEMENTED.

// I2C_PORT_1 is I2C1 on the controller board (PB9/PB8) and I2C_PORT_2 is I2C2 (PB11/PB10).

// ==== WRITE PARAMETERS ====

// Set this to true to perform an I2C write.
#define SHOULD_WRITE false

// Fill in these variables with the port and address to write to.
#define WRITE_I2C_PORT I2C_PORT_1  // or I2C_PORT_2
#define WRITE_I2C_ADDRESS 0x74

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0x10, 0x2F };

// Set this to true to write to a register or false to write normally.
#define SHOULD_WRITE_REGISTER false

// If the previous parameter is true, fill in this variable with the register to write to.
#define REGISTER_TO_WRITE 0x06

// ==== READ PARAMETERS ====

// Set this to true to perform an I2C read.
#define SHOULD_READ false

// Fill in these variables with the port and address to read from.
#define READ_I2C_PORT I2C_PORT_1  // or I2C_PORT_2
#define READ_I2C_ADDRESS 0x74

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 1

// Set this to true to read from a register or false to read normally.
#define SHOULD_READ_REGISTER false

// If the previous parameter is true, fill in this variable with the register to read from.
#define REGISTER_TO_READ 0x06

// ==== END OF PARAMETERS ====

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

int main(void) {
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

  return 0;
}
