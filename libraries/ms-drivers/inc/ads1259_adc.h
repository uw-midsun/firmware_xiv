#pragma once

#include "spi.h"
#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "ads1259_adc_defs.h" // TODO(SOFT-173): Double check all includes are needed


typedef struct Ads1259RxData {
  uint8_t ADS_RX_MSB;
  uint8_t ADS_RX_MID;
  uint8_t ADS_RX_LSB;
  uint8_t ADS_RX_CHK_SUM;
} Ads1259RxData;

// Initialize Ads1259Settings with SPI settings
typedef struct Ads1259Settings {
  SpiPort spi_port;
  uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} Ads1259Settings;

// Static instance of Ads1259Storage must be declared
// ads1259_get_data() reads 24-bit conversion data into 'reading'
typedef struct Ads1259Storage {
  Ads1259RxData data;
  SpiPort spi_port;
  uint32_t reading;
} Ads1259Storage;


// Initializes ads1259 - soft-timers and spi must be initialized
StatusCode ads1259_init(Ads1259Settings* settings, Ads1259Storage* storage);

// Gets reading via single conversion mode
StatusCode ads1259_get_data(Ads1259Storage* storage);
