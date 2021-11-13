#include "smoke_charger_comms.h"

#include "can.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "connection_sense.h"
#include "event_queue.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_PERIOD_MS 10000  // The time between each softtimer call in ms

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

static void prv_softtimer_charger_controller_call(SoftTimerId timer_id, void *context) {
  StatusCode charger_controller_active();
  StatusCode charger_controller_deactive();
  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_softtimer_charger_controller_call, NULL, NULL);
}

void smoke_charger_controll_perform(void) {
  gpio_init();
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);
  charger_controller_init();

  LOG_DEBUG("Initializing soft timer for charger controller smoke test\n");
  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_softtimer_charger_controller_call, NULL, NULL);
  while (true) {
    wait();
  }
}
