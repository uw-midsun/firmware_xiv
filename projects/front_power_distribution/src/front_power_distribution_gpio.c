#include "front_power_distribution_gpio.h"
#include "front_power_distribution_events.h"

static FrontPowerDistributionGpioOutput s_events_to_gpio_outputs[] = {
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY] =
      FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_STEERING] = FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE] =
      FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_HEADLIGHTS] = FRONT_POWER_DISTRIBUTION_OUTPUT_HEADLIGHTS,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT] = FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT] = FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT,
};

static GpioState s_default_gpio_states[] = {
  [FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] = GPIO_STATE_LOW,  //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING] = GPIO_STATE_LOW,        //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] = GPIO_STATE_LOW,  //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_HEADLIGHTS] = GPIO_STATE_LOW,      //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT] = GPIO_STATE_LOW,     //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT] = GPIO_STATE_LOW,    //
};

static GpioAddress s_output_gpio_pins[] = {
  [FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] = { .port = GPIO_PORT_B, .pin = 2 },  //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING] = { .port = GPIO_PORT_A, .pin = 10 },       //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] = { .port = GPIO_PORT_A, .pin = 8 },  //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_HEADLIGHTS] = { .port = GPIO_PORT_B, .pin = 11 },     //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT] = { .port = GPIO_PORT_B, .pin = 13 },    //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT] = { .port = GPIO_PORT_B, .pin = 12 },   //
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
  FrontPowerDistributionGpioEvent id = e->id;

  if (!(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY <= id &&
        id < NUM_FRONT_POWER_DISTRIBUTION_GPIO_EVENTS)) {
    // this event isn't for us
    return STATUS_CODE_OUT_OF_RANGE;
  }

  GpioState new_state = (e->data == 1) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;

  if (id == FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD) {
    // special case: maps to both left and right signals
    gpio_set_state(&s_output_gpio_pins[FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT], new_state);
    gpio_set_state(&s_output_gpio_pins[FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT], new_state);
  } else {
    gpio_set_state(&s_output_gpio_pins[s_events_to_gpio_outputs[id]], new_state);
  }

  return STATUS_CODE_OK;
}

GpioAddress *front_power_distribution_gpio_test_provide_gpio_addresses(void) {
  return s_output_gpio_pins;
}

FrontPowerDistributionGpioOutput *front_power_distribution_gpio_test_provide_events_to_outputs(
    void) {
  return s_events_to_gpio_outputs;
}
