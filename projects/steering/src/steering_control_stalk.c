#include "steering_control_stalk.h"

#include "adc.h"
#include "adc_periodic_reader.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "log.h"
#include "steering_can.h"
#include "steering_digital_input.h"
#include "steering_events.h"
AdcChannel chan;

AdcPeriodicReaderSettings reader_settings = { .address = { .port = GPIO_PORT_A, .pin = 3 },
                                              .callback = control_stalk_callback };

// Stores event id of the previous event that was just raised
static SteeringAnalogEvent prev = STEERING_CONTROL_STALK_EVENT_NO_SIGNAL;

void control_stalk_callback(uint16_t data, PeriodicReaderId id, void *context) {
  if (data > STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
      data < STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV &&
      prev != STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL, EE_LIGHT_STATE_ON);
    prev = STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL;
  } else if (data > STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
             data < STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV &&
             prev != STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL) {
    event_raise((EventId)STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL, EE_LIGHT_STATE_ON);
    prev = STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL;
  } else if (data > STEERING_CONTROL_STALK_NO_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
             data < STEERING_CONTROL_STALK_NO_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV &&
             prev != STEERING_CONTROL_STALK_EVENT_NO_SIGNAL) {
    event_raise(prev, EE_LIGHT_STATE_OFF);
    prev = STEERING_CONTROL_STALK_EVENT_NO_SIGNAL;
  }

  uint16_t d = 0;
  adc_read_converted(chan, &d);
  printf("reading: %d\n", d);
}

StatusCode control_stalk_init() {
  status_ok_or_return(adc_periodic_reader_set_up_reader(PERIODIC_READER_ID_0, &reader_settings));
  status_ok_or_return(adc_periodic_reader_start(PERIODIC_READER_ID_0));

  GpioSettings sets = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  GpioAddress addr = { .port = GPIO_PORT_B, .pin = 0 };
  gpio_init_pin(&addr, &sets);
  adc_get_channel(addr, &chan);
  adc_set_channel(chan, true);
  return STATUS_CODE_OK;
}
