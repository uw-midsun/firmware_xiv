#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "pwm.h"
#include "pwm_input.h"
#include "soft_timer.h"

#define TEST_INPUT_PWM_TIMER PWM_TIMER_3
#define TEST_INPUT_PWM_ALTFN GPIO_ALTFN_1
#define TEST_INPUT_PWM_CHANNEL PWM_CHANNEL_1
#define TEST_INPUT_PWM_ADDR \
  { .port = GPIO_PORT_A, .pin = 6, }


#define CONTROL_PILOT_SEL_PIN \
  { GPIO_PORT_A, 2 }

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  PwmInputReading reading = { 0 };

  PwmTimer input_timer = TEST_INPUT_PWM_TIMER;

  GpioAddress input = TEST_INPUT_PWM_ADDR;
  GpioAddress s_cp_select = CONTROL_PILOT_SEL_PIN;


  GpioSettings input_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = TEST_INPUT_PWM_ALTFN,
  };

  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_LOW,         //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };
  gpio_init_pin(&s_cp_select, &settings);
  // pwm_init_hz(TEST_INPUT_PWM_TIMER, 1000);
  gpio_init_pin(&input, &input_settings);
  pwm_input_init(input_timer, TEST_INPUT_PWM_CHANNEL);
  // gpio_set_state(&s_cp_select, GPIO_STATE_HIGH);
  LOG_DEBUG("HELLO WORLD\n");
  // delay_s(5);
  while (true) {
    pwm_input_get_reading(TEST_INPUT_PWM_TIMER, &reading);
    LOG_DEBUG("DC: %d | Period: %d\n", (int)reading.dc_percent, (int)reading.period_us);
    delay_ms(500);
  }

  return 0;
}
