#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "power_selection.h"
#include "power_selection_events.h"
#include "soft_timer.h"

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_POWER_SELECTION,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = POWER_SELECTION_CAN_EVENT_RX,
  .tx_event = POWER_SELECTION_CAN_EVENT_TX,
  .fault_event = POWER_SELECTION_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

int main() {
  LOG_DEBUG("Welcome to Power Selection!\n");
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  can_init(&s_can_storage, &s_can_settings);
  aux_dcdc_monitor_init();

  // Hardware requirement to set PB8 to high before anything works
  GpioAddress enable_pin = { .port = GPIO_PORT_B, .pin = 8 };
  GpioSettings enable_settings = { .direction = GPIO_DIR_OUT,
                                   .state = GPIO_STATE_HIGH,
                                   .alt_function = GPIO_ALTFN_NONE,
                                   .resistor = GPIO_RES_NONE };
  gpio_init_pin(&enable_pin, &enable_settings);

  LOG_DEBUG("Working!\n");
  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }

  return 0;
}
