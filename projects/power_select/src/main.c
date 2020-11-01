// Power select FW implementation

#include "can.h"
#include "interrupt.h"
#include "power_select.h"
#include "power_select_events.h"

#define POWER_SELECT_CAN_DEVICE_ID 0x1  // from old power_selection, not sure if up to date
static CanStorage s_can_storage;

static CanSettings s_can_settings = {
  .device_id = POWER_SELECT_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx_event = POWER_SELECT_CAN_EVENT_TX,
  .rx_event = POWER_SELECT_CAN_EVENT_RX,
  .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

int main() {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  can_init(&s_can_storage, &s_can_settings);

  power_select_init();
  power_select_start();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }
  return 0;
}