#pragma once

// wrapper over spv1020 driver
// Requires GPIO and SPI to be initialized. SPI must be initialized with SPI_MODE_3.

#include <stdbool.h>
#include "spi.h"
#include "status.h"

// needs to initialized first in order to use mppt
StatusCode mppt_init();

// Sets a specific SPV1020 with a demux
// Then issues a SHUT command to the specific SPV1020. This shuts down the SPV1020.
StatusCode mppt_shut(SpiPort port, uint8_t pin);

// Sets a specific SPV1020 with a demux
// Then issues a Turn ON command to the specific SPV1020. This turns on the SPV1020 after a previous
// SHUT command. It is only required after a SHUT command as the SPV1020s turn on automatically on
// power up.
StatusCode mppt_turn_on(SpiPort port, uint8_t pin);

// Sets a specific SPV1020 with a demux
// Read current through the SPV1020. Current is 10 bits, converted by the SPV1020's internal ADC.
// This is either the input current or the average phase current (input current / 4 / duty cycle).
StatusCode mppt_read_current(SpiPort port, uint16_t *current, uint8_t pin);

// Sets a specific SPV1020 with a demux
// Read the SPV1020's input voltage. Voltage is 10 bits, converted by the SPV1020's internal ADC.
StatusCode mppt_read_voltage_in(SpiPort port, uint16_t *vin, uint8_t pin);

// Sets a specific SPV1020 with a demux
// Read the PWM duty cycle from the SPV1020, converted from the original 9 bits to a percentage
// accurate to 0.2%, represented with 4 decimal places (e.g. 12.3% = 123, actually permille).
// This should be in the range [50, 900] = [5%, 90%].
StatusCode mppt_read_pwm(SpiPort port, uint16_t *pwm, uint8_t pin);

// Sets a specific SPV1020 with a demux
// Read a status byte from the SPV1020. The status byte should then be passed to
// |spv1020_is_overcurrent|, |spv_1020_is_overvoltage|, and |spv1020_is_overtemperature| to check
// each flag in the status byte.
StatusCode mppt_read_status(SpiPort port, uint8_t *status, uint8_t pin);
