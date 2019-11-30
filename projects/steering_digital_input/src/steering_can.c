#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can.h"
#include "can_transmit.h"
#include "delay.h"
#include "digital_input_config.h"
#include "digital_input_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

 

StatusCode steering_can_init() {
 // Set up settings for CAN
  static CanStorage storage = { 0 };
  const CanSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_STEERING,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = STEERING_DIGITAL_INPUT_CAN_RX,
    .tx_event = STEERING_DIGITAL_INPUT_CAN_TX,
    .tx = {GPIO_PORT_A, 11},
    .rx = {GPIO_PORT_A, 12},
    .loopback=true
  };
  // Initialize CAN
  can_init(&storage, &settings);
  return STATUS_CODE_OK;
}

StatusCode steering_can_process_event(Event *e) {
  if (status_ok(event_process(e))) {
      can_process_event(e);
    }    
    return STATUS_CODE_EMPTY;
}





