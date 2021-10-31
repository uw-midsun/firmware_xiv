#include <stdbool.h>

#include "gpio.h"
#include "i2c.h"
#include "log.h"

#define WRITE_I2C_PORT I2C_PORT_2
#define WRITE_I2C_ADDRESS 0x48

static const uint8_t bytes_to_write[] = { 0x24, 0xA6 };

#define REGISTER_TO_WRITE 0x0C

#define READ_I2C_PORT I2C_PORT_2
#define READ_I2C_ADDRESS 0x48

#define NUM_BYTES_TO_READ 1

#define REGISTER_TO_READ 0x0C

#define I2C2_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C2_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

static void prv_initialize(void) {
  gpio_init();

  I2CSettings i2c2_settings = {
    .speed = I2C_SPEED_STANDARD,
    .sda = I2C2_SDA,
    .scl = I2C2_SCL,
  };
  i2c_init(I2C_PORT_2, &i2c2_settings);
}

int main(void) {
  prv_initialize();

  uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);

  StatusCode status;

  status =
      i2c_write_reg(WRITE_I2C_PORT, WRITE_I2C_ADDRESS, REGISTER_TO_WRITE, bytes_to_write, tx_len);

  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Successfully wrote %u byte(s) to register %x at I2C address %x on I2C_PORT_%d\n",
              tx_len, REGISTER_TO_WRITE, WRITE_I2C_ADDRESS, (WRITE_I2C_PORT == I2C_PORT_2) ? 2 : 1);
  } else {
    LOG_DEBUG("Write failed: status code %d\n", status);
  }

  uint8_t rx_data[NUM_BYTES_TO_READ] = { 0 };

  status =
      i2c_read_reg(READ_I2C_PORT, READ_I2C_ADDRESS, REGISTER_TO_READ, rx_data, NUM_BYTES_TO_READ);

  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Successfully read %u byte(s) to register %x at I2C address %x on I2C_PORT_%d\n",
              NUM_BYTES_TO_READ, REGISTER_TO_READ, READ_I2C_ADDRESS,
              (READ_I2C_PORT == I2C_PORT_2) ? 2 : 1);
  } else {
    LOG_DEBUG("Read failed: status code %d\n", status);
  }

  return 0;
}