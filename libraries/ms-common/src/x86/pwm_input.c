#include "pwm_input.h"

#include <stdint.h>

StatusCode pwm_input_init(PwmTimer timer, PwmChannel channel) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode pwm_input_get_reading(PwmTimer timer, PwmInputReading *reading) {
  status_code(STATUS_CODE_UNIMPLEMENTED);
  return 0;
}
