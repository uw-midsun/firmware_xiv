#include "front_power_distribution_gpio.h"
#include "front_power_distribution_events.h"

// make (most of?) this configurable

#define I2C_ADDR_0 0x74
#define I2C_ADDR_1 0x76

static FrontPowerDistributionGpioOutput s_events_to_gpio_outputs[] = {
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY] =
      FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_STEERING] = FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE] =
      FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRL] = FRONT_POWER_DISTRIBUTION_OUTPUT_DRL,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_PEDAL] = FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_HORN] = FRONT_POWER_DISTRIBUTION_OUTPUT_HORN,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT] = FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT,
  [FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT] = FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT,
};

static Pca9539rGpioState s_default_gpio_states[] = {
  [FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] = PCA9539R_GPIO_STATE_LOW,  //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING] = PCA9539R_GPIO_STATE_LOW,        //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] = PCA9539R_GPIO_STATE_LOW,  //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_DRL] = PCA9539R_GPIO_STATE_LOW,             //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL] = PCA9539R_GPIO_STATE_LOW,           //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_HORN] = PCA9539R_GPIO_STATE_LOW,            //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT] = PCA9539R_GPIO_STATE_LOW,     //
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT] = PCA9539R_GPIO_STATE_LOW,    //
};

static Pca9539rGpioAddress s_output_gpio_pins[] = {
  [FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] = { .i2c_address = I2C_ADDR_0,
                                                       .pin = PCA9539R_PIN_IO0_7 },
  [FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING] = { .i2c_address = I2C_ADDR_1,
                                                 .pin = PCA9539R_PIN_IO1_3 },
  [FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] = { .i2c_address = I2C_ADDR_1,
                                                       .pin = PCA9539R_PIN_IO1_5 },
  [FRONT_POWER_DISTRIBUTION_OUTPUT_DRL] = { .i2c_address = I2C_ADDR_1, .pin = PCA9539R_PIN_IO0_5 },
  [FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL] = { .i2c_address = I2C_ADDR_1,
                                              .pin = PCA9539R_PIN_IO1_1 },
  [FRONT_POWER_DISTRIBUTION_OUTPUT_HORN] = { .i2c_address = I2C_ADDR_0, .pin = PCA9539R_PIN_IO1_7 },
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT] = { .i2c_address = I2C_ADDR_1,
                                                    .pin = PCA9539R_PIN_IO0_4 },
  [FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT] = { .i2c_address = I2C_ADDR_1,
                                                     .pin = PCA9539R_PIN_IO0_2 },
};

void front_power_distribution_gpio_init(void) {
  // initialize all the output pins
  Pca9539rGpioSettings settings = {
    .direction = GPIO_DIR_OUT,
  };
  for (FrontPowerDistributionGpioOutput i = 0; i < NUM_FRONT_POWER_DISTRIBUTION_GPIO_OUTPUTS; i++) {
    settings.state = s_default_gpio_states[i];
    pca9539r_gpio_init_pin(&s_output_gpio_pins[i], &settings);
  }
}

StatusCode front_power_distribution_gpio_process_event(Event *e) {
  FrontPowerDistributionGpioEvent id = e->id;

  if (!(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY <= id &&
        id < NUM_FRONT_POWER_DISTRIBUTION_GPIO_EVENTS)) {
    // this event isn't for us
    return STATUS_CODE_OUT_OF_RANGE;
  }

  Pca9539rGpioState new_state = (e->data == 1) ? PCA9539R_GPIO_STATE_HIGH : PCA9539R_GPIO_STATE_LOW;

  if (id == FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD) {
    // special case: maps to both left and right signals
    pca9539r_gpio_set_state(&s_output_gpio_pins[FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT],
                            new_state);
    pca9539r_gpio_set_state(&s_output_gpio_pins[FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT],
                            new_state);
  } else {
    pca9539r_gpio_set_state(&s_output_gpio_pins[s_events_to_gpio_outputs[id]], new_state);
  }

  return STATUS_CODE_OK;
}

Pca9539rGpioAddress *front_power_distribution_gpio_test_provide_gpio_addresses(void) {
  return s_output_gpio_pins;
}

FrontPowerDistributionGpioOutput *front_power_distribution_gpio_test_provide_events_to_outputs(
    void) {
  return s_events_to_gpio_outputs;
}
