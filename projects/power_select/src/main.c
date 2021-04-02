// Power select FW implementation
#include "can.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "generic_can.h"
#include "interrupt.h"
#include "log.h"

#include "power_select.h"
#include "power_select_can.h"
#include "power_select_defs.h"
#include "power_select_events.h"

static CanStorage s_can_storage = { 0 };

static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_POWER_SELECT,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = POWER_SELECT_CAN_EVENT_RX,
  .tx_event = POWER_SELECT_CAN_EVENT_TX,
  .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

int main() {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  event_queue_init();

  can_init(&s_can_storage, &s_can_settings);

  power_select_can_init();
  power_select_init();
  power_select_start(POWER_SELECT_MEASUREMENT_INTERVAL_MS);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }
  return 0;
}
