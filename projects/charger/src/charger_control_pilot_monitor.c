#include "charger_control_pilot_monitor.h"
#include "charger_events.h"

#include "event_queue.h"
#include "gpio.h"
#include "pwm_input.h"
#include "status.h"

// page 17 of the SAE J1772 standard OCT2017
// DC <= 8%: no charging allowed
// 9.5% <= DC < 10%: max = 6A
// 10% <= DC <= 85%: max = DC% * 0.6
// 85% < DC <= 96%: max = (DC% - 64) * 2.5
// 96% < DC <= 96.5%: max = 80A
// 96.5 < DC: no charging allowed
uint16_t prv_dc_to_current(uint32_t dc) {
  // based on a dc representation of ddd.d%
  float ret = 0;
  if (95 <= dc && dc < 100) {
    ret = 6.0f;
  } else if (100 <= dc && dc <= 850) {
    ret = (float)dc * 0.6f;
  } else if (850 < dc && dc <= 960) {
    ret = (float)(dc - 640) * 2.5f;
  } else if (960 < dc && dc <= 965) {
    ret = 80.0f;
  } else {
    ret = 0.0f;
  }
  // return with same representation rrr.r A
  return (uint16_t)(ret * 10);
}

void control_pilot_monitor_process_event(Event *e) {
  if (e->id == CHARGER_PWM_EVENT_REQUEST_READING) {
    PwmInputReading reading = { 0 };
    pwm_input_get_reading(PWM_TIMER_3, &reading);
    event_raise(CHARGER_PWM_EVENT_VALUE_AVAILABLE, prv_dc_to_current(reading.dc_percent));
  }
}

StatusCode control_pilot_monitor_init() {
  PwmTimer input_timer = PWM_TIMER_3;
  GpioAddress pwm_input = { .port = GPIO_PORT_A, .pin = 6 };
  GpioSettings input_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_1 };
  gpio_init_pin(&pwm_input, &input_settings);
  pwm_input_init(input_timer, PWM_CHANNEL_1);
  return STATUS_CODE_OK;
}
