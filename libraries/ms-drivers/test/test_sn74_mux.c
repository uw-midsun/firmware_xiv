#include "log.h"
#include "sn74_mux.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  gpio_init();
}
void teardown_test(void) {}

// Test intializing with a valid mux address.
void test_sn74_mux_init_mux_valid(void) {
  Sn74MuxAddress valid_address = {
    .sel_pins =
        {
            { .port = GPIO_PORT_A, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },  //
            { .port = GPIO_PORT_A, .pin = 4 },  //
        },
    .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },
  };
  TEST_ASSERT_OK(sn74_mux_init_mux(&valid_address));
}

// Test initializing with an invalid mux address.
void test_sn74_mux_init_mux_invalid(void) {
  Sn74MuxAddress invalid_address = {
    .sel_pins =
        {
            { .port = NUM_GPIO_PORTS, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },     //
            { .port = GPIO_PORT_A, .pin = 4 },     //
        },
    .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },
  };
  TEST_ASSERT_NOT_OK(sn74_mux_init_mux(&invalid_address));
  invalid_address.sel_pins[0].port = GPIO_PORT_A;
  invalid_address.mux_output_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(sn74_mux_init_mux(&invalid_address));
}

// Test that mux_set is ok when setting pins. Doesn't test that the pins are set correctly.
void test_sn74_mux_set_valid(void) {
  Sn74MuxAddress address = {
    .sel_pins =
        {
            { .port = GPIO_PORT_A, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },  //
            { .port = GPIO_PORT_A, .pin = 4 },  //
        },
    .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },
  };
  TEST_ASSERT_OK(sn74_mux_init_mux(&address));
  TEST_ASSERT_OK(sn74_mux_set(&address, 0));
  TEST_ASSERT_OK(sn74_mux_set(&address, (1 << SN74_MUX_BIT_WIDTH) - 1));
}

// Test that mux_set fails upon passing a too-large selection.
void test_sn74_mux_set_invalid_selection(void) {
  Sn74MuxAddress address = {
    .sel_pins =
        {
            { .port = GPIO_PORT_A, .pin = 6 },  //
            { .port = GPIO_PORT_A, .pin = 5 },  //
            { .port = GPIO_PORT_A, .pin = 4 },  //
        },
    .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },
  };
  TEST_ASSERT_OK(sn74_mux_init_mux(&address));

  TEST_ASSERT_NOT_OK(sn74_mux_set(&address, 1 << SN74_MUX_BIT_WIDTH));
  TEST_ASSERT_NOT_OK(sn74_mux_set(&address, 0xBE));
}
