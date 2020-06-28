#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "ads1259_adc_defs.h"  // TODO(SOFT-173): Double check all includes are needed
#include "gpio.h"
#include "spi.h"

typedef struct Ads1259RxData {
  uint8_t MSB;
  uint8_t MID;
  uint8_t LSB;
  uint8_t CHK_SUM;
} Ads1259RxData;

typedef union Ads1259ConversionData {
  struct {
    uint8_t LSB;
    uint8_t MID;
    uint8_t MSB;
  };
  uint32_t raw;
} Ads1259ConversionData;

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
  Ads1259RxData rx_data;
  SpiPort spi_port;
  Ads1259ConversionData conv_data;
  double reading;
} Ads1259Storage;

// Initializes ads1259 - soft-timers and spi must be initialized
StatusCode ads1259_init(Ads1259Settings *settings, Ads1259Storage *storage);

// Gets reading via single conversion mode
StatusCode ads1259_get_conversion_data(Ads1259Storage *storage);
