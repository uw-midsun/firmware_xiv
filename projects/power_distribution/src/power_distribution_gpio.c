#include "power_distribution_gpio.h"
#include "power_distribution_events.h"

// NOT YET GENERIC
// make (most of?) this configurable

#define I2C_ADDR_0 0x74
#define I2C_ADDR_1 0x76

static PowerDistributionGpioOutput s_events_to_gpio_outputs[] = {
  [POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY] = POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY,
  [POWER_DISTRIBUTION_GPIO_EVENT_STEERING] = POWER_DISTRIBUTION_OUTPUT_STEERING,
  [POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE] = POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
  [POWER_DISTRIBUTION_GPIO_EVENT_DRL] = POWER_DISTRIBUTION_OUTPUT_DRL,
  [POWER_DISTRIBUTION_GPIO_EVENT_PEDAL] = POWER_DISTRIBUTION_OUTPUT_PEDAL,
  [POWER_DISTRIBUTION_GPIO_EVENT_HORN] = POWER_DISTRIBUTION_OUTPUT_HORN,
  [POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT] = POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT,
  [POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT] = POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT,
};

static Pca9539rGpioState s_default_gpio_states[] = {
  [POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] = PCA9539R_GPIO_STATE_LOW,  //
  [POWER_DISTRIBUTION_OUTPUT_STEERING] = PCA9539R_GPIO_STATE_LOW,        //
  [POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] = PCA9539R_GPIO_STATE_LOW,  //
  [POWER_DISTRIBUTION_OUTPUT_DRL] = PCA9539R_GPIO_STATE_LOW,             //
  [POWER_DISTRIBUTION_OUTPUT_PEDAL] = PCA9539R_GPIO_STATE_LOW,           //
  [POWER_DISTRIBUTION_OUTPUT_HORN] = PCA9539R_GPIO_STATE_LOW,            //
  [POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT] = PCA9539R_GPIO_STATE_LOW,     //
  [POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT] = PCA9539R_GPIO_STATE_LOW,    //
};

static Pca9539rGpioAddress s_output_gpio_pins[] = {
  [POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] = { .i2c_address = I2C_ADDR_0,
                                                 .pin = PCA9539R_PIN_IO0_7 },
  [POWER_DISTRIBUTION_OUTPUT_STEERING] = { .i2c_address = I2C_ADDR_1, .pin = PCA9539R_PIN_IO1_3 },
  [POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] = { .i2c_address = I2C_ADDR_1,
                                                 .pin = PCA9539R_PIN_IO1_5 },
  [POWER_DISTRIBUTION_OUTPUT_DRL] = { .i2c_address = I2C_ADDR_1, .pin = PCA9539R_PIN_IO0_5 },
  [POWER_DISTRIBUTION_OUTPUT_PEDAL] = { .i2c_address = I2C_ADDR_1, .pin = PCA9539R_PIN_IO1_1 },
  [POWER_DISTRIBUTION_OUTPUT_HORN] = { .i2c_address = I2C_ADDR_0, .pin = PCA9539R_PIN_IO1_7 },
  [POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT] = { .i2c_address = I2C_ADDR_1,
                                              .pin = PCA9539R_PIN_IO0_4 },
  [POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT] = { .i2c_address = I2C_ADDR_1,
                                               .pin = PCA9539R_PIN_IO0_2 },
};

void power_distribution_gpio_init(void) {
  // initialize all the output pins
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
  };
  for (PowerDistributionGpioOutput i = 0; i < NUM_POWER_DISTRIBUTION_GPIO_OUTPUTS; i++) {
    settings.state = s_default_gpio_states[i];
    pca9539r_gpio_init_pin(&s_output_gpio_pins[i], &settings);
  }
}

StatusCode power_distribution_gpio_process_event(Event *e) {
  PowerDistributionGpioEvent id = e->id;

  if (!(POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY <= id &&
        id < NUM_POWER_DISTRIBUTION_GPIO_EVENTS)) {
    // this event isn't for us
    return STATUS_CODE_OUT_OF_RANGE;
  }

  Pca9539rGpioState new_state = (e->data == 1) ? PCA9539R_GPIO_STATE_HIGH : PCA9539R_GPIO_STATE_LOW;

  if (id == POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD) {
    // special case: maps to both left and right signals
    pca9539r_gpio_set_state(&s_output_gpio_pins[POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT], new_state);
    pca9539r_gpio_set_state(&s_output_gpio_pins[POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT], new_state);
  } else {
    pca9539r_gpio_set_state(&s_output_gpio_pins[s_events_to_gpio_outputs[id]], new_state);
  }

  return STATUS_CODE_OK;
}

Pca9539rGpioAddress *power_distribution_gpio_test_provide_gpio_addresses(void) {
  return s_output_gpio_pins;
}

PowerDistributionGpioOutput *power_distribution_gpio_test_provide_events_to_outputs(void) {
  return s_events_to_gpio_outputs;
}
