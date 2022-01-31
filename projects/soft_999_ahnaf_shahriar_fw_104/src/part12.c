#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "log.h"


#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 9 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 8 }
#define CONFIG_REGISTER_ADDR 0x48


StatusCode read_pedal_adc(void){
  gpio_init();
  I2CSettings i2c1_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C1_SDA,
    .scl = I2C1_SCL,
  };
  i2c_init(I2C_PORT_1, &i2c1_settings);

    uint8_t i2c_data_write[2] = {0b00100100, 0b10100110};
    uint8_t i2c_data_read;
    uint16_t write_size = SIZEOF_ARRAY(i2c_data_write);
    status_ok_or_return(i2c_write_reg(I2C_PORT_1, CONFIG_REGISTER_ADDR, 0, i2c_data_write, write_size));
    status_ok_or_return(i2c_write_reg(I2C_PORT_1, CONFIG_REGISTER_ADDR, 1, i2c_data_read, 1));

    return STATUS_CODE_OK;
}

#define SPI_MOSI \ 
    {.port= GPIO_PORT_B, .pin=15}

#define SPI_MISO \ 
    {.port= GPIO_PORT_B, .pin=14}
#define SPI_SCLK \ 
    {.port= GPIO_PORT_B, .pin=13}
#define SPI_SS \ 
    {.port= GPIO_PORT_B, .pin=12}

StatusCode I_SPI(){
  SpiSettings spi_settings {
    .baudrate = 600000,
    .mode = SPI_MODE_0,
    .mosi = SPI_MOSI,
    .miso = SPI_MISO,
    .sclk = SPI_SCLK,
    .cs = SPI_SS
  };
  spi_init(SPI_PORT_1, &spi_settings);

  status_ok_or_return()
  return STATUS_CODE_OK;
}