#include "gpio_interrupts.h"

#include "babydriver_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static CanStorage s_can_storage;
static uint8_t s_times_register_callback_called;
static uint8_t s_times_handler_callback_called;
static StatusCode s_received_status;
static void *s_received_context;
static GpioPort s_test_port;
static uint8_t s_test_pin;
static InterruptEdge s_test_edge;
static GpioState s_returned_state;

// Required since gpio_it_trigger_interrupt cannot change gpio state
StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *state) {
  *state = s_returned_state;
  return STATUS_CODE_OK;
}

// Called when an attempt is made to register/unregister an interrupt
static StatusCode prv_rx_register_gpio_interrupt_callback(uint8_t data[8], void *context,
                                                          bool *tx_result) {
  s_times_register_callback_called++;
  s_received_status = data[1];
  s_received_context = context;
  *tx_result = false;
  return STATUS_CODE_OK;
}

// Called when the interrupt handler is called
static StatusCode prv_rx_gpio_interrupt_handler_callback(uint8_t data[8], void *context,
                                                         bool *tx_result) {
  s_times_handler_callback_called++;
  s_test_port = data[1];
  s_test_pin = data[2];
  s_test_edge = data[3];
  s_received_context = context;
  *tx_result = false;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  TEST_ASSERT_OK(dispatcher_init());
  TEST_ASSERT_OK(gpio_interrupts_init());
  s_times_register_callback_called = 0;
  s_times_handler_callback_called = 0;
  s_received_status = 0;
  s_test_port = 0;
  s_test_pin = 0;
  s_test_edge = 0;

  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS,
                                              prv_rx_register_gpio_interrupt_callback, NULL));
  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_GPIO_IT_INTERRUPT,
                                              prv_rx_gpio_interrupt_handler_callback, NULL));
}

void teardown_test(void) {}

// Test that gpio interrupts are registered, unregistered, and reregistered correctly
void test_register_gpio_interrupts(void) {
  GpioPort port = GPIO_PORT_A;
  uint8_t pin = 15;
  InterruptEdge edge = INTERRUPT_EDGE_RISING;
  // Since edge is rising gpio state will become high
  s_returned_state = GPIO_STATE_HIGH;
  GpioAddress address = { .port = port, .pin = pin };
  uint8_t data[8] = { BABYDRIVER_MESSAGE_GPIO_IT_REGISTER_COMMAND, port, pin, edge, 0, 0, 0, 0 };

  // Test whether a single register command works
  // Simultaneously test whether triggering interrupt with opposite edge fails
  // Send CAN message with register command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Trigger the gpio interrupt with incorrect edge and a message should not be received
  s_returned_state = GPIO_STATE_LOW;
  gpio_it_trigger_interrupt(&address);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Trigger the gpio interrupt with correct edge and a message should be received
  s_returned_state = GPIO_STATE_HIGH;
  gpio_it_trigger_interrupt(&address);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_handler_callback_called);
  // Compares data from command message with data transmitted after the gpio interrupt was triggered
  TEST_ASSERT_EQUAL(port, s_test_port);
  TEST_ASSERT_EQUAL(pin, s_test_pin);
  TEST_ASSERT_EQUAL(edge, s_test_edge);

  // Test whether reregistering the same interrupt works with different edge
  edge = INTERRUPT_EDGE_FALLING;
  // Since the edge is falling the state will fall to low
  s_returned_state = GPIO_STATE_LOW;
  data[3] = edge;

  // Send CAN message with register command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Trigger the gpio interrupt and a message should be received
  gpio_it_trigger_interrupt(&address);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_handler_callback_called);
  // Compares data from command message with data transmitted after the gpio interrupt was triggered
  TEST_ASSERT_EQUAL(port, s_test_port);
  TEST_ASSERT_EQUAL(pin, s_test_pin);
  TEST_ASSERT_EQUAL(edge, s_test_edge);

  // Test whether unregistering an interrupt that has been registered works
  data[0] = BABYDRIVER_MESSAGE_GPIO_IT_UNREGISTER_COMMAND;

  // Send CAN message with unregister command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Trigger the gpio interrupt but no message should be received since the interrupt has not
  // been registered
  gpio_it_trigger_interrupt(&address);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Test whether reregistering an interrupt that has been unregistered works
  // Also tests whether a falling interrupt edge triggers the interrupt when the
  // edge is INTERRUPT_EDGE_RISING_FALLING
  edge = INTERRUPT_EDGE_RISING_FALLING;
  // Since edge is rising the gpio state will become high
  s_returned_state = GPIO_STATE_HIGH;
  data[0] = BABYDRIVER_MESSAGE_GPIO_IT_REGISTER_COMMAND;
  data[3] = edge;

  // Send CAN message with register command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Trigger the gpio interrupt and a message should be received
  gpio_it_trigger_interrupt(&address);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_handler_callback_called);
  // Compares data from command message with data transmitted after the gpio interrupt was triggered
  TEST_ASSERT_EQUAL(port, s_test_port);
  TEST_ASSERT_EQUAL(pin, s_test_pin);
  TEST_ASSERT_EQUAL(INTERRUPT_EDGE_RISING, s_test_edge);

  // Test whether a rising edge triggers the interrupt when the edge is
  // INTERRUPT_EDGE_RISING_FALLING
  // Since the edge is falling the gpio state will become low
  s_returned_state = GPIO_STATE_LOW;

  // Send CAN message with register command message information
  CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(5, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Trigger the gpio interrupt and a message should be received
  gpio_it_trigger_interrupt(&address);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_handler_callback_called);
  // Compares data from command message with data transmitted after the gpio interrupt was triggered
  TEST_ASSERT_EQUAL(port, s_test_port);
  TEST_ASSERT_EQUAL(pin, s_test_pin);
  TEST_ASSERT_EQUAL(INTERRUPT_EDGE_FALLING, s_test_edge);
}

