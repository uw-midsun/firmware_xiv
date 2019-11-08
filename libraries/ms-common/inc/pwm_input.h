#pragma once
// PWM Input Module. Requires that GPIO and timer pins are initialized

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

// This struct is used for getting the DC value and period at the same time
// The DC percent value is reported as a number 0-1000 which represents ddd.d%.
typedef struct {
  uint32_t dc_percent;
  uint32_t period_us;
} PwmInputReading;

// Initializes the timer for PWM input.

// In order to choose a timer, and a timer channel for a GPIO pin, please
// consult:
//
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/16253071/Resources?preview=/16253071/38486554/stm32f072_af.xlsx
//
// Note 1: Only channels one and two are supported at the moment.
// Note 2: The PWM output module uses all channels on a given timer,
//         this means that you cannot use PWM input with PWM output
//         on the same channel.
// Note 3: Not all timers support PWM output. TIM2 is reserved for
//         soft timers. Other timers may be flat out incompatible.
//         Please check the STM32 reference manual for compatability
//         information.
StatusCode pwm_input_init(PwmTimer timer, PwmChannel channel);

// Gets the PWM reading from a timer.
//
// Known issue: if the PWM DC reading transitions from a non zero number to
// zero, then the next |pwm_input_get_reading| will return the previous value
// instead of 0. The one after that will return the correct value DC and period
// value of 0. This is because a flag needs to be cleared first (internally, the
// stm32 register does not update for a PWM reading of 0)
//
// Therefore we need to do this check twice to see if it works. The end user
// should not need to worry about this too much assuming they are calling the
// read function relatively frequently
StatusCode pwm_input_get_reading(PwmTimer timer, PwmInputReading *reading);
