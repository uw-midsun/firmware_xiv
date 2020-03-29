#include "can.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"

#define TEST_CAN_DEVICE_ID 0x1
#define BCA_CCS_ID 0x18FF50E5
#define CCS_BMS_ID 0x1806E5F4
#define MAX_ALLOWABLE_VC 0x00000000ffffffff

static CanStorage s_can_storage;
static GenericCanMcp2515 s_generic_can = { 0 };

static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_250KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .rx_event = CHARGER_CAN_EVENT_RX,
  .tx_event = CHARGER_CAN_EVENT_TX,
  .fault_event = CHARGER_CAN_EVENT_FAULT,
  .loopback = false,
};

static Mcp2515Settings mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .spi_baudrate = 6000000,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
  .int_pin = { .port = GPIO_PORT_A, .pin = 8 },
  .can_bitrate = MCP2515_BITRATE_250KBPS,
  .loopback = false,
};

void prv_rx_cb(const GenericCanMsg *msg, void *context) {
  TEST_ASSERT_EQUAL(msg->id, CCS_BMS_ID);
  TEST_ASSERT_EQUAL(msg->data, MAX_ALLOWABLE_VC);
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  can_init(&s_can_storage, &s_can_settings);
  generic_can_mcp2515_init(&s_generic_can, &mcp2515_settings);

  TEST_ASSERT_OK(charger_controller_init(&s_generic_can.base));
}

void teardown_test(void) {}

void test_charger_controller_activate(void) {
  TEST_ASSERT_OK(charger_controller_activate());
}

void test_charger_controller_deactivate(void) {
  TEST_ASSERT_EQUAL(charger_controller_deactivate(), STATUS_CODE_UNINITIALIZED);

  TEST_ASSERT_OK(charger_controller_activate());
  TEST_ASSERT_OK(charger_controller_deactivate());
}

// this test makes sure the charger is transmitting properly
void test_charger_tx(void) {
  generic_can_register_rx(&s_generic_can.base, prv_rx_cb, GENERIC_CAN_EMPTY_MASK, NULL, false, NULL);
  charger_controller_activate();
  // should transmit immediately
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  delay_ms(1000);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
}

void test_charger_rx(void) {
  static GenericCanMsg s_gen_can_msg = {
    .id = BCA_CCS_ID,
    .extended = false,
    .data = (uint64_t)(1 << 24),
    .dlc = 0,
  };

  // hardware fault
  charger_controller_activate();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  generic_can_tx(&s_generic_can.base, &s_gen_can_msg);

  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, e.id);
  TEST_ASSERT_EQUAL(EE_CHARGER_HARDWARE_FAULT, e.data);

  // temperature fault
  s_gen_can_msg.data = (uint64_t)(1 << 25);
  charger_controller_activate();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  generic_can_tx(&s_generic_can.base, &s_gen_can_msg);

  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, e.id);
  TEST_ASSERT_EQUAL(EE_CHARGER_TEMP_FAULT, e.data);

  // temperature fault
  s_gen_can_msg.data = (uint64_t)(1 << 25);
  charger_controller_activate();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  generic_can_tx(&s_generic_can.base, &s_gen_can_msg);

  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, e.id);
  TEST_ASSERT_EQUAL(EE_CHARGER_TEMP_FAULT, e.data);

  // input fault
  s_gen_can_msg.data = (uint64_t)(1 << 26);
  charger_controller_activate();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  generic_can_tx(&s_generic_can.base, &s_gen_can_msg);

  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, e.id);
  TEST_ASSERT_EQUAL(EE_CHARGER_INPUT_FAULT, e.data);

  // state fault
  s_gen_can_msg.data = (uint64_t)(1 << 27);
  charger_controller_activate();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  generic_can_tx(&s_generic_can.base, &s_gen_can_msg);

  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, e.id);
  TEST_ASSERT_EQUAL(EE_CHARGER_STATE_FAULT, e.data);

  // communication fault
  s_gen_can_msg.data = (uint64_t)(1 << 28);
  charger_controller_activate();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  generic_can_tx(&s_generic_can.base, &s_gen_can_msg);

  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, e.id);
  TEST_ASSERT_EQUAL(EE_CHARGER_COMMUNICATION_FAULT, e.data);
}