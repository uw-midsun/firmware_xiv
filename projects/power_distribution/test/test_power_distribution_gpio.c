#include "log.h"
#include "power_distribution_events.h"
#include "power_distribution_gpio.h"
#include "test_helpers.h"
#include "unity.h"

// NOT YET GENERIC

#define TEST_I2C_PORT I2C_PORT_2

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

static void prv_test_turn_on(EventId id, Pca9539rGpioAddress *address) {
  Event e = { .id = id, .data = 1 };
  StatusCode code = power_distribution_gpio_process_event(&e);
  TEST_ASSERT(code == STATUS_CODE_OK);

  Pca9539rGpioState new_state = PCA9539R_GPIO_STATE_LOW;
  TEST_ASSERT_OK(pca9539r_gpio_get_state(address, &new_state));
  TEST_ASSERT(new_state == PCA9539R_GPIO_STATE_HIGH);
}

void setup_test(void) {
  gpio_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = TEST_CONFIG_PIN_I2C_SCL,
    .sda = TEST_CONFIG_PIN_I2C_SDA,
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);
  pca9539r_gpio_init(TEST_I2C_PORT, 0x74);  // change this upon genericization

  power_distribution_gpio_init();
}
void teardown_test(void) {}

void test_power_distribution_gpio_each_pin_on(void) {
  Pca9539rGpioAddress *gpio_addresses = power_distribution_gpio_test_provide_gpio_addresses();
  PowerDistributionGpioOutput *events_to_outputs =
      power_distribution_gpio_test_provide_events_to_outputs();

  // perform the test for every event except SIGNAL_HAZARD which is handled separately
  for (PowerDistributionGpioEvent id = POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY;
       id < POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD; id++) {
    prv_test_turn_on(id, &gpio_addresses[events_to_outputs[id]]);
  }
}

void test_power_distribution_gpio_signal_hazard_on(void) {
  Pca9539rGpioAddress *gpio_addresses = power_distribution_gpio_test_provide_gpio_addresses();
  PowerDistributionGpioOutput *events_to_outputs =
      power_distribution_gpio_test_provide_events_to_outputs();

  Event e = { .id = POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, .data = 1 };
  StatusCode code = power_distribution_gpio_process_event(&e);
  TEST_ASSERT(code == STATUS_CODE_OK);

  Pca9539rGpioState new_state;
  pca9539r_gpio_get_state(
      &gpio_addresses[events_to_outputs[POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT]], &new_state);
  TEST_ASSERT(new_state == PCA9539R_GPIO_STATE_HIGH);
  pca9539r_gpio_get_state(
      &gpio_addresses[events_to_outputs[POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT]], &new_state);
  TEST_ASSERT(new_state == PCA9539R_GPIO_STATE_HIGH);
}

void test_power_distribution_gpio_invalid_id(void) {
  Event e = { .id = 0xABCD, .data = 1 };
  StatusCode code = power_distribution_gpio_process_event(&e);
  TEST_ASSERT(code == STATUS_CODE_OUT_OF_RANGE);
}
