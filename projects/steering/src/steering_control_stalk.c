#include "steering_control_stalk.h"
#include <stdio.h>
#include "adc.h"
#include "adc_periodic_reader.h"
#include "can.h"
#include "can_ack.h"
#include "can_fifo.h"
#include "can_fsm.h"
#include "can_hw.h"
#include "can_msg.h"
#include "can_rx.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"
#include "steering_digital_input.h"
#include "steering_events.h"

// Will be edited later
#define STEERING_CONTROL_STALK_LEFT_VOLTAGE 1000
#define STEERING_CONTROL_STALK_RIGHT_VOLTAGE 2000
#define STEERING_CC_INCREASE_SPEED_VOLTAGE 2000
#define STEERING_CC_DECREASE_SPEED_VOLTAGE 2500
#define STEERING_CC_BRAKE_PRESSED_VOLTAGE 100

// Function prototype
void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context);

// Needs to be edited for the actual stalk
AdcPeriodicReaderSettings reader_settings = { .address = { .port = GPIO_PORT_A, .pin = 3 },
                                              .callback = control_stalk_callback };

void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context) {
  if (data == STEERING_CONTROL_STALK_LEFT_VOLTAGE) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_LEFT, data);
  } else if (data == STEERING_CONTROL_STALK_RIGHT_VOLTAGE) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_RIGHT, data);
  } else if (data == STEERING_CC_INCREASE_SPEED_VOLTAGE) {
    event_raise((EventId)STEERING_CC_EVENT_INCREASE_SPEED, data);
  } else if (data == STEERING_CC_DECREASE_SPEED_VOLTAGE) {
    event_raise((EventId)STEERING_CC_EVENT_DECREASE_SPEED, data);
  }
}

StatusCode control_stalk_init() {
  status_ok_or_return(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_0, &reader_settings));
  status_ok_or_return(adc_periodic_reader_start(PERIODIC_READER_ID_0));
  return STATUS_CODE_OK;
}
