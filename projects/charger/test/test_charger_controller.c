#include "can.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can_hw.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"

#define TEST_CAN_DEVICE_ID 0x1
#define BCA_CCS_ID 0x18FF50E5
#define CCS_BMS_ID 0x1806E5F4
#define MAX_ALLOWABLE_VC 0xffff

static CanStorage s_can_storage;
static GenericCanMcp2515 s_can_mcp2515;
static GenericCan *s_can;
static int generic_counter = 0;
static int system_counter = 0;
static uint64_t data;

static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_250KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .rx_event = CHARGER_CAN_EVENT_RX,
  .tx_event = CHARGER_CAN_EVENT_TX,
  .fault_event = CHARGER_CAN_EVENT_FAULT,
  .loopback = true,
};

static Mcp2515Settings s_mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .spi_baudrate = 6000000,
  .mosi = { .port = GPIO_PORT_B, 15 },
  .miso = { .port = GPIO_PORT_B, 14 },
  .sclk = { .port = GPIO_PORT_B, 13 },
  .cs = { .port = GPIO_PORT_B, 12 },
  .int_pin = { .port = GPIO_PORT_A, 8 },

  .can_bitrate = MCP2515_BITRATE_250KBPS,
  .loopback = true,
};

static void prv_generic_rx_cb (uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  ++generic_counter;
  TEST_ASSERT_EQUAL(id, CCS_BMS_ID);
  TEST_ASSERT_EQUAL(data, MAX_ALLOWABLE_VC);
}

StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("WORKING\n");
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, msg->msg_id);
  data = msg->data;
  ++system_counter;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mcp2515_tx)(Mcp2515Storage *storage, uint32_t id, bool extended, uint64_t data,
                                 size_t dlc) {
  if (storage->rx_cb != NULL) {
    storage->rx_cb(id, extended, data, dlc, storage->context);
  }
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  // system can
  can_init(&s_can_storage, &s_can_settings);
  // charger can
  generic_can_mcp2515_init(&s_can_mcp2515, &s_mcp2515_settings);

  s_can = (GenericCan *)&s_can_mcp2515;
  TEST_ASSERT_OK(charger_controller_init(&s_can_mcp2515));
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
  TEST_ASSERT_OK(mcp2515_register_cbs(s_can_mcp2515.mcp2515, prv_generic_rx_cb, NULL, NULL));
  TEST_ASSERT_OK(charger_controller_activate());
  // should not transmit immediately
  delay_ms(1000);
  TEST_ASSERT_EQUAL(1, generic_counter);

  delay_ms(1000);
  TEST_ASSERT_EQUAL(2, generic_counter);
}

void test_charger_rx(void) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, prv_rx_callback, NULL);

  static GenericCanMsg s_gen_can_msg = {
    .id = BCA_CCS_ID,
    .extended = true,
    .data = (uint64_t)(1 << 24),
    .dlc = 8,
  };

  // hardware fault
  charger_controller_activate();
  generic_can_tx(s_can, &s_gen_can_msg);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_CHARGER_HARDWARE_FAULT, data);
  TEST_ASSERT_EQUAL(system_counter, 1);

  // temperature fault
  s_gen_can_msg.data = (uint64_t)(1 << 25);
  charger_controller_activate();
  generic_can_tx(s_can, &s_gen_can_msg);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_CHARGER_TEMP_FAULT, data);
  TEST_ASSERT_EQUAL(system_counter, 2);

  // temperature fault
  s_gen_can_msg.data = (uint64_t)(1 << 25);
  charger_controller_activate();
  generic_can_tx(s_can, &s_gen_can_msg);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_CHARGER_TEMP_FAULT, data);
  TEST_ASSERT_EQUAL(system_counter, 3);

  // input fault
  s_gen_can_msg.data = (uint64_t)(1 << 26);
  charger_controller_activate();
  generic_can_tx(s_can, &s_gen_can_msg);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_CHARGER_INPUT_FAULT, data);
  TEST_ASSERT_EQUAL(system_counter, 4);

  // state fault
  s_gen_can_msg.data = (uint64_t)(1 << 27);
  charger_controller_activate();
  generic_can_tx(s_can, &s_gen_can_msg);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_CHARGER_STATE_FAULT, data);
  TEST_ASSERT_EQUAL(system_counter, 5);

  // communication fault
  s_gen_can_msg.data = (uint64_t)(1 << 28);
  charger_controller_activate();
  generic_can_tx(s_can, &s_gen_can_msg);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_CHARGER_COMMUNICATION_FAULT, data);
  TEST_ASSERT_EQUAL(system_counter, 6);
}
