#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "log.h"

#define ADC_I2C_PORT I2C_PORT_2
#define ADC_I2C_ADDRESS 0x48  // ADS101x i2c address of 1001000

#define REGISTER_TO_WRITE 0x1  // 0b00000001 points to Config register
#define REGISTER_TO_READ 0x0   // 0b00000000 points to Conversion register

#define NUM_BYTES_TO_READ 2  // ADS101x responds with two bytes of data

void i2c_func(void) {
  const uint8_t bytes_to_write[] = { 0x24, 0xA6 };
  uint16_t tx_len = SIZEOF_ARRAY(bytes_to_write);

  gpio_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_STANDARD,
    .sda = { .port = GPIO_PORT_B, .pin = 10 },
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
  };

  i2c_init(I2C_PORT_2, &i2c_settings);

  StatusCode status;
  status = i2c_write_reg(ADC_I2C_PORT, ADC_I2C_ADDRESS, REGISTER_TO_WRITE, bytes_to_write, tx_len);

  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Successfully wrote %d bytes to I2C address %x on I2C_PORT_%d\n", tx_len,
              ADC_I2C_ADDRESS, 2);
  } else {
    LOG_DEBUG("Failed to write  %d bytes to I2C address %x on I2C_PORT_%d\n", tx_len,
              ADC_I2C_ADDRESS, 2);
  }

  uint8_t read_bytes[NUM_BYTES_TO_READ] = { 0 };
  status =
      i2c_read_reg(ADC_I2C_PORT, ADC_I2C_ADDRESS, REGISTER_TO_READ, read_bytes, NUM_BYTES_TO_READ);

  if (status == STATUS_CODE_OK) {
    for (uint8_t i = 0; i < NUM_BYTES_TO_READ; i++) {
      LOG_DEBUG("Byte %x read: %x\n", i, read_bytes[i]);
    }
  } else {
    LOG_DEBUG("Read failed");
  }
}

#include "spi.h"
#define EXPECTED_RESPONSE_LENGTH 1  // CAN STATUS REGISTER contains 8 bits to read

void spi_func(void) {
  gpio_init();

  SpiSettings spi_settings = {
    .baudrate = 6000000,  // Not sure what baudrate to set at
    .mode = SPI_MODE_0,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
  };

  spi_init(SPI_PORT_2, &spi_settings);

  uint8_t tx_bytes[] = { 0b01001001 };
  uint8_t tx_len = SIZEOF_ARRAY(tx_bytes);

  uint8_t rx_bytes[EXPECTED_RESPONSE_LENGTH] = { 0 };

  StatusCode status;
  status = spi_exchange(SPI_PORT_2, tx_bytes, tx_len, rx_bytes, EXPECTED_RESPONSE_LENGTH);

  if (status == STATUS_CODE_OK) {
    uint8_t txb1_cntrl =
        (tx_bytes[0] >> 3) &
        0x01;  // Not sure what TXB1CNTRL[3] means, assumed to be the 3rd bit of the response
    LOG_DEBUG("Successfully read, TXB1CNTRL[3] bit = %d/n", txb1_cntrl);
  }
}
