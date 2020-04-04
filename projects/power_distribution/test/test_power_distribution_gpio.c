#include "log.h"
#include "power_distribution_gpio.h"
#include "power_distribution_gpio_config.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_I2C_ADDRESS 0x74

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

static const Pca9539rGpioAddress s_test_address_0 = { .i2c_address = TEST_I2C_ADDRESS,
                                                      .pin = PCA9539R_PIN_IO0_0 };
static const Pca9539rGpioAddress s_test_address_1 = { .i2c_address = TEST_I2C_ADDRESS,
                                                      .pin = PCA9539R_PIN_IO0_1 };
static const Pca9539rGpioAddress s_test_invalid_address = { .i2c_address = TEST_I2C_ADDRESS,
                                                            .pin = NUM_PCA9539R_GPIO_PINS };

typedef enum {
  TEST_EVENT_0 = 0,
  TEST_EVENT_1,
  NUM_TEST_EVENTS,
} TestEvents;

#define SEND_TEST_EVENT(event_id, event_data)                  \
  ({                                                           \
    Event e = { .id = (event_id), .data = (event_data) };      \
    TEST_ASSERT_OK(power_distribution_gpio_process_event(&e)); \
  })
#define TEST_ASSERT_GPIO_STATE(address, expected_state)                 \
  ({                                                                    \
    Pca9539rGpioState actual_state = NUM_PCA9539R_GPIO_STATES;          \
    TEST_ASSERT_OK(pca9539r_gpio_get_state(&(address), &actual_state)); \
    TEST_ASSERT_EQUAL((expected_state), actual_state);                  \
  })

void setup_test(void) {
  event_queue_init();
  gpio_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = TEST_CONFIG_PIN_I2C_SCL,
    .sda = TEST_CONFIG_PIN_I2C_SDA,
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);
  pca9539r_gpio_init(TEST_I2C_PORT, TEST_I2C_ADDRESS);
}
void teardown_test(void) {}

// Test simply successfully receiving two events.
void test_power_distribution_gpio_basic(void) {
  // event 0: low -> high, event 1: high -> low
  PowerDistributionGpioConfig test_config = {
    .events =
        (PowerDistributionGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PowerDistributionGpioOutputSpec[]){
                        {
                            .address = s_test_address_0,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                        },
                    },
                .num_outputs = 1,
            },
            {
                .event_id = TEST_EVENT_1,
                .outputs =
                    (PowerDistributionGpioOutputSpec[]){
                        {
                            .address = s_test_address_1,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 2,
    .all_addresses_and_default_states =
        (PowerDistributionGpioOutputSpec[]){
            {
                .address = s_test_address_0,
                .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
            },
            {
                .address = s_test_address_1,
                .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
            },
        },
    .num_addresses = 2,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(test_config));

  // make sure it initialized correctly
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_HIGH);

  // trigger each one at a time
  SEND_TEST_EVENT(TEST_EVENT_0, 0);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);

  SEND_TEST_EVENT(TEST_EVENT_1, 0xFF);  // not responsive to data
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_LOW);

  // make sure these states are idempotent
  SEND_TEST_EVENT(TEST_EVENT_0, 0xDE);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);
  SEND_TEST_EVENT(TEST_EVENT_1, 0xAD);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_LOW);
}

// Test setting multiple outputs for the same event.
void test_power_distribution_gpio_multiple_outputs(void) {
  // address 0: low -> high, address 1: high -> low
  PowerDistributionGpioConfig test_config = {
    .events =
        (PowerDistributionGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PowerDistributionGpioOutputSpec[]){
                        {
                            .address = s_test_address_0,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                        },
                        {
                            .address = s_test_address_1,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
                        },
                    },
                .num_outputs = 2,
            },
        },
    .num_events = 1,
    .all_addresses_and_default_states =
        (PowerDistributionGpioOutputSpec[]){
            {
                .address = s_test_address_0,
                .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
            },
            {
                .address = s_test_address_1,
                .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
            },
        },
    .num_addresses = 2,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(test_config));

  // make sure it initialized correctly
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_HIGH);

  // make sure it sets correctly
  SEND_TEST_EVENT(TEST_EVENT_0, 0xBE);  // not responsive to data
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_LOW);

  // make sure it's idempotent with these states
  SEND_TEST_EVENT(TEST_EVENT_0, 0xEF);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_LOW);
}

