#include "control_pilot.h"

#include <stdint.h>

#include "gpio.h"
#include "pwm_input.h"
#include "status.h"

#include "log.h"

// arbitrary PWM resources since PWM isn't used elsewhere in the project.
#define CONTROL_PILOT_PWM_TIMER PWM_TIMER_3
#define CONTROL_PILOT_PWM_CHANNEL PWM_CHANNEL_1

// page 17 of the SAE J1772 standard OCT2017
// Duty Cycle meaning depends on current:
// DC <= 8%: no charging allowed
// 9.5% <= DC < 10%: max = 6A
// 10% <= DC <= 85%: max = DC% * 0.6
// 85% < DC <= 96%: max = (DC% - 64) * 2.5
// 96% < DC <= 96.5%: max = 80A
// 96.5 < DC: no charging allowed
static uint16_t prv_duty_to_current(uint32_t duty) {
  // based on a duty cycle representation of ddd.d%
  // return with same representation rrr.r Amps
  uint16_t ret = 0;
  if (95 <= duty && duty < 100) {
    ret = 60;
  } else if (100 <= duty && duty <= 850) {
    ret = (duty * 3) / 5;  // multiply by 0.6
  } else if (850 < duty && duty <= 960) {
    ret = ((duty - 640) * 5) / 2;  // subtract 64% then multiply by 2.5
  } else if (960 < duty && duty <= 965) {
    ret = 800;
  } else {
    ret = 0;
  }
  return ret;
}

uint16_t control_pilot_get_current() {
  PwmInputReading reading = { 0 };
  pwm_input_get_reading(PWM_TIMER_3, &reading);
  // LOG_DEBUG("PERCENT: %ld, PERIOD: %ld\n", reading.dc_percent, reading.period_us);
  return prv_duty_to_current(reading.dc_percent);
}

StatusCode control_pilot_init() {
  PwmTimer input_timer = CONTROL_PILOT_PWM_TIMER;
  GpioAddress cp_address = CONTROL_PILOT_PWM_GPIO_ADDR;
  GpioSettings cp_settings = {
    .direction = GPIO_DIR_IN,     //
    .state = GPIO_STATE_LOW,      //
    .resistor = GPIO_RES_NONE,    //
    .alt_function = GPIO_ALTFN_1  //
  };
  LOG_DEBUG("INITIALIZING PWM\n");
  gpio_init_pin(&cp_address, &cp_settings);
  pwm_input_init(input_timer, CONTROL_PILOT_PWM_CHANNEL);
  return STATUS_CODE_OK;
}
