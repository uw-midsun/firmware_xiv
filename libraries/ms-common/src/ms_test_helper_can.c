#include "ms_test_helper_can.h"
#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"

StatusCode initialize_can_and_dependencies(CanStorage *storage, SystemCanDevice device,
                                           EventId tx_event, EventId rx_event,
                                           EventId fault_event) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  soft_timer_init();
  const CanSettings s_can_settings = { .device_id = device,
                                       .loopback = true,
                                       .bitrate = CAN_HW_BITRATE_500KBPS,
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .tx = { .port = GPIO_PORT_A, .pin = 12 },
                                       .rx_event = rx_event,
                                       .tx_event = tx_event,
                                       .fault_event = fault_event };

  return can_init(storage, &s_can_settings);
}
