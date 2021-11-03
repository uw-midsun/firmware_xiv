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

static void prv_i2c_initialize(void) {
  gpio_init();

  I2CSettings i2c2_settings = {
    .speed = I2C_SPEED_STANDARD,
    .sda = I2C2_SDA,
    .scl = I2C2_SCL,
  };
  i2c_init(I2C_PORT_2, &i2c2_settings);
}

int i2c(void) {
  prv_i2c_initialize();

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

#include "spi.h"

static const uint8_t canctrl_info_tx[] = { 0b01001001 };
static const uint8_t read_status_tx[] = { 0b00000011 };

#define EXPECTED_RESPONSE_LENGTH 2

static SpiPort port_to_use = SPI_PORT_2;

const SpiSettings settings_to_use = {
  .baudrate = 6000000,  // Not sure what to set, copied spi smoke test
  .mode = SPI_MODE_0,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

// Confused on how to write to CANCTRL register, because unlike i2c,
// there's no spi write-to-register function where an address is used
int spi(void) {
  gpio_init();
  spi_init(port_to_use, &settings_to_use);

  StatusCode status;

  uint16_t tx_len = SIZEOF_ARRAY(canctrl_info_tx);

  const uint8_t response[EXPECTED_RESPONSE_LENGTH] = { 0 };

  status = spi_tx(port_to_use, canctrl_info_tx, tx_len);

  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Successfully wrote %d byte(s) to spi port %d\n", tx_len,
              (port_to_use == SPI_PORT_1) ? 1 : 2);
  } else {
    LOG_DEBUG("Error: Could not write to spi port %d\n", port_to_use);
  }

  tx_len = SIZEOF_ARRAY(read_status_tx);

  status = spi_exchange(port_to_use, read_status_tx, tx_len, response, EXPECTED_RESPONSE_LENGTH);
  LOG_DEBUG("TXB1CNTRL[3] bit: %d\n", (*response >> 4) & 1);

  return 0;
}
// Comment to allow other file to run
// int main(void) {
//   i2c();
//   spi();
// }
