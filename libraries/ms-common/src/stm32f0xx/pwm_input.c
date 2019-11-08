#include "pwm_input.h"

#include <stdint.h>
#include <string.h>

#include "critical_section.h"
#include "gpio.h"
#include "log.h"
#include "pwm.h"
#include "stm32f0xx.h"

#define MAX_DC_PERCENT (1000)

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  TIM_TypeDef *base;
  uint16_t channel;
} PwmTimerData;

static PwmTimerData s_port[] = {
  [PWM_TIMER_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                    .periph = RCC_APB2Periph_TIM1,
                    .base = TIM1 },
  [PWM_TIMER_3] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_TIM3,
                    .base = TIM3 },
  [PWM_TIMER_14] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                     .periph = RCC_APB1Periph_TIM14,
                     .base = TIM14 },
  [PWM_TIMER_15] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                     .periph = RCC_APB2Periph_TIM15,
                     .base = TIM15 },
  [PWM_TIMER_16] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                     .periph = RCC_APB2Periph_TIM16,
                     .base = TIM16 },
  [PWM_TIMER_17] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                     .periph = RCC_APB2Periph_TIM17,
                     .base = TIM17 },
};

StatusCode pwm_input_init(PwmTimer timer, PwmChannel channel) {
  s_port[timer].rcc_cmd(s_port[timer].periph, ENABLE);

  uint16_t trigger_source = 0;

  // Sets the trigger source depending on whether we're using channel 1 or 2
  if (channel == PWM_CHANNEL_1) {
    s_port[timer].channel = TIM_Channel_1;
    trigger_source = TIM_TS_TI1FP1;
  } else {
    s_port[timer].channel = TIM_Channel_2;
    trigger_source = TIM_TS_TI2FP2;
  }

  TIM_TypeDef *tim_location = s_port[timer].base;

  // Gets our current frequency
  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  // Struct to configure PWM frequency (this will determine what unit the user
  // gets)
  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000000) - 1,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = 0xFFFFFFFF,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  TIM_TimeBaseInit(tim_location, &tim_init);

  // Struct to configure the timer for PWM input mode
  TIM_ICInitTypeDef tim_icinit = {
    .TIM_Channel = s_port[timer].channel,
    .TIM_ICPolarity = TIM_ICPolarity_Rising,
    .TIM_ICSelection = TIM_ICSelection_DirectTI,
    .TIM_ICPrescaler = TIM_ICPSC_DIV1,
    .TIM_ICFilter = 0x0,
  };

  TIM_PWMIConfig(tim_location, &tim_icinit);

  // Puts the timer into PWM input mode
  TIM_SelectInputTrigger(tim_location, trigger_source);
  TIM_SelectSlaveMode(tim_location, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(tim_location, TIM_MasterSlaveMode_Enable);
  TIM_SelectOutputTrigger(tim_location, TIM_TRGOSource_Reset);
  TIM_Cmd(tim_location, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode pwm_input_get_reading(PwmTimer timer, PwmInputReading *reading) {
  if (reading == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "|reading cannot be null|\n");
  }

  TIM_TypeDef *tim_location = s_port[timer].base;

  // Returns reading if the Capture Compare flag is set, otherwise
  // returns a PWM of 0
  if (TIM_GetFlagStatus(tim_location, TIM_FLAG_CC1) == SET) {
    TIM_ClearFlag(tim_location, TIM_FLAG_CC1);
  } else {
    reading->dc_percent = 0;
    reading->period_us = 0;
    return STATUS_CODE_OK;
  }

  uint32_t IC2Value_1 = 0;
  uint32_t IC2Value_2 = 0;

  uint32_t period_us = 0;
  uint32_t dc_percent = 0;

  // Depending on which channel we use, the values need to be flipped
  if (s_port[timer].channel == TIM_Channel_2) {
    IC2Value_1 = TIM_GetCapture2(tim_location);
    IC2Value_2 = TIM_GetCapture1(tim_location);
  } else {
    IC2Value_2 = TIM_GetCapture2(tim_location);
    IC2Value_1 = TIM_GetCapture1(tim_location);
  }

  // Perform the PWM calculation. IC2Value_1 is the period, and IC2Value_2 is
  // the time that the signal is high
  if (IC2Value_1 != 0) {
    dc_percent = (IC2Value_2 * MAX_DC_PERCENT) / IC2Value_1;
    period_us = IC2Value_1;
  } else {
    dc_percent = 0;
    period_us = 0;
  }

  if (dc_percent > MAX_DC_PERCENT) {
    dc_percent = MAX_DC_PERCENT;
  }

  reading->dc_percent = dc_percent;
  reading->period_us = period_us;

  return STATUS_CODE_OK;
}
