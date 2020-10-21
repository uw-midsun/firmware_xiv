#include "adc_read.h"
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

static CanStorage s_can_storage;

static uint8_t s_times_callback_called;
static uint8_t s_received_data[8];
static void *s_received_context;

static bool s_should_tx_result;
static StatusCode s_status_return;

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);

  TEST_ASSERT_OK(adc_read_init());
}
