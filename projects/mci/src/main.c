#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"

#include "drive_fsm.h"
#include "mci_events.h"
#include "motor_controller.h"

static CanStorage s_can_storage;
static MotorControllerStorage s_mci_storage;

void prv_setup_system_can() {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = MCI_CAN_EVENT_RX,
    .tx_event = MCI_CAN_EVENT_TX,
    .fault_event = MCI_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);
}

void prv_mci_storage_init(void *context) {
  PrechargeControlSettings precharge_settings = {
    .precharge_control = { .port = GPIO_PORT_A, .pin = 9 },
    .precharge_control2 = { .port = GPIO_PORT_B, .pin = 1 },
    .precharge_monitor = { .port = GPIO_PORT_B, .pin = 0 }
  };
}

int main(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  prv_setup_system_can();
  prv_mci_storage_init(&s_mci_storage);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  return 0;
}
