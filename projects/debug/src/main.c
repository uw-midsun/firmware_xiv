#include <stdlib.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define HORN_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 1 }
#define RADIO_PPT_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 6 }
#define HIGH_BEAM_FORWARD_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 7 }
#define HIGH_BEAM_REAR_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 0 }
#define REGEN_BRAKE_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 4 }
#define CC_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 5 }
#define CC_INCREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 2 }
#define CC_DECREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 1 }


#define TEST_1 \
  { .port = GPIO_PORT_A, .pin = 3 }
#define TEST_2 \
  { .port = GPIO_PORT_A, .pin = 3 }
#define TEST_3 \
  { .port = GPIO_PORT_A, .pin = 3 }

typedef enum {
  STEERING_DIGITAL_INPUT_HORN = 0,
  STEERING_DIGITAL_INPUT_RADIO_PPT,
  STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD,
  STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR,
  STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED,
  STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED,
  S_TEST_1,
  S_TEST_2,
  S_TEST_3,
  NUM_STEERING_DIGITAL_INPUTS,
} SteeringInterfaceDigitalInput;

static GpioAddress lookup[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = HORN_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_RADIO_PPT] = RADIO_PPT_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] = HIGH_BEAM_FORWARD_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = HIGH_BEAM_REAR_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = REGEN_BRAKE_TOGGLE_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = CC_TOGGLE_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED] = CC_INCREASE_SPEED_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED] = CC_DECREASE_SPEED_GPIO_ADDR,
  [S_TEST_1] = TEST_1,
  [S_TEST_2] = TEST_2,
  [S_TEST_3] = TEST_3,
};

int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_SINGLE);

  GpioSettings sets = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  AdcChannel chan;

  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
    gpio_init_pin(&lookup[i], &sets);
    adc_get_channel(lookup[i], &chan);
    adc_set_channel(chan, true);
  }

  while (1) {
    printf("=======reading %d=======\n", rand() % 10000);
    for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
      adc_get_channel(lookup[i], &chan);
      uint16_t d;
      adc_read_converted(chan, &d);
      printf("Port: %d, Pin: %d, Data: %d\n", lookup[i].port, lookup[i].pin, d);
    }
    delay_ms(500);
  }
}