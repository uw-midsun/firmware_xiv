#include "pwm.h"

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

StatusCode pwm_init(PwmTimer timer, uint16_t period_ms) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

uint16_t pwm_get_period(PwmTimer timer) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_ms) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
