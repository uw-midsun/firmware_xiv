#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "spi.h"

#define I2C_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }

// returns the data in conversion register, I2C
StatusCode read_pedal_adc(uint8_t *read_data) {
  I2CAddress i2c_addr = 0x48;
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_STANDARD,
    .sda = I2C_SDA,
    .scl = I2C_SCL,
  };
  uint8_t write_data[] = { 0b00100100, 0b10100110 };

  status_ok_or_return(i2c_init(I2C_PORT_2, &i2c_settings));
  status_ok_or_return(i2c_write_reg(I2C_PORT_2, i2c_addr, 0b01, write_data, 2));
  status_ok_or_return(i2c_read_reg(I2C_PORT_2, i2c_addr, 0b00, read_data, 2));
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

StatusCode spi_read_status() {
  SpiSettings spi_settings = {
    .baudrate = 6000000,
    .mode = SPI_MODE_0,
    .mosi = SPI_MOSI,
    .miso = SPI_MISO,
    .sclk = SPI_SCLK,
    .cs = SPI_CS,
  };

  status_ok_or_return(spi_init(SPI_PORT_2, &spi_settings));

  uint8_t data = 0b01001001;
  // 010 -set loop back
  // 0   -terminates request to abort all transmissions
  // 1   -enable one-shot mode
  // 0   -disable CLKOUT pin
  // 01  -F_CLKOUT = System Clock / 2
  status_ok_or_return(spi_tx(SPI_PORT_2, &data, 1));

  uint8_t read_status_instr = 0b10100000;
  uint8_t data_out;
  status_ok_or_return(spi_exchange(SPI_PORT_2, &read_status_instr, 1, &data_out, 1));
  // print TXB1CNTRL[3], 4-th bit
  LOG_DEBUG("TXB1CNTRL[3]: %i\n", (data_out >> 4) & 1);
  return STATUS_CODE_OK;
}
