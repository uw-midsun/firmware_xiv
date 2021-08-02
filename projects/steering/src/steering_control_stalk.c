#include "steering_control_stalk.h"
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "steering_can.h"
#include "steering_events.h"

AdcPeriodicReaderSettings reader_settings = { .address = { .port = GPIO_PORT_A, .pin = 3 },
                                              .callback = control_stalk_callback };
AdcPeriodicReaderSettings drl_on_settings = { .address = DRL_ON_GPIO_ADDR,
                                              .callback = drl_on_callback };
AdcPeriodicReaderSettings drl_off_settings = { .address = DRL_OFF_GPIO_ADDR,
                                               .callback = drl_off_callback };

// Stores event id of the previous event that was just raised
static SteeringAnalogEvent prev = 0;

void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context) {
  if (data > STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
      data < STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV &&
      prev != STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL, data);
    prev = STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL;
  } else if (data > STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
             data < STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV &&
             prev != STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL, data);
    prev = STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL;
  }
}

void drl_on_callback(uint16_t data, PeriodicReaderId id, void *context) {
  if (data > DRL_SIGNAL_VOLTAGE_MV - DRL_SIGNAL_VOLTAGE_TOLERANCE_MV &&
      data < DRL_SIGNAL_VOLTAGE_MV + DRL_SIGNAL_VOLTAGE_TOLERANCE_MV) {
    event_raise((EventId)STEERING_DRL_ON_EVENT, data);
  }
}

void drl_off_callback(uint16_t data, PeriodicReaderId id, void *context) {
  if (data > DRL_SIGNAL_VOLTAGE_MV - DRL_SIGNAL_VOLTAGE_TOLERANCE_MV &&
      data < DRL_SIGNAL_VOLTAGE_MV + DRL_SIGNAL_VOLTAGE_TOLERANCE_MV) {
    event_raise((EventId)STEERING_DRL_OFF_EVENT, data);
  }
}

StatusCode control_stalk_init() {
  status_ok_or_return(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_0, &reader_settings));
  status_ok_or_return(adc_periodic_reader_start(PERIODIC_READER_ID_0));
  status_ok_or_return(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_1, &drl_on_settings));
  status_ok_or_return(adc_periodic_reader_start(PERIODIC_READER_ID_1));
  status_ok_or_return(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_2, &drl_off_settings));
  status_ok_or_return(adc_periodic_reader_start(PERIODIC_READER_ID_2));
  return STATUS_CODE_OK;
}
