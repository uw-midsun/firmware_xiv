#include "front_power_distribution_gpio.h"
#include "front_power_distribution_events.h"

typedef enum {
  OUTPUT_DRIVER_DISPLAY = 0,
  OUTPUT_STEERING,
  OUTPUT_CENTRE_CONSOLE,
  OUTPUT_HEADLIGHTS,
  OUTPUT_SIGNAL_LEFT,
  OUTPUT_SIGNAL_RIGHT,
  NUM_FRONT_POWER_DISTRIBUTION_GPIO_OUTPUTS,
} FrontPowerDistributionGpioOutput;

static FrontPowerDistributionGpioOutput s_events_to_gpio_outputs[] = {
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY] = OUTPUT_DRIVER_DISPLAY, //
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_STEERING] = OUTPUT_STEERING, //
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE] = OUTPUT_CENTRE_CONSOLE, //
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_HEADLIGHTS] = OUTPUT_HEADLIGHTS, //
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT] = OUTPUT_SIGNAL_LEFT, //
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT] = OUTPUT_SIGNAL_RIGHT, //
};

static GpioState s_default_gpio_states[] = {
  [OUTPUT_DRIVER_DISPLAY] = GPIO_STATE_LOW, //
  [OUTPUT_STEERING] = GPIO_STATE_LOW, //
  [OUTPUT_CENTRE_CONSOLE] = GPIO_STATE_LOW, //
  [OUTPUT_HEADLIGHTS] = GPIO_STATE_LOW, //
  [OUTPUT_SIGNAL_LEFT] = GPIO_STATE_LOW, //
  [OUTPUT_SIGNAL_RIGHT] = GPIO_STATE_LOW, //
};

static GpioAddress s_output_gpio_pins[] = {
  [OUTPUT_DRIVER_DISPLAY] = { .port = GPIO_PORT_B, .pin = 2 }, //
  [OUTPUT_STEERING] = { .port = GPIO_PORT_A, .pin = 10 }, //
  [OUTPUT_CENTRE_CONSOLE] = { .port = GPIO_PORT_A, .pin = 8 }, //
  [OUTPUT_HEADLIGHTS] = { .port = GPIO_PORT_B, .pin = 11 }, //
  [OUTPUT_SIGNAL_LEFT] = { .port = GPIO_PORT_B, .pin = 13 }, //
  [OUTPUT_SIGNAL_RIGHT] = { .port = GPIO_PORT_B, .pin = 12 }, //
};

void front_power_distribution_gpio_init(void) {
  // initialize all the output pins
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  for (FrontPowerDistributionGpioOutput i = 0; i < NUM_FRONT_POWER_DISTRIBUTION_GPIO_OUTPUTS; i++) {
    settings.state = s_default_gpio_states[i];
    gpio_init_pin(&s_output_gpio_pins[i], &settings);
  }
}

StatusCode front_power_distribution_gpio_process_event(Event *e) {
  GpioState new_state = e->data;
  
  if (e->id == FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD) {
    // special case: maps to both left and right signals
    gpio_set_state(&s_output_gpio_pins[OUTPUT_SIGNAL_LEFT], new_state);
    gpio_set_state(&s_output_gpio_pins[OUTPUT_SIGNAL_RIGHT], new_state);
    return STATUS_CODE_OK;
  } else if (FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY <= e->id && e->id < NUM_FRONT_POWER_DISTRIBUTION_GPIO_EVENTS) {
    gpio_set_state(&s_output_gpio_pins[s_events_to_gpio_outputs[e->id]], new_state);
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_OUT_OF_RANGE;
  }
}
