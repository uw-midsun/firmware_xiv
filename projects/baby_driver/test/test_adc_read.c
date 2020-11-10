#include <string.h>

#include "adc.h"
#include "adc_read.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "dispatcher.h"
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

static GpioAddress s_test_adc_pin_addr = { .port = 1, .pin = 1 };

static CanStorage s_can_storage = { 0 };

static uint8_t s_times_callback_called;
static uint8_t s_received_data[8];
static void *s_received_context;

static bool s_should_tx_result;
static StatusCode s_status_return;

static uint16_t s_mock_reading;
static uint8_t s_low;
static uint8_t s_high;

// On x86, adc_read_raw is hardcoded to return 2500. Low byte of 2500 is -60 and high byte is 9
#define LOW_BYTE -60
#define HIGH_BYTE 9

void TEST_MOCK(adc_read_raw_pin)(GpioAddress address, uint16_t *reading) {
  *reading = s_mock_reading;
  s_low = s_mock_reading & 0xff;
  s_high = (s_mock_reading >> 8) & 0xff;
}

void TEST_MOCK(adc_read_converted_pin)(GpioAddress address, uint16_t *reading) {
  *reading = s_mock_reading;
  s_low = s_mock_reading & 0xff;
  s_high = (s_mock_reading >> 8) & 0xff;
}

static StatusCode prv_rx_adc_read_callback(uint8_t data[8], void *context, bool *tx_result) {
  s_times_callback_called++;
  memcpy(s_received_data, data, 8);
  s_received_context = context;
  *tx_result = s_should_tx_result;
  return s_status_return;
}

// static StatusCode prv_test_adc_read_callback_handler(uint8_t data[8], void *context,
//                                                      bool *tx_result) {
//   s_received_context = context;
//   *tx_result = s_should_tx_result;

//   // // todo RAIYAN
//   // // Two callbacks: one for raw read, another for converted read
//   // if (s_times_callback_called < 2) {
//   //   CAN_UNPACK_BABYDRIVER(NULL, NULL, &s_received_data[0], &s_received_data[1],
//   &s_received_data[2],
//   //                         &s_received_data[3], NULL, NULL, NULL);
//   //   s_times_callback_called++;
//   //   return STATUS_CODE_OK;
//   // }
//   // return STATUS_CODE_RESOURCE_EXHAUSTED;
// }

void setup_test(void) {
  TEST_ASSERT_OK(initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER,
                                                 TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX,
                                                 TEST_CAN_EVENT_FAULT));
  TEST_ASSERT_OK(dispatcher_init());
  TEST_ASSERT_OK(adc_read_init());

  s_times_callback_called = 0;
  memset(s_received_data, 0, sizeof(s_received_data));
  s_received_context = NULL;

  // TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS,
  // prv_rx_adc_read_callback, NULL));
  // TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_ADC_READ_DATA,
  // prv_rx_adc_read_callback, NULL));
}

void teardown_test(void) {}

// Test that raw data command works
void test_adc_read_raw(void) {
  // TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_ADC_READ_COMMAND,
  //                                             prv_test_adc_read_callback_handler, NULL));
  s_should_tx_result = true;

  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_ADC_READ_DATA,
                                              prv_rx_adc_read_callback, NULL));
  TEST_ASSERT_OK(
      dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_rx_adc_read_callback, NULL));

  TEST_ASSERT_EQUAL(0, s_times_callback_called);

  uint8_t data[7] = { s_test_adc_pin_addr.port,
                      s_test_adc_pin_addr.pin,
                      1,  // raw read
                      0,
                      0,
                      0,
                      0 };

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_ADC_READ_COMMAND, data[0], data[1], data[2], data[3],
                          data[4], data[5], data[6]);

  // process adc_read message
  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(0, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data[1]);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  // s_should_tx_result = false; // disable tx for status message

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_ADC_READ_DATA, s_received_data[0]);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_ADC_READ_DATA, s_received_data[1]);

  TEST_ASSERT_EQUAL_INT8(LOW_BYTE, s_received_data[2]);
  TEST_ASSERT_EQUAL_INT8(LOW_BYTE, s_low);

  TEST_ASSERT_EQUAL_INT8(HIGH_BYTE, s_received_data[3]);
  TEST_ASSERT_EQUAL_INT8(HIGH_BYTE, s_high);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  // RAIYAN change array indices to understandable macros e.g LOW_BYTE = 1
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
}

// Test that converted data command works
void test_adc_read_converted(void) {
  // TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_ADC_READ_COMMAND,
  //                                             prv_test_adc_read_callback_handler, NULL));
  s_should_tx_result = true;

  TEST_ASSERT_OK(dispatcher_register_callback(BABYDRIVER_MESSAGE_ADC_READ_DATA,
                                              prv_rx_adc_read_callback, NULL));
  TEST_ASSERT_OK(
      dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_rx_adc_read_callback, NULL));

  uint8_t data[7] = { s_test_adc_pin_addr.port,
                      s_test_adc_pin_addr.pin,
                      0,  // converted read
                      0,
                      0,
                      0,
                      0 };

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_ADC_READ_COMMAND, data[0], data[1], data[2], data[3],
                          data[4], data[5], data[6]);

  // MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  // MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(0, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_received_data[1]);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_ADC_READ_DATA, s_received_data[1]);
  TEST_ASSERT_EQUAL_INT8(1, s_received_data[2]);  // confused on what to put for "expected" here.
  TEST_ASSERT_EQUAL_INT8(1, s_received_data[3]);

  MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Test that dispatch sends error if not pin data is incorrect
