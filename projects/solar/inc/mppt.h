#pragma once

#include <stdbool.h>
#include "spi.h"
#include "status.h"

StatusCode mppt_shut(SpiPort port, uint8_t pin);

StatusCode mppt_turn_on(SpiPort port, uint8_t pin);

StatusCode mppt_read_current(SpiPort port, uint16_t *current, uint8_t pin);

StatusCode mppt_read_voltage_in(SpiPort port, uint16_t *vin, uint8_t pin);

StatusCode sspecific_pv1020_read_pwm(SpiPort port, uint16_t *pwm, uint8_t pin);

StatusCode mppt_read_status(SpiPort port, uint8_t *status, uint8_t pin);
