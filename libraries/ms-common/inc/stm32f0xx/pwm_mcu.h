#pragma once

// Not all timers on the stm32f0xx support PWM mode these are the ones that do.
// The number comes from the id of the hardware timer.
typedef enum {
  PWM_TIMER_1 = 0,
  // PWM_TIMER_2,  // Requisitioned to back the soft_timer module.
  PWM_TIMER_3,
  PWM_TIMER_14,
  PWM_TIMER_15,
  PWM_TIMER_16,
  PWM_TIMER_17,
  NUM_PWM_TIMERS,
} PwmTimer;

typedef enum {
  PWM_CHANNEL_1 = 0,
  PWM_CHANNEL_2,
  NUM_PWM_CHANNELS,
} PwmChannel;
