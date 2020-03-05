#include "charger_control_pilot_monitor.h"
#include "charger_events.h"

#include "event_queue.h"
#include "gpio.h"
#include "pwm_input.h"
#include "status.h"

void handle_pwm_event(Event e) {
  if (e.id == PWM_READING_REQUEST) {
    PwmInputReading reading = { 0 };
    pwm_input_get_reading(PWM_TIMER_3, &reading);
    event_raise(PWM_READING_VALUE, reading.dc_percent);
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
