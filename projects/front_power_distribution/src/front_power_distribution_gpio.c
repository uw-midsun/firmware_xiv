#include "front_power_distribution_gpio.h"
#include "front_power_distribution_events.h"

typedef enum {
  OUTPUT_GPIO_PIN_DRIVER_DISPLAY = 0,
  OUTPUT_GPIO_PIN_STEERING,
  OUTPUT_GPIO_PIN_CENTRE_CONSOLE,
  OUTPUT_GPIO_PIN_HEADLIGHTS,
  OUTPUT_GPIO_PIN_SIGNAL_LEFT,
  OUTPUT_GPIO_PIN_SIGNAL_RIGHT,
  NUM_OUTPUT_GPIO_PINS,
} OutputGpioPin;

static GpioAddress s_output_gpio_pins[] = {
  [OUTPUT_GPIO_PIN_DRIVER_DISPLAY] = { .port = GPIO_PORT_B, .pin = 2 }, //
  [OUTPUT_GPIO_PIN_STEERING] = { .port = GPIO_PORT_A, .pin = 10 }, //
  [OUTPUT_GPIO_PIN_CENTRE_CONSOLE] = { .port = GPIO_PORT_A, .pin = 8 }, //
  [OUTPUT_GPIO_PIN_HEADLIGHTS] = { .port = GPIO_PORT_B, .pin = 11 }, //
  [OUTPUT_GPIO_PIN_SIGNAL_LEFT] = { .port = GPIO_PORT_B, .pin = 13 }, //
  [OUTPUT_GPIO_PIN_SIGNAL_RIGHT] = { .port = GPIO_PORT_B, .pin = 12 }, //
};

void front_power_distribution_gpio_init(void) {
  // initialize all the output pins
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  for (uint8_t i = 0; i < NUM_OUTPUT_GPIO_PINS; i++) {
    gpio_init_pin(&s_output_gpio_pins[i], &settings);
  }
}

void front_power_distribution_gpio_process_event(Event *e) {
  GpioState new_state = e->data;
  
  switch (e->id) {
    case FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY:
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_DRIVER_DISPLAY], new_state);
      break;
    case FRONT_POWER_DISTRIBUTION_GPIO_EVENT_STEERING:
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_STEERING], new_state);
      break;
    case FRONT_POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE:
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_CENTRE_CONSOLE], new_state);
      break;
    case FRONT_POWER_DISTRIBUTION_GPIO_EVENT_HEADLIGHTS:
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_HEADLIGHTS], new_state);
      break;
    case FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT:
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_SIGNAL_LEFT], new_state);
      break;
    case FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT:
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_SIGNAL_RIGHT], new_state);
      break;
    case FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD:
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_SIGNAL_LEFT], new_state);
      gpio_set_state(&s_output_gpio_pins[OUTPUT_GPIO_PIN_SIGNAL_RIGHT], new_state);
      break;
  }
}
