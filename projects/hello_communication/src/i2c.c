#include "i2c.h"
#include <stdbool.h>
#include "gpio.h"
#include "log.h"

// ==== WRITE PARAMETERS ====

// Set this to true to perform an I2C write.
#define SHOULD_WRITE true

// Fill in these variables with the port and address to write to.
#define WRITE_I2C_PORT 4
#define WRITE_I2C_ADDRESS 0x48

// Fill in this array with the bytes to write.
static const uint8_t bytes_to_write[] = { 0b0010010010100110 };

// Set this to true to write to a register or false to write normally.
#define SHOULD_WRITE_REGISTER true

// If the previous parameter is true, fill in this variable with the register to write to.
#define REGISTER_TO_WRITE 0b00000001  // config register

// ==== READ PARAMETERS ====

// Set this to true to perform an I2C read.
#define SHOULD_READ true

// Fill in these variables with the port and address to read from.
#define READ_I2C_PORT 4
#define READ_I2C_ADDRESS 0x48

// Fill in this variable with the number of bytes to read.
#define NUM_BYTES_TO_READ 16

// Set this to true to read from a register or false to read normally.
#define SHOULD_READ_REGISTER true

// If the previous parameter is true, fill in this variable with the register to read from.
#define REGISTER_TO_READ 0b00000000  // conversion register

// ==== END OF PARAMETERS ====

// These are the SDA and SCL ports for the I2C ports 1 and 2.
#define I2C1_SDA \
  { .port = 4, .pin = 13 }
#define I2C1_SCL \
  { .port = 4, .pin = 14 }

static void prv_initialize(void) {
  gpio_init();

  I2CSettings i2c1_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C1_SDA,
    .scl = I2C1_SCL,
  };
  i2c_init(WRITE_I2C_PORT, &i2c1_settings);
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
