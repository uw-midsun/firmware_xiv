#pragma once
// Thermistor Reading interface for NXRT15XH103FA1B (10k Ohm NTC)
// Requires ADC, GPIO and Interrupts to be initialized.
//
// The thermistor and its fixed resistor forms a voltage divider.
// A temperature value ranging from 0~100 degrees is calculated from the voltage
// divider.
#include "adc.h"
#include "gpio.h"

// A 10kOhm resistor is used as the fixed value
#define THERMISTOR_FIXED_RESISTANCE_OHMS 10000

// ThermistorPosition indicates which resistor the node voltage is measured from
// ex: VDDA ---> R1 ---> R2 ---> GND
typedef enum {
  THERMISTOR_POSITION_R1 = 0,
  THERMISTOR_POSITION_R2,
  NUM_THERMISTOR_POSITIONS,
} ThermistorPosition;

typedef struct ThermistorStorage {
  ThermistorPosition position;
  AdcChannel adc_channel;
} ThermistorStorage;

// Initializes the GPIO pin and ADC Channel associated with the thermistor
StatusCode thermistor_init(ThermistorStorage *storage, GpioAddress thermistor_gpio,
                           ThermistorPosition position);

// Fetch the temperature reading in deciCelsius from the MCU's ADC
// Note: "dc" (deciCelsius) is a tenth of a celsius (0.1C)
StatusCode thermistor_get_temp(ThermistorStorage *storage, uint16_t *temperature_dc);

// Calculate the temperature in deciCelsius from ohms
StatusCode thermistor_calculate_temp(uint32_t thermistor_resistance_ohms, uint16_t *temperature_dc);

// Calculates the thermistor resistance given a certain temperature in dC
StatusCode thermistor_calculate_resistance(uint16_t temperature_dc,
                                           uint16_t *thermistor_resistor_ohms);
