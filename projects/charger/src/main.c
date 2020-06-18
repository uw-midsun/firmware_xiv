#include "battery_monitor.h"
#include "begin_sequence.h"
#include "can.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "connection_sense.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "stop_sequence.h"

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CHARGER,  //
  .bitrate = CAN_HW_BITRATE_500KBPS,       //
  .tx = { GPIO_PORT_A, 12 },               //
  .rx = { GPIO_PORT_A, 11 },               //
  .rx_event = CHARGER_CAN_EVENT_RX,        //
  .tx_event = CHARGER_CAN_EVENT_TX,        //
  .fault_event = CHARGER_CAN_EVENT_FAULT,  //
  .loopback = false                        //
};

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);

  begin_sequence_init();
  battery_monitor_init();
  connection_sense_init();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      can_process_event(&e);
      begin_sequence_process_event(&e);
      stop_sequence_process_event(&e);
    }
  }

  return 0;
}
