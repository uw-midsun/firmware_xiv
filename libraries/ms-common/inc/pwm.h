#pragma once
// PWM Module

// Usage Notes:
// - Module must be initialized for the timer you want before use.
// - Use pwm_set_dc unless you have reason not to.
// - pwm_set_pulse affords resolution < 1% unlike pwm_set_dc.
// - For stm32f0xx GPIO pins must use the correct GPIO_ALTFN to utilize the PWM
// (See Datasheet).
// - On stm32f0xx all PWM channels for a timer are automatically connected.

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

// Initializes the PWM for a set timer with a specific frequency (Hz)
#define pwm_init_hz(timer, frequency) pwm_init((timer), 1000000 / (frequency))

// Initializes the PWM for a set timer with a period in microseconds.
StatusCode pwm_init(PwmTimer timer, uint16_t period_us);

// Gets the current period of a specified PWM timer in microseconds.
uint16_t pwm_get_period(PwmTimer timer);

// Sets the pulse width in us of the PWM timer. Use for high resolution control.
StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us);

// Sets the duty cycle, in units of 1%, of the PWM timer. This wraps
// pwm_set_pulse doing the necessary math to convert from 0-100% to the fraction
// of the period.
StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc);
