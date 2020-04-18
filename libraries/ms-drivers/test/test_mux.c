#include "log.h"
#include "mux.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  gpio_init();
}
void teardown_test(void) {}

// Test intializing with a valid mux address.
void test_mux_init_valid(void) {
  MuxAddress valid_address = {
    .bit_width = 4,
    .sel_pins =
        {
            { .port = GPIO_PORT_A, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },  //
            { .port = GPIO_PORT_A, .pin = 4 },  //
            { .port = GPIO_PORT_A, .pin = 3 },  //
        },
    .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },
    .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },
  };
  TEST_ASSERT_OK(mux_init(&valid_address));
}

// Test initializing with an invalid mux address.
void test_mux_init_invalid(void) {
  MuxAddress invalid_address = {
    .bit_width = 3,
    .sel_pins =
        {
            { .port = NUM_GPIO_PORTS, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },     //
            { .port = GPIO_PORT_A, .pin = 4 },     //
        },
    .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },
    .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },
  };
  TEST_ASSERT_NOT_OK(mux_init(&invalid_address));
  invalid_address.sel_pins[0].port = GPIO_PORT_A;
  invalid_address.mux_output_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(mux_init(&invalid_address));
}

// Test initializing with an invalid bit width.
void test_mux_init_bad_bit_width(void) {
  MuxAddress invalid_address = {
    .bit_width = MAX_MUX_BIT_WIDTH + 1,
  };
  TEST_ASSERT_NOT_OK(mux_init(&invalid_address));
}

// Test that mux_set is ok when setting pins. Doesn't test that the pins are set correctly.
void test_mux_set_valid(void) {
  uint8_t bit_width = 4;
  MuxAddress address = {
    .bit_width = bit_width,
    .sel_pins =
        {
            { .port = GPIO_PORT_A, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },  //
            { .port = GPIO_PORT_A, .pin = 4 },  //
            { .port = GPIO_PORT_A, .pin = 3 },  //
        },
    .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },
    .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },
  };
  TEST_ASSERT_OK(mux_init(&address));
  TEST_ASSERT_OK(mux_set(&address, 0));
  TEST_ASSERT_OK(mux_set(&address, (1 << bit_width) - 1));
}

// Test that mux_set fails upon passing a too-large selection.
void test_mux_set_invalid_selection(void) {
  uint8_t bit_width = 4;
  MuxAddress address = {
    .bit_width = bit_width,
    .sel_pins =
        {
            { .port = GPIO_PORT_A, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },  //
            { .port = GPIO_PORT_A, .pin = 4 },  //
            { .port = GPIO_PORT_A, .pin = 3 },  //
        },
    .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },
    .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },
  };
  TEST_ASSERT_OK(mux_init(&address));

  TEST_ASSERT_NOT_OK(mux_set(&address, 1 << bit_width));
  TEST_ASSERT_NOT_OK(mux_set(&address, 0xBE));
}
