#pragma once

// Driver for the SPV1020 maximum power point tracker.
// Requires GPIO and SPI to be initialized.

// This driver only issues commands over SPI and does not select between MPPTs. To use this driver
// with multiple SPV1020s, make a wrapper.

// Note that this driver does not attempt to convert data from the SPV1020's internal ADC values.
// The reference voltage is 1.25V.

#include <stdint.h>
#include <stdbool.h>
#include "status.h"

// Issue a SHUT command to the SPV1020. This shuts down the SPV1020.
StatusCode spv1020_shut(void);

// Issue a Turn ON command to the SPV1020. This turns on the SPV1020 after a previous SHUT command.
// It is only required after a SHUT command as the SPV1020s turn on automatically on power up.
StatusCode spv1020_turn_on(void);

// Read current through the SPV1020. Current is 10 bits, converted by the SPV1020's internal ADC.
StatusCode spv1020_read_current(uint16_t *current);

// Read the SPV1020's input voltage. Voltage is 10 bits, converted by the SPV1020's internal ADC.
StatusCode spv1020_read_voltage_in(uint16_t *vin);

// Read PWM from the SPV1020. This is 9 bits.
StatusCode spv1020_read_pwm(uint16_t *pwm);

// Read a status byte from the SPV1020. The status byte should then be passed to
// |spv1020_is_overcurrent|, |spv_1020_is_overvoltage|, and |spv1020_is_overtemperature| to check
// each flag in the status byte.
StatusCode spv1020_read_status(uint8_t *status);

// Return whether an overcurrent (OVC) bit is set in the status byte. If an OVC bit is set,
// |ovc_branch| will be set to 1, 2, 3, or 4, indicating on which branch the overcurrent took place;
// 1 indicates the least significant OVC bit and 4 is the most significant. Otherwise, |ovc_branch|
// is set to 0.
bool spv1020_is_overcurrent(uint8_t status, uint8_t *ovc_branch);

// Return whether the overvoltage (OVV) bit is set in the status byte.
bool spv1020_is_overvoltage(uint8_t status);

// Return whether the overtemperature (OVT) bit is set in the status byte.
bool spv1020_is_overtemperature(uint8_t status);
