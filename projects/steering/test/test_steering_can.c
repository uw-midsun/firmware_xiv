#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can.h"
#include "can_msg_defs.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_digital_input.h"
#include "test_helpers.h"
#include "wait.h"

#define STEERING_CAN_DEVICE_ID 0x1

typedef enum {
  STEERING_CAN_EVENT_RX = 10,
  STEERING_CAN_EVENT_TX,
  STEERING_CAN_FAULT,
} SteeringCanEvent;

static CanStorage s_can_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  steering_digital_input_init();

  CanSettings can_settings = {
    .device_id = STEERING_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = STEERING_CAN_EVENT_RX,
    .tx_event = STEERING_CAN_EVENT_TX,
    .fault_event = STEERING_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };

  can_init(&s_can_storage, &can_settings);
}

void test_steering_can() {}

void teardown_test(void) {}
