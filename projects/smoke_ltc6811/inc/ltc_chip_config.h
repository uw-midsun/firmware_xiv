#pragma once

#define SMOKE_LTC_AFE_ADC_MODE LTC_AFE_ADC_MODE_7KHZ

#define SMOKE_LTC_AFE_SPI_PORT SPI_PORT_1
#define SMOKE_LTC_AFE_SPI_BAUDRATE 750000
#define SMOKE_LTC_AFE_SPI_MOSI \
  { .port = GPIO_PORT_A, .pin = 7 }
#define SMOKE_LTC_AFE_SPI_MISO \
  { .port = GPIO_PORT_A, .pin = 6 }
#define SMOKE_LTC_AFE_SPI_SCLK \
  { .port = GPIO_PORT_A, .pin = 5 }
#define SMOKE_LTC_AFE_SPI_CS \
  { .port = GPIO_PORT_A, .pin = 4 }
