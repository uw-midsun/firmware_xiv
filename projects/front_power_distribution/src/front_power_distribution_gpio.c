#include "front_power_distribution_gpio.h"

typedef enum {
  OUTPUT_GPIO_PIN_DRIVER_DISPLAY,
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

typedef enum {
  OUTPUT_DRIVER_DISPLAY = 0,
  OUTPUT_STEERING,
  OUTPUT_CENTRE_CONSOLE,
  OUTPUT_HEADLIGHTS,
  OUTPUT_SIGNAL_LEFT,
  OUTPUT_SIGNAL_RIGHT,
  OUTPUT_SIGNAL_HAZARD,
  NUM_OUTPUTS,
} Output;

static OutputGpioPin *s_output_to_gpio_pin[] = {
  [OUTPUT_DRIVER_DISPLAY] = {OUTPUT_GPIO_PIN_DRIVER_DISPLAY}, //
  [OUTPUT_STEERING] = {OUTPUT_GPIO_PIN_STEERING}, //
  [OUTPUT_CENTRE_CONSOLE] = {OUTPUT_GPIO_PIN_CENTRE_CONSOLE}, //
  [OUTPUT_HEADLIGHTS] = {OUTPUT_GPIO_PIN_HEADLIGHTS}, //
  [OUTPUT_SIGNAL_LEFT] = {OUTPUT_GPIO_PIN_SIGNAL_LEFT}, //
  [OUTPUT_SIGNAL_RIGHT] = {OUTPUT_GPIO_PIN_SIGNAL_RIGHT}, //
  [OUTPUT_SIGNAL_HAZARD] = {OUTPUT_GPIO_PIN_SIGNAL_LEFT, OUTPUT_GPIO_PIN_SIGNAL_RIGHT}, //
};

void front_power_distribution_init(void) {
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
