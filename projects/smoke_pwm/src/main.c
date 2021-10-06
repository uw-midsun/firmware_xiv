#include "pwm_input.h"

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "pwm.h"
#include "soft_timer.h"
#include "adc.h"

#define TEST_INPUT_PWM_TIMER PWM_TIMER_3
#define TEST_INPUT_PWM_ALTFN GPIO_ALTFN_1
#define TEST_INPUT_PWM_CHANNEL PWM_CHANNEL_2
#define TEST_INPUT_PWM_ADDR \
  { .port = GPIO_PORT_A, .pin = 6, }

#define TEST_ADC_PIN \
  { .port = GPIO_PORT_A, .pin = 6, }

#define CONTROL_PILOT_SEL_PIN \
  { GPIO_PORT_A, 2 }

void print_reading(uint16_t *adc) {
  LOG_DEBUG("READ ADC: %d\n", *adc);// read DC: %d | read period: %d\n", 
            // *adc, (int)reading->dc_percent, (int)reading->period_us);
}

static void prv_get_pwm(SoftTimerId id, void *context) {
  // PwmInputReading pwm = { 0 };
  uint16_t reading = 0;

  // pwm_input_get_reading(TEST_INPUT_PWM_TIMER, &pwm);
  GpioAddress adc_pin = TEST_ADC_PIN;

  adc_read_converted_pin(adc_pin, &reading);

  print_reading(&reading);
  soft_timer_start(500, prv_get_pwm, NULL, NULL);
}

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();


  // PwmTimer input_timer = TEST_INPUT_PWM_TIMER;
  // GpioAddress input = TEST_INPUT_PWM_ADDR;
  GpioAddress adc = TEST_ADC_PIN;
  GpioAddress cp_sel = CONTROL_PILOT_SEL_PIN;

  // GpioSettings input_settings = {
  //   .direction = GPIO_DIR_IN,
  //   .alt_function = TEST_INPUT_PWM_ALTFN,
  // };

  GpioSettings adc_settings = {
    GPIO_DIR_IN,        //
    GPIO_STATE_LOW,     //
    GPIO_RES_NONE,      //
    GPIO_ALTFN_ANALOG,  //
  };

  GpioSettings cp_settings = {
    GPIO_DIR_OUT,        //
    GPIO_STATE_LOW,      //
    GPIO_RES_NONE,       //
    GPIO_ALTFN_NONE,     //
  };

  gpio_init_pin(&cp_sel, &cp_settings);
  gpio_init_pin(&adc, &adc_settings);
  gpio_set_state(&cp_sel, GPIO_STATE_HIGH);

  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(adc, true);

  // gpio_init_pin(&input, &input_settings);
  // pwm_input_init(input_timer, TEST_INPUT_PWM_CHANNEL);
  soft_timer_start_millis(1000, prv_get_pwm, NULL, NULL);

  return 0;
}
