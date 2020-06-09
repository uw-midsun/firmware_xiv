#pragma once

#include "spi.h"
#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "ads1259_adc_defs.h"

//Time in ms needed after calibration for each data rate
typedef enum {
  ADS1259_CALIBRATION_TIME_10 = 1900,
  ADS1259_CALIBRATION_TIME_17 = 1140,
  ADS1259_CALIBRATION_TIME_50 = 380,
  ADS1259_CALIBRATION_TIME_60 = 318,
  ADS1259_CALIBRATION_TIME_400 = 49,
  ADS1259_CALIBRATION_TIME_1200 = 17,
  ADS1259_CALIBRATION_TIME_3600 = 7,
  ADS1259_CALIBRATION_TIME_3600 = 3,
}

typedef struct Ads1259Settings {
  SpiPort spi_port;
  uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;

  GpioAddress int_pin;
} Ads1259Settings;

typedef struct Ads1259Storage {
  uint8_t* data;
  SpiPort spi_port;
} Ads1259Storage;

// Initializes ads1259 - soft-timers and spi must be initialized
StatusCode ads1259_init(Ads1259Settings* settings, Ads1259Storage* storage);

// Gets reading via single conversion mode
StatusCode ads1259_get_data(Ads1259Storage* storage);


//Do we want sleep/reset funcs?