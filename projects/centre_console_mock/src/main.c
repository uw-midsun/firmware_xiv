#include "can.h"
#include "can_msg.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_DEVICE_ID 0x1

#define DELAY_TIME_MS 5000

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
} CanEvent;

static CanStorage s_can_storage = { 0 };

static void prv_can_transmit_transition(SoftTimerId timer_id, void *context) {
  static EEDriveOutput transition = EE_DRIVE_OUTPUT_OFF;
  transition++;
  transition %= NUM_EE_DRIVE_OUTPUTS;

  CanMessage message = {
    .source_id = CAN_DEVICE_ID,
    .msg_id = SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT,
    .data_u8 = { transition },
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 1,
  };
  can_transmit(&message, NULL);

  switch (transition) {
    case EE_DRIVE_OUTPUT_OFF:
      LOG_DEBUG("Sent transition to OFF\n");
      break;
    case EE_DRIVE_OUTPUT_DRIVE:
      LOG_DEBUG("Sent transition to DRIVE\n");
      break;
    case EE_DRIVE_OUTPUT_REVERSE:
      LOG_DEBUG("Sent transition to REVERSE\n");
      break;
    default:
      break;
  }

  soft_timer_start_millis(DELAY_TIME_MS, prv_can_transmit_transition, NULL, NULL);
}

int main() {
  gpio_init();
  event_queue_init();
  interrupt_init();

  soft_timer_init();

  const CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CAN_EVENT_RX,
    .tx_event = CAN_EVENT_TX,
    .fault_event = CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);

  soft_timer_start_millis(DELAY_TIME_MS, prv_can_transmit_transition, NULL, NULL);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }
  return 0;
}
