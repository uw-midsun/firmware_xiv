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
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE 1000
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE 2000
#define STEERING_CC_INCREASE_SPEED_VOLTAGE 3000
#define STEERING_CC_DECREASE_SPEED_VOLTAGE 4000
#define STEERING_CC_BRAKE_PRESSED_VOLTAGE 5000
#define VOLTAGE_TOLERANCE_MV 100

// Function prototype
void prv_control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context);

// Needs to be edited for the actual stalk
AdcPeriodicReaderSettings reader_settings = { .address = { .port = GPIO_PORT_A, .pin = 3 },
                                              .callback = control_stalk_callback };

// Stores event id of the event that was just raised
static SteeringAnalogEvent prev = NUM_STEERING_EVENTS;

void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context) {
  if (data > STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE - VOLTAGE_TOLERANCE_MV &&
      data < STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE + VOLTAGE_TOLERANCE_MV &&
      prev != STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL, data);
    prev = STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL;
  } else if (data > STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE - VOLTAGE_TOLERANCE_MV &&
             data < STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE + VOLTAGE_TOLERANCE_MV &&
             prev != STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL, data);
    prev = STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL;
  } else if (data > STEERING_CC_INCREASE_SPEED_VOLTAGE - VOLTAGE_TOLERANCE_MV &&
             data < STEERING_CC_INCREASE_SPEED_VOLTAGE + VOLTAGE_TOLERANCE_MV &&
             prev != STEERING_CC_EVENT_INCREASE_SPEED) {
    event_raise((EventId)STEERING_CC_EVENT_INCREASE_SPEED, data);
    prev = STEERING_CC_EVENT_INCREASE_SPEED;
  } else if (data > STEERING_CC_DECREASE_SPEED_VOLTAGE - VOLTAGE_TOLERANCE_MV &&
             data < STEERING_CC_DECREASE_SPEED_VOLTAGE + VOLTAGE_TOLERANCE_MV &&
             prev != STEERING_CC_EVENT_DECREASE_SPEED) {
    event_raise((EventId)STEERING_CC_EVENT_DECREASE_SPEED, data);
    prev = STEERING_CC_EVENT_DECREASE_SPEED;
  }
}

StatusCode control_stalk_init() {
  status_ok_or_return(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_0, &reader_settings));
  status_ok_or_return(adc_periodic_reader_start(PERIODIC_READER_ID_0));
  return STATUS_CODE_OK;
}
