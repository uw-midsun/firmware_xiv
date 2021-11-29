#include "mci_mock.h"

#include "can.h"
#include "can_msg.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_DEVICE_ID 0x1

#define STATE_CHECK_DELAY 1000

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
} CanEvent;

static CanStorage s_can_storage = { 0 };

static void prv_print_drive_state(SoftTimerId timer_id, void *context) {
  uint8_t state = drive_fsm_get_drive_state();

  switch (state) {
    case EE_DRIVE_OUTPUT_OFF:
      LOG_DEBUG("Current state: OFF\n");
      break;
    case EE_DRIVE_OUTPUT_DRIVE:
      LOG_DEBUG("Current state: DRIVE\n");
      break;
    case EE_DRIVE_OUTPUT_REVERSE:
      LOG_DEBUG("Current state: REVERSE\n");
      break;
    default:
      LOG_DEBUG("Current state: UNKNOWN");
      break;
  }
  soft_timer_start_millis(STATE_CHECK_DELAY, prv_print_drive_state, NULL, NULL);
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

  drive_fsm_init();

  soft_timer_start_millis(STATE_CHECK_DELAY, prv_print_drive_state, NULL, NULL);

  Event e = { 0 };

  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  return 0;
}
