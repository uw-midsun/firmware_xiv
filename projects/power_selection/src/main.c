#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TEST_CAN_DEVICE_ID 0x1

typedef enum {
  POWER_SELECTION_CAN_EVENT_RX = 10,
  POWER_SELECTION_CAN_EVENT_TX,
  POWER_SELECTION_CAN_EVENT_FAULT,
} PowerSelectionCanEvent;

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = POWER_SELECTION_CAN_EVENT_RX,
  .tx_event = POWER_SELECTION_CAN_EVENT_TX,
  .fault_event = POWER_SELECTION_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received a message!\n");

  if (msg->data_u16 == EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS) {
  }
  if (msg->data_u16 == EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC) {
  }

  return STATUS_CODE_OK;
}

static void aux_dcdc_monitor_init() {
  
}

int main() {
  LOG_DEBUG("Welcome to Power Selection!\n");
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  can_init(&s_can_storage, &s_can_settings);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_rx_callback, NULL);


  LOG_DEBUG("Working!\n");
  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }

  return 0;
}
