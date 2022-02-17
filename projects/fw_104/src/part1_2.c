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
