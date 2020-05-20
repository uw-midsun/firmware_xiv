#include "adc.h"
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"
#include "steering_control_stalk.h"
#include "steering_digital_input.h"
#include "wait.h"
#define STEERING_CAN_DEVICE_ID 0x1

static CanStorage s_can_storage;

typedef enum {
  STEERING_CAN_EVENT_RX = 13,
  STEERING_CAN_EVENT_TX,
  STEERING_CAN_FAULT,
} SteeringCanEvent;

int main() {
  adc_init(ADC_MODE_SINGLE);
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  steering_digital_input_init();
  adc_periodic_reader_init();
  control_stalk_init();

  // Will be changed for the actual one
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

  Event e = { 0 };

  while (true) {
    while (event_process(&e)) {
      can_process_event(&e);
      steering_can_process_event(&e);
    }
  }

  return 0;
}
