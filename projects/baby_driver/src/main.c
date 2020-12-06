// This project will receive a CAN message
// on the stm32 and perform specific callback
// based on the message

// To run this project, simply run
// make babydriver
// and that should program to the stm32 / run on x86
// To send a CAN message from Python write
// can_util.send_message(<id>, <data>) to send can message

#include "adc.h"
#include "adc_read.h"
#include "can.h"
#include "can_msg_defs.h"
#include "dispatcher.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_get.h"
<<<<<<< HEAD
=======
#include "gpio_set.h"
>>>>>>> master
#include "interrupt.h"
#include "log.h"
#include "wait.h"

    typedef enum { CAN_EVENT_RX = 0, CAN_EVENT_TX, CAN_EVENT_FAULT, NUM_CAN_EVENTS, } CanEvent;

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BABYDRIVER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_EVENT_RX,
  .tx_event = CAN_EVENT_TX,
  .fault_event = CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

int main() {
  LOG_DEBUG("Welcome to BabyDriver!\n");
  gpio_init();
  event_queue_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);

  can_init(&s_can_storage, &s_can_settings);

  dispatcher_init();
<<<<<<< HEAD
=======
  adc_read_init();
  gpio_set_init();
>>>>>>> master
  gpio_get_init();

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
    wait();
  }

  return 0;
}
