#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "spi.h"

#define CONFIG_ADDRESS 0x01
#define CONVERSION_ADDRESS 0x0
#define I2C_ADDRESS 0x48

#define I2C_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

StatusCode write_read_i2c(void) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C_SDA,
    .scl = I2C_SCL,
  };
  i2c_init(I2C_PORT_2, &i2c_settings);

  uint8_t read_bytes[2] = { 0 };

  uint8_t bytes_to_write[] = { 0b00100100, 0b10100110 };

  status_ok_or_return(i2c_write_reg(I2C_PORT_2, I2C_ADDRESS, CONFIG_ADDRESS, bytes_to_write,
                                    SIZEOF_ARRAY(bytes_to_write)));

  status_ok_or_return(i2c_read_reg(I2C_PORT_2, I2C_ADDRESS, CONVERSION_ADDRESS, read_bytes, 2));

  return STATUS_CODE_OK;
}

#define SPI_MOSI \
  { .port = GPIO_PORT_B, .pin = 15 }
#define SPI_MISO \
  { .port = GPIO_PORT_B, .pin = 14 }
#define SPI_SCLK \
  { .port = GPIO_PORT_B, .pin = 13 }
#define SPI_CS \
  { .port = GPIO_PORT_B, .pin = 12 }

StatusCode read_spi(void) {
  SpiPort port_to_use = SPI_PORT_2;

  SpiSettings spi_settings = {
    .baudrate = 6000000,
    .mode = SPI_MODE_0,
    .mosi = SPI_MOSI,
    .miso = SPI_MISO,
    .sclk = SPI_SCLK,
    .cs = SPI_CS,
  };
  spi_init(port_to_use, &spi_settings);

  // 010 sets loopback mode
  // 0 terminated request to abort all transmissions
  // 1 enables one-shot mode
  // 0 disables CLKOUT pin
  // 01 set CLKOUT prescaler to system clock/2
  uint8_t write_data = 0b01001001;

  status_ok_or_return(port_to_us, &write_data, sizeof(write_data));

  uint8_t read_status_data = 0b10100000;

  uint8_t response;

  status_ok_or_return(spi_exchange(port_to_use, &read_status_data, sizeof(read_status_data),
                                   &response, sizeof(response)));

  LOG_DEBUG("Response is: %d\n", response >> 4);

  return STATUS_CODE_OK;
}
