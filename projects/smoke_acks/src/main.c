// This is a utility that acknowledges all messages with a configurable id.
// It was originally intended to be used for startup sequence testing,
// so it also transmits PRECHARGE_COMPLETED when it receives
// SYSTEM_CAN_MESSAGE_BEGIN_PRECHARGE.

// To run from a laptop, use this command in a separate terminal:
// make run PROJECT=smoke_acks PLATFORM=x86 DEFINE=CAN_HW_DEV_USE_CAN0

// IMPORTANT USAGE NOTE: you may have to increase the CAN_ACK_TIMEOUT_MS
// in can_ack.h, since laptops tend to be slow. A value of 250 ms should be safe.

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "controller_board_pins.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

// Change this id to whatever device you want.
#define ACK_DEVICE_ID SYSTEM_CAN_DEVICE_BABYDRIVER

static CanStorage s_can_storage;

static StatusCode prv_handle_msg(const CanMessage *msg, void *context, CanAckStatus *ack) {
  // Only messages with ID <= 16 get acknowledged.
  if (msg->msg_id <= (1 << 4)) {
    LOG_DEBUG("acked %d\n", msg->msg_id);
  }
  // Mock precharge for ease of use in power sequencing. Comment this out if unnecessary.
  if (msg->msg_id == SYSTEM_CAN_MESSAGE_BEGIN_PRECHARGE) {
    CAN_TRANSMIT_PRECHARGE_COMPLETED();
  }
  return STATUS_CODE_OK;
}

int main(void) {
  gpio_init();
  interrupt_init();
  event_queue_init();

  CanSettings can_settings = {
    .device_id = ACK_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = 1,
    .tx_event = 2,
    .fault_event = 3,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);

  can_register_rx_default_handler(prv_handle_msg, NULL);

  LOG_DEBUG("testing transmit via horn\n");
  CAN_TRANSMIT_HORN(1);

  LOG_DEBUG("now acking all critical messages\n");

  while (true) {
    Event e = { 0 };
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
    wait();
  }

  return STATUS_CODE_INTERNAL_ERROR;
}
