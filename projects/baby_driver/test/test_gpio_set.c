#include "gpio_set.h"

#include <string.h>

#include "dispatcher.h"
#include "babydriver_msg_defs.h"
#include "gpio.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;
static uint8_t s_times_callback_called;
static uint8_t s_received_data[8];
static void *s_received_context;

static StatusCode s_status_return;

static StatusCode prv_rx_gpio_set_callback(uint8_t data[8], void *context, bool *tx_result) {
    s_times_callback_called++;
    memcpy(s_received_data, data, 8);
    s_received_context = context;
    *tx_result = false; // avoid TXing result of the status message
    return s_status_return;
}

void setup_test(void) {
    initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
    TEST_ASSERT_OK(dispatcher_init());
    TEST_ASSERT_OK(gpio_set_init());
    s_times_callback_called = 0;
    memset(s_received_data, 0, sizeof(s_received_data));
    s_status_return = STATUS_CODE_OK;
}

void teardown_test(void) {}

// Test that we can succesfully set the gpio pin to specified value (0 to 1) with an OK status
void test_setting_gpio(void) {
    // register a callback for the status message to check the resulting status
    TEST_ASSERT_OK(
      dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_rx_gpio_set_callback, NULL));

    // initialize existing gpio pin to modify: port = 1, pin = 2, state = GPIO_STATE_LOW
    uint8_t data[7] = { 1, 2, 1, 0, 0, 0, 0 };
    GpioAddress gpio_address = {
        .port = data[0],
        .pin = data[1]
    };
    GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,     
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,    
    .alt_function = GPIO_ALTFN_NONE,
    };
    gpio_init_pin(&gpio_address, &gpio_settings);

    // send can message to set gpio pin state to high
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_SET, data[0], data[1], data[2], data[3], data[4],
                          data[5], data[6]);
    // proces BABYDRIVER_MESSAGE_GPIO_SET message
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
    // process BABYDRIVER_MESSAGE_STATUS message
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

    // check that the gpio pin state has now been correctly set to high
    GpioState state_new = GPIO_STATE_LOW;
    gpio_get_state(&gpio_address, &state_new);
    TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state_new);

    // the data received should be from the status message with the status in the second byte
    TEST_ASSERT_EQUAL(1, s_times_callback_called);
    TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
    TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);
}

// Test that the status message has an invalid status when trying to set invalid gpio ports and pins
void test_setting_invalid_gpio(void) {
    // register a callback for the status message to check the resulting status
    TEST_ASSERT_OK(
      dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_rx_gpio_set_callback, NULL));
    s_status_return = STATUS_CODE_INVALID_ARGS;

    // test setting invalid pin 
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_SET, 7, 0, 0, 0, 0, 0, 0);
    // proces BABYDRIVER_MESSAGE_GPIO_SET message
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
    // process BABYDRIVER_MESSAGE_STATUS message
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

    // the data received should be from the status message with the status in the second byte
    TEST_ASSERT_EQUAL(1, s_times_callback_called);
    TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
    TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);
    
    // test setting invalid port
    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_SET, 0, 16, 0, 0, 0, 0, 0);
    // proces BABYDRIVER_MESSAGE_GPIO_SET message
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
    // process BABYDRIVER_MESSAGE_STATUS message
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

    // the data received should be from the status message with the status in the second byte
    TEST_ASSERT_EQUAL(2, s_times_callback_called);
    TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
    TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);



}

