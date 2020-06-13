#pragma once

// Driver for the SPV1020 maximum power point tracker.
// Requires GPIO and SPI to be initialized. SPI must be initialized with SPI_MODE_3.

// This driver only issues commands to the given SPI port and does not select between MPPTs. To use
// this driver with multiple SPV1020s, make a wrapper.

// Note that this driver does not attempt to convert data from the SPV1020's internal ADC values.
// The reference voltage is 1.25V as per the application note.

#include <stdbool.h>
#include "spi.h"
#include "status.h"

// Issue a SHUT command to the SPV1020. This shuts down the SPV1020.
StatusCode spv1020_shut(SpiPort port);

// Issue a Turn ON command to the SPV1020. This turns on the SPV1020 after a previous SHUT command.
// It is only required after a SHUT command as the SPV1020s turn on automatically on power up.
StatusCode spv1020_turn_on(SpiPort port);

// Read current through the SPV1020. Current is 10 bits, converted by the SPV1020's internal ADC.
// This is either the input current or the average phase current (input current / 4 / duty cycle).
StatusCode spv1020_read_current(SpiPort port, uint16_t *current);

// Read the SPV1020's input voltage. Voltage is 10 bits, converted by the SPV1020's internal ADC.
StatusCode spv1020_read_voltage_in(SpiPort port, uint16_t *vin);

// Read the PWM duty cycle from the SPV1020, converted from the original 9 bits to a percentage
// accurate to 0.2%, represented with 4 decimal places (e.g. 12.3% = 123, actually permille).
// This should be in the range [50, 900] = [5%, 90%].
StatusCode spv1020_read_pwm(SpiPort port, uint16_t *pwm);

// Read a status byte from the SPV1020. The status byte should then be passed to
// |spv1020_is_overcurrent|, |spv_1020_is_overvoltage|, and |spv1020_is_overtemperature| to check
// each flag in the status byte.
StatusCode spv1020_read_status(SpiPort port, uint8_t *status);

// Return whether an overcurrent (OVC) bit is set in the status byte. As well, |ovc_branches| will
// be set to a 4-bit bitmask indicating which, if any, branches have overcurrent.
bool spv1020_is_overcurrent(uint8_t status, uint8_t *ovc_branches);

// Return whether the overvoltage (OVV) bit is set in the status byte.
bool spv1020_is_overvoltage(uint8_t status);

// Return whether the overtemperature (OVT) bit is set in the status byte.
bool spv1020_is_overtemperature(uint8_t status);

// Return whether the CR bit is set in the status byte. We don't know what the CR bit is, but it
// might be useful to log it.
bool spv1020_is_cr_bit_set(uint8_t status);