// Test that the POWER_DISTRIBUTION_STATE_SAME_AS_DATA and OPPOSITE_TO_DATA states work.
void test_power_distribution_gpio_same_opposite(void) {
  PowerDistributionGpioConfig test_config = {
    .events =
        (PowerDistributionGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PowerDistributionGpioOutputSpec[]){
                        {
                            .address = s_test_address_0,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,
                        },
                    },
                .num_outputs = 1,
            },
            {
                .event_id = TEST_EVENT_1,
                .outputs =
                    (PowerDistributionGpioOutputSpec[]){
                        {
                            .address = s_test_address_1,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_OPPOSITE_TO_DATA,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 2,
    .all_addresses_and_default_states =
        (PowerDistributionGpioOutputSpec[]){
            {
                .address = s_test_address_0,
                .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
            },
            {
                .address = s_test_address_1,
                .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
            },
        },
    .num_addresses = 2,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(test_config));

  // make sure it initialized correctly
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_HIGH);

  // make sure SAME_AS_DATA functions correctly (and the other doesn't change)
  SEND_TEST_EVENT(TEST_EVENT_0, 1);  // set to high
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_HIGH);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_0, 0);                                    // set to low
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_HIGH);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_0, 0xFF);  // anything nonzero is mapped to 1 => set to high
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_HIGH);  // doesn't change

  // make sure OPPOSITE_TO_DATA functions correctly (and the other doesn't change)
  SEND_TEST_EVENT(TEST_EVENT_1, 1);  // set to low
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_1, 0);                                    // set to high
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_1, 0xFF);  // anything nonzero is mapped to 1 => set to low
  TEST_ASSERT_GPIO_STATE(s_test_address_1, PCA9539R_GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_HIGH);  // doesn't change
}

// Test that we ignore any event that isn't specified in the config.
void test_power_distribution_gpio_ignore_unspecified_event(void) {
  PowerDistributionGpioConfig test_config = {
    .events =
        (PowerDistributionGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PowerDistributionGpioOutputSpec[]){
                        {
                            .address = s_test_address_0,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 1,
    .all_addresses_and_default_states =
        (PowerDistributionGpioOutputSpec[]){
            {
                .address = s_test_address_0,
                .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
            },
        },
    .num_addresses = 1,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(test_config));

  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_LOW);
  SEND_TEST_EVENT(NUM_TEST_EVENTS, 0);  // unspecified event
  TEST_ASSERT_GPIO_STATE(s_test_address_0, PCA9539R_GPIO_STATE_LOW);
}

// Test that an invalid configuration is rejected.
void test_power_distribution_gpio_invalid_config(void) {
  PowerDistributionGpioConfig test_config = {
    .events =
        (PowerDistributionGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PowerDistributionGpioOutputSpec[]){
                        {
                            .address = s_test_address_0,
                            .state = POWER_DISTRIBUTION_GPIO_STATE_HIGH,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 1,
    .all_addresses_and_default_states =
        (PowerDistributionGpioOutputSpec[]){
            {
                .address = s_test_address_0,
                .state = POWER_DISTRIBUTION_GPIO_STATE_LOW,
            },
        },
    .num_addresses = 1,
  };

  // invalid gpio address
  test_config.all_addresses_and_default_states[0].address = s_test_invalid_address;
  test_config.events[0].outputs[0].address = s_test_invalid_address;
  TEST_ASSERT_NOT_OK(power_distribution_gpio_init(test_config));
  test_config.all_addresses_and_default_states[0].address = s_test_address_0;
  test_config.events[0].outputs[0].address = s_test_address_0;

  // invalid default state
  test_config.all_addresses_and_default_states[0].state = NUM_POWER_DISTRIBUTION_GPIO_STATES;
  TEST_ASSERT_NOT_OK(power_distribution_gpio_init(test_config));
  test_config.all_addresses_and_default_states[0].state = POWER_DISTRIBUTION_GPIO_STATE_LOW;

  // invalid output state
  test_config.events[0].outputs[0].state = NUM_POWER_DISTRIBUTION_GPIO_STATES;
  TEST_ASSERT_NOT_OK(power_distribution_gpio_init(test_config));
  test_config.events[0].outputs[0].state = POWER_DISTRIBUTION_GPIO_STATE_HIGH;

  // otherwise valid
  TEST_ASSERT_OK(power_distribution_gpio_init(test_config));
}

// Test that FRONT_POWER_DISTRIBUTION_GPIO_CONFIG is valid.
void test_front_power_distribution_gpio_config_valid(void) {
  TEST_ASSERT_OK(power_distribution_gpio_init(FRONT_POWER_DISTRIBUTION_GPIO_CONFIG));
}

// Test that REAR_POWER_DISTRIBUTION_GPIO_CONFIG is valid.
void test_rear_power_distribution_gpio_config_valid(void) {
  TEST_ASSERT_OK(power_distribution_gpio_init(REAR_POWER_DISTRIBUTION_GPIO_CONFIG));
}
