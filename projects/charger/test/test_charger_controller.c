#include <string.h>

#include "battery_monitor.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"

#define TEST_CAN_DEVICE_ID 0x1
#define CHARGER_RX_CAN_ID 0x18FF50E5

static uint8_t s_msg_txed;
static GenericCanMsg s_tx_msg;

static Mcp2515RxCb s_mcp2515_cb;

static uint8_t s_charger_fault_counter;
static EEChargerFault s_prev_charger_fault = NUM_EE_CHARGER_FAULTS;

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,         //
  .bitrate = CAN_HW_BITRATE_250KBPS,       //
  .tx = { GPIO_PORT_A, 12 },               //
  .rx = { GPIO_PORT_A, 11 },               //
  .rx_event = CHARGER_CAN_EVENT_RX,        //
  .tx_event = CHARGER_CAN_EVENT_TX,        //
  .fault_event = CHARGER_CAN_EVENT_FAULT,  //
  .loopback = true                         //
};

StatusCode TEST_MOCK(generic_can_tx)(const GenericCan *can, const GenericCanMsg *msg) {
  s_tx_msg = *msg;
  s_msg_txed++;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mcp2515_register_cbs)(Mcp2515Storage *storage, Mcp2515RxCb rx_cb,
                                           Mcp2515BusErrorCb bus_err_cb, void *context) {
  s_mcp2515_cb = rx_cb;
  return STATUS_CODE_OK;
}

StatusCode prv_charger_fault_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  RxMsgData rx_data = { .raw = msg->data };
  s_charger_fault_counter++;
  uint8_t prev_fault = NUM_EE_CHARGER_FAULTS;
  CAN_UNPACK_CHARGER_FAULT(msg, &prev_fault);
  s_prev_charger_fault = prev_fault;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  s_msg_txed = 0;
  memset(&s_tx_msg, 0, sizeof(s_tx_msg));
  s_mcp2515_cb = NULL;
  s_charger_fault_counter = 0;

  interrupt_init();
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);
  TEST_ASSERT_OK(charger_controller_init());
}

void teardown_test(void) {}

void test_activate_deactivate() {
  uint16_t arbitrary_current = 999;
  ChargerVC vc = { .values.complete.current = arbitrary_current,
                   .values.complete.voltage = CHARGER_BATTERY_THRESHOLD };

  Event e = { 0 };
  TEST_ASSERT(s_msg_txed == 0);
  TEST_ASSERT_OK(charger_controller_activate(vc.values.complete.current));
  TEST_ASSERT(s_msg_txed == 1);
  TEST_ASSERT_OK(charger_controller_deactivate());
  TEST_ASSERT(s_msg_txed = 2);

  TxMsgData tx_data = { .raw = s_tx_msg.data };
  TEST_ASSERT(tx_data.fields.max_voltage_high = vc.values.bytes.voltage_high);
  TEST_ASSERT(tx_data.fields.max_voltage_low = vc.values.bytes.voltage_low);
  TEST_ASSERT(tx_data.fields.max_current_high = vc.values.bytes.current_high);
  TEST_ASSERT(tx_data.fields.max_current_low = vc.values.bytes.current_low);
}

void test_rx() {
  RxMsgData rx_data = { 0 };
  rx_data.fields.status_flags.flags.hardware_failure = 1;
  rx_data.fields.status_flags.flags.communication_timeout = 1;

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CHARGER_FAULT, prv_charger_fault_rx, NULL);

  TEST_ASSERT(s_msg_txed == 0);
  s_mcp2515_cb(CHARGER_RX_CAN_ID, true, rx_data.raw, sizeof(rx_data), NULL);
  TEST_ASSERT(s_msg_txed == 1);

  TEST_ASSERT(s_charger_fault_counter == 0);
  TEST_ASSERT(s_prev_charger_fault == NUM_EE_CHARGER_FAULTS);
  MS_TEST_HELPER_CAN_TX(CHARGER_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_TX(CHARGER_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(CHARGER_CAN_EVENT_RX);
  TEST_ASSERT(s_charger_fault_counter == 1);
  TEST_ASSERT(s_prev_charger_fault == EE_CHARGER_FAULT_HARDWARE_FAILURE);
  MS_TEST_HELPER_CAN_RX(CHARGER_CAN_EVENT_RX);
  TEST_ASSERT(s_charger_fault_counter == 2);
  TEST_ASSERT(s_prev_charger_fault == EE_CHARGER_FAULT_COMMUNICATION_TIMEOUT);
}