void test_invalid_args_gpio_interrupts(void) {
  GpioPort port = GPIO_PORT_E;
  uint8_t pin = 0;
  InterruptEdge invalid_edge = NUM_INTERRUPT_EDGES;
  s_returned_state = GPIO_STATE_HIGH;
  GpioAddress address = { .port = port, .pin = pin };
  uint8_t invalid_data[8] = {
    BABYDRIVER_MESSAGE_GPIO_IT_REGISTER_COMMAND, port, pin, invalid_edge, 0, 0, 0, 0
  };

  // Test whether registering an interrupt fails when given an invalid interrupt edge
  // Send CAN message with invalid information for the register command message
  CAN_TRANSMIT_BABYDRIVER(invalid_data[0], invalid_data[1], invalid_data[2], invalid_data[3],
                          invalid_data[4], invalid_data[5], invalid_data[6], invalid_data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_status);

  // Test whether unregistering an interrupt fails when given an invalid gpio pin
  // First an interrupt must be registered
  InterruptEdge valid_edge = INTERRUPT_EDGE_RISING;
  uint8_t valid_data[8] = {
    BABYDRIVER_MESSAGE_GPIO_IT_REGISTER_COMMAND, port, pin, valid_edge, 0, 0, 0, 0
  };

  // Send CAN message with valid information for the register command message
  CAN_TRANSMIT_BABYDRIVER(valid_data[0], valid_data[1], valid_data[2], valid_data[3], valid_data[4],
                          valid_data[5], valid_data[6], valid_data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_status);

  // Trigger the gpio interrupt and a message should be received
  gpio_it_trigger_interrupt(&address);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_handler_callback_called);
  // Compares data from command message with data transmitted after the gpio interrupt was triggered
  TEST_ASSERT_EQUAL(port, s_test_port);
  TEST_ASSERT_EQUAL(pin, s_test_pin);
  TEST_ASSERT_EQUAL(valid_edge, s_test_edge);

  // The only invalid piece of information becomes the pin
  uint8_t invalid_pin = GPIO_PINS_PER_PORT;
  invalid_data[0] = BABYDRIVER_MESSAGE_GPIO_IT_UNREGISTER_COMMAND;
  invalid_data[2] = invalid_pin;
  invalid_data[3] = valid_edge;

  // Send CAN message with invalid information for the unregister command message
  CAN_TRANSMIT_BABYDRIVER(invalid_data[0], invalid_data[1], invalid_data[2], invalid_data[3],
                          invalid_data[4], invalid_data[5], invalid_data[6], invalid_data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_status);

  // Test whether unregistering an interrupt that has not been registered fails
  pin = 5;
  valid_data[0] = BABYDRIVER_MESSAGE_GPIO_IT_UNREGISTER_COMMAND;
  valid_data[2] = pin;

  // Send CAN message with valid information for the register command message
  CAN_TRANSMIT_BABYDRIVER(valid_data[0], valid_data[1], valid_data[2], valid_data[3], valid_data[4],
                          valid_data[5], valid_data[6], valid_data[7]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_register_callback_called);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s_received_status);
}
