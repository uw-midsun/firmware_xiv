#include "gpio_get.h"

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define VALID_PIN 0
#define VALID_PORT 0

#define INVALID_PIN (GPIO_PINS_PER_PORT)
#define INVALID_PORT (NUM_GPIO_PORTS)

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

// tests that gpio_get_init works
void test_gpio_init_valid() {
  TEST_ASSERT_OK(gpio_get_init);
}

void test_prv_callback_gpio_get() {
  // test for valid port and pin
  TEST_ASSERT_OK(CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                                         INVALID_PORT,                         //
                                         VALID_PIN, 0, 0, 0, 0, 0));           //

  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  // test for invalid port and valid pin
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                                            INVALID_PORT,                         //
                                            VALID_PIN, 0, 0, 0, 0, 0));           //
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  // test for invalid pin and valid port
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                                            VALID_PORT,                           //
                                            INVALID_PIN, 0, 0, 0, 0, 0));         //
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  // test for invalid pin and port
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                                            INVALID_PORT,                         //
                                            INVALID_PIN, 0, 0, 0, 0, 0));         //
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}
