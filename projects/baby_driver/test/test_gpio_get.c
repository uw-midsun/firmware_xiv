#include "gpio_get.h"

#include <string.h>

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define VALID_PIN 0
#define VALID_PORT GPIO_PORT_A

#define INVALID_PIN (GPIO_PINS_PER_PORT)
#define INVALID_PORT (NUM_GPIO_PORTS)

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;
static uint8_t s_callback_counter;
static uint8_t s_callback_counter_state;
static uint8_t s_received_data[8];
static uint8_t s_received_data_state[8];

// This callback is used to receive and store the data from a babydriver status message
// for use in testing cases where the port/pin is invalid
static StatusCode prv_callback_gpio_get(uint8_t data[8], void *context, bool *tx_result) {
  s_callback_counter++;
  memcpy(s_received_data, data, 8);
  *tx_result = false;  // sending a status Babydriver message is not necessary

  return STATUS_CODE_OK;
}

// This callback is used to receive and store the data from a babydriver gpio_get data
// message for use in testing cases where the valid port and pin is set to high/low
static StatusCode prv_callback_gpio_get_state(uint8_t data[8], void *context, bool *tx_result) {
  s_callback_counter_state++;
  memcpy(s_received_data_state, data, 8);
  *tx_result = false;  // sending a status Babydriver message is not necessary

  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage,
                                  SYSTEM_CAN_DEVICE_BABYDRIVER,  //
                                  TEST_CAN_EVENT_TX,             //
                                  TEST_CAN_EVENT_RX,             //
                                  TEST_CAN_EVENT_FAULT);         //

  TEST_ASSERT_OK(dispatcher_init());
  TEST_ASSERT_OK(gpio_get_init());

  s_callback_counter = 0;
  s_callback_counter_state = 0;
  memset(s_received_data, 0, sizeof(s_received_data));
  memset(s_received_data_state, 0, sizeof(s_received_data_state));
  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS,  //
                                              prv_callback_gpio_get,      //
                                              NULL));                     //

  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_GPIO_GET_DATA,  //
                                              prv_callback_gpio_get_state,       //
                                              NULL));                            //
}

void teardown_test(void) {}

void test_invalid_input() {
  // test for invalid port and valid pin
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                          INVALID_PORT,                         //
                          VALID_PIN, 0, 0, 0, 0, 0);            //

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_callback_counter);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data[1]);

  // test for invalid pin and valid port
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                          VALID_PORT,                           //
                          INVALID_PIN, 0, 0, 0, 0, 0);          //

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_callback_counter);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data[1]);

  // test for invalid pin and port
  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                          INVALID_PORT,                         //
                          INVALID_PIN, 0, 0, 0, 0, 0);          //

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_callback_counter);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_data[1]);
}

// Testing that gpio_get works by checking if it outputs the correct state of a given pin and port
void test_gpio_get_valid_input() {
  // Valid pin and port with a high state
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,        //
    .state = GPIO_STATE_HIGH,         //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  GpioAddress address = {
    .port = VALID_PORT,  //
    .pin = VALID_PIN,    //
  };

  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                          VALID_PORT,                           //
                          VALID_PIN, 0, 0, 0, 0, 0);            //

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_callback_counter_state);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_GPIO_GET_DATA, s_received_data_state[0]);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, s_received_data_state[1]);

  TEST_ASSERT_EQUAL(1, s_callback_counter);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data[1]);

  // Valid pin and port with low state
  settings.state = GPIO_STATE_LOW;

  TEST_ASSERT_OK(gpio_init_pin(&address, &settings));

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                          VALID_PORT,                           //
                          VALID_PIN, 0, 0, 0, 0, 0);            //

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_callback_counter_state);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_GPIO_GET_DATA, s_received_data_state[0]);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, s_received_data_state[1]);

  TEST_ASSERT_EQUAL(2, s_callback_counter);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data[1]);
}
