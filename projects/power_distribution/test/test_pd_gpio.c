#include "pd_gpio.h"

#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "output.h"
#include "pd_gpio_config.h"
#include "pin_defs.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Note: since output handles bts7200/bts7040 outputs for us, we test only with gpio outputs.

#define TEST_ADDRESS_0 \
  { GPIO_PORT_A, 0 }
#define TEST_ADDRESS_1 \
  { GPIO_PORT_A, 1 }

static const GpioAddress s_test_address_0 = TEST_ADDRESS_0;
static const GpioAddress s_test_address_1 = TEST_ADDRESS_1;

#define TEST_OUTPUT_0 0
#define TEST_OUTPUT_1 1
#define TEST_INVALID_OUTPUT NUM_OUTPUTS

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
#define TEST_ASSERT_GPIO_STATE(address, expected_state)        \
  ({                                                           \
    GpioState actual_state = NUM_GPIO_STATES;                  \
    TEST_ASSERT_OK(gpio_get_state(&(address), &actual_state)); \
    TEST_ASSERT_EQUAL((expected_state), actual_state);         \
  })

static OutputConfig s_test_output_config = {
  .specs =
      {
          [TEST_OUTPUT_0] =
              {
                  .type = OUTPUT_TYPE_GPIO,
                  .on_front = true,
                  .gpio_spec =
                      {
                          .address = TEST_ADDRESS_0,
                      },
              },
          [TEST_OUTPUT_1] =
              {
                  .type = OUTPUT_TYPE_GPIO,
                  .on_front = true,
                  .gpio_spec =
                      {
                          .address = TEST_ADDRESS_1,
                      },
              },
      },
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  PD_MUX_SEL1_PIN,
                  PD_MUX_SEL2_PIN,
                  PD_MUX_SEL3_PIN,
                  PD_MUX_SEL4_PIN,
              },
      },
  .mux_output_pin = PD_MUX_OUTPUT_PIN,
  .mux_enable_pin = PD_MUX_ENABLE_PIN,
  .i2c_addresses = (I2CAddress[]){},
  .num_i2c_addresses = 0,
  .i2c_port = PD_I2C_PORT,
};

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = PD_I2C_SCL_PIN,
    .sda = PD_I2C_SDA_PIN,
  };
  i2c_init(PD_I2C_PORT, &i2c_settings);

  output_init(&s_test_output_config, true);
}
void teardown_test(void) {}

// Test simply successfully receiving two events.
void test_power_distribution_gpio_basic(void) {
  // event 0: low -> high, event 1: high -> low
  PdGpioConfig test_config = {
    .events =
        (PdGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PdGpioOutputSpec[]){
                        {
                            .output = TEST_OUTPUT_0,
                            .state = PD_GPIO_STATE_ON,
                        },
                    },
                .num_outputs = 1,
            },
            {
                .event_id = TEST_EVENT_1,
                .outputs =
                    (PdGpioOutputSpec[]){
                        {
                            .output = TEST_OUTPUT_1,
                            .state = PD_GPIO_STATE_OFF,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 2,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(&test_config));

  // make sure it initialized correctly
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);

  // trigger each one at a time
  SEND_TEST_EVENT(TEST_EVENT_0, 0);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);

  SEND_TEST_EVENT(TEST_EVENT_1, 0xFF);  // not responsive to data
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);

  // make sure these states are idempotent
  SEND_TEST_EVENT(TEST_EVENT_0, 0xDE);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);
  SEND_TEST_EVENT(TEST_EVENT_1, 0xAD);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);
}

// Test setting multiple outputs for the same event.
void test_power_distribution_gpio_multiple_outputs(void) {
  // address 0: low -> high, address 1: high -> low
  PdGpioConfig test_config = {
    .events =
        (PdGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PdGpioOutputSpec[]){
                        {
                            .output = TEST_OUTPUT_0,
                            .state = PD_GPIO_STATE_ON,
                        },
                        {
                            .output = TEST_OUTPUT_1,
                            .state = PD_GPIO_STATE_OFF,
                        },
                    },
                .num_outputs = 2,
            },
        },
    .num_events = 1,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(&test_config));

  // make sure it initialized correctly
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);

  // make sure it sets correctly
  SEND_TEST_EVENT(TEST_EVENT_0, 0xBE);  // not responsive to data
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);

  // make sure it's idempotent with these states
  SEND_TEST_EVENT(TEST_EVENT_0, 0xEF);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);
}

