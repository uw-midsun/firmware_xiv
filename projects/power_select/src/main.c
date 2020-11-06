// Power select FW implementation

#include "can.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "generic_can.h"
#include "interrupt.h"
#include "log.h"
#include "power_select.h"
#include "power_select_defs.h"
#include "power_select_events.h"

static CanStorage s_can_storage;

static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_POWER_SELECTION,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx_event = POWER_SELECT_CAN_EVENT_TX,
  .rx_event = POWER_SELECT_CAN_EVENT_RX,
  .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

// Handles CAN message from centre console during startup.
// TODO(SOFT-341): move this out of main
static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint16_t sequence = 0;
  CAN_UNPACK_POWER_ON_MAIN_SEQUENCE(msg, &sequence);
  uint16_t fault_bitset = power_select_get_fault_bitset();
  if (sequence == EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS) {
    if ((fault_bitset & 1 << POWER_SELECT_AUX_OVERCURRENT) ||
        fault_bitset & 1 << POWER_SELECT_AUX_OVERVOLTAGE) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    } else {
      *ack_reply = CAN_ACK_STATUS_OK;
    }
  }
  if (sequence == EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC) {
    uint8_t valid_bitset = power_select_get_valid_bitset();
    if (valid_bitset & 1 << POWER_SELECT_DCDC_VALID ||
        fault_bitset & 1 << POWER_SELECT_DCDC_OVERCURRENT ||
        fault_bitset & 1 << POWER_SELECT_DCDC_OVERVOLTAGE) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    } else {
      *ack_reply = CAN_ACK_STATUS_OK;
    }
  }

  return STATUS_CODE_OK;
}

int main() {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  event_queue_init();

  can_init(&s_can_storage, &s_can_settings);

  LOG_DEBUG("init: %d\n", power_select_init());
  power_select_start();

  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_rx_callback, NULL));
  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }
  return 0;
}