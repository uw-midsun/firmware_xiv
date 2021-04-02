#include "power_select_can.h"

static CanStorage s_can_storage;

static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_POWER_SELECT,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx_event = POWER_SELECT_CAN_EVENT_TX,
  .rx_event = POWER_SELECT_CAN_EVENT_RX,
  .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static CanAckStatus prv_confirm_aux(uint16_t fault_bitset, uint8_t valid_bitset) {
  if (!(valid_bitset & 1 << POWER_SELECT_AUX_VALID) ||
      fault_bitset & 1 << POWER_SELECT_AUX_OVERCURRENT ||
      fault_bitset & 1 << POWER_SELECT_AUX_OVERVOLTAGE) {
    return CAN_ACK_STATUS_INVALID;
  } else {
    return CAN_ACK_STATUS_OK;
  }
}

static CanAckStatus prv_confirm_dcdc(uint16_t fault_bitset, uint8_t valid_bitset) {
  if (!(valid_bitset & 1 << POWER_SELECT_DCDC_VALID) ||
      fault_bitset & 1 << POWER_SELECT_DCDC_OVERCURRENT ||
      fault_bitset & 1 << POWER_SELECT_DCDC_OVERVOLTAGE) {
    return CAN_ACK_STATUS_INVALID;
  } else {
    return CAN_ACK_STATUS_OK;
  }
}
// Handles CAN message from centre console during main power on.
static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint16_t sequence = 0;
  CAN_UNPACK_POWER_ON_MAIN_SEQUENCE(msg, &sequence);
  uint16_t fault_bitset = power_select_get_fault_bitset();
  uint8_t valid_bitset = power_select_get_valid_bitset();
  if (sequence == EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS) {
    *ack_reply = prv_confirm_aux(fault_bitset, valid_bitset);
  }
  if (sequence == EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC) {
    *ack_reply = prv_confirm_dcdc(fault_bitset, valid_bitset);
  }

  return STATUS_CODE_OK;
}

// Handles CAN message from centre console during aux power on.
static StatusCode prv_rx_aux_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint16_t sequence = 0;
  CAN_UNPACK_POWER_ON_AUX_SEQUENCE(msg, &sequence);
  uint16_t fault_bitset = power_select_get_fault_bitset();
  uint8_t valid_bitset = power_select_get_valid_bitset();
  if (sequence == EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS) {
    *ack_reply = prv_confirm_aux(fault_bitset, valid_bitset);
  }
  return STATUS_CODE_OK;
}

StatusCode power_select_can_init(void) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE, prv_rx_aux_callback, NULL);
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_rx_callback, NULL);
}