// Test that the PD_STATE_SAME_AS_DATA and PD_STATE_OPPOSITE_TO_DATA states work.
void test_power_distribution_gpio_same_opposite(void) {
  PdGpioConfig test_config = {
    .events =
        (PdGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PdGpioOutputSpec[]){
                        {
                            .output = TEST_OUTPUT_0,
                            .state = PD_GPIO_STATE_SAME_AS_DATA,
                        },
                    },
                .num_outputs = 1,
            },
            {
                .event_id = TEST_EVENT_1,
                .outputs =
                    (PdGpioOutputSpec[]){
                        {
                            .output = TEST_OUTPUT_1,
                            .state = PD_GPIO_STATE_OPPOSITE_TO_DATA,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 2,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(&test_config));

  // make sure it initialized correctly
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);

  // make sure SAME_AS_DATA functions correctly (and the other doesn't change)
  SEND_TEST_EVENT(TEST_EVENT_0, 1);  // set to high
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_0, 0);                          // set to low
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_0, 0xFF);  // anything nonzero is mapped to 1 => set to high
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);  // doesn't change

  // make sure OPPOSITE_TO_DATA functions correctly (and the other doesn't change)
  SEND_TEST_EVENT(TEST_EVENT_1, 1);  // set to low
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_1, 0);                           // set to high
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_HIGH);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);  // doesn't change
  SEND_TEST_EVENT(TEST_EVENT_1, 0xFF);  // anything nonzero is mapped to 1 => set to low
  TEST_ASSERT_GPIO_STATE(s_test_address_1, GPIO_STATE_LOW);
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_HIGH);  // doesn't change
}

// Test that we ignore any event that isn't specified in the config.
void test_power_distribution_gpio_ignore_unspecified_event(void) {
  PdGpioConfig test_config = {
    .events =
        (PdGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PdGpioOutputSpec[]){
                        {
                            .output = TEST_OUTPUT_0,
                            .state = PD_GPIO_STATE_ON,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 1,
  };

  TEST_ASSERT_OK(power_distribution_gpio_init(&test_config));

  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_LOW);
  SEND_TEST_EVENT(NUM_TEST_EVENTS, 0);  // unspecified event
  TEST_ASSERT_GPIO_STATE(s_test_address_0, GPIO_STATE_LOW);
}

// Test that an invalid configuration is rejected.
void test_power_distribution_gpio_invalid_config(void) {
  PdGpioConfig test_config = {
    .events =
        (PdGpioEventSpec[]){
            {
                .event_id = TEST_EVENT_0,
                .outputs =
                    (PdGpioOutputSpec[]){
                        {
                            .output = TEST_OUTPUT_0,
                            .state = PD_GPIO_STATE_ON,
                        },
                    },
                .num_outputs = 1,
            },
        },
    .num_events = 1,
  };

  // invalid output state
  test_config.events[0].outputs[0].state = NUM_PD_GPIO_STATES;
  TEST_ASSERT_NOT_OK(power_distribution_gpio_init(&test_config));
  test_config.events[0].outputs[0].state = PD_GPIO_STATE_ON;

  // otherwise valid
  TEST_ASSERT_OK(power_distribution_gpio_init(&test_config));
}

// Test that FRONT_POWER_DISTRIBUTION_GPIO_CONFIG is valid.
void test_front_power_distribution_gpio_config_valid(void) {
  TEST_ASSERT_OK(power_distribution_gpio_init(&FRONT_POWER_DISTRIBUTION_GPIO_CONFIG));
}

// Test that REAR_POWER_DISTRIBUTION_GPIO_CONFIG is valid.
void test_rear_power_distribution_gpio_config_valid(void) {
  TEST_ASSERT_OK(power_distribution_gpio_init(&REAR_POWER_DISTRIBUTION_GPIO_CONFIG));
}
