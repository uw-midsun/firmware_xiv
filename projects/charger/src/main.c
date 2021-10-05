#include "adc.h"
#include "battery_monitor.h"
#include "begin_sequence.h"
#include "can.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "control_pilot.h"
#include "charger_events.h"
#include "connection_sense.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "stop_sequence.h"
#include "log.h"

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CHARGER,  //
  .bitrate = CAN_HW_BITRATE_500KBPS,       //
  .tx = { GPIO_PORT_A, 12 },               //
  .rx = { GPIO_PORT_A, 11 },               //
  .rx_event = CHARGER_CAN_EVENT_RX,        //
  .tx_event = CHARGER_CAN_EVENT_TX,        //
  .fault_event = CHARGER_CAN_EVENT_FAULT,  //
  .loopback = false                        //
};


static void prv_start_periodic_read(SoftTimerId id, void *context) {
  uint16_t current = control_pilot_get_current();
  GpioAddress ad = {
    .pin = 5,
    .port = GPIO_PORT_A,
  };
  GpioAddress sense = CHARGER_SENSE_PIN;
  GpioState sense_state = GPIO_STATE_LOW;
  gpio_get_state(&sense, &sense_state);
  uint16_t reading = 0;
  adc_read_converted_pin(ad, &reading);

  LOG_DEBUG("CHARGER SENSE: %d, ADC VALUE: %d\n", sense_state, reading);
  soft_timer_start_millis(1000, prv_start_periodic_read, NULL, NULL);
}

int main(void) {
  gpio_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);


  GpioSettings settings = {
    .direction = GPIO_DIR_IN,       //
    .state = GPIO_STATE_LOW,         //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_ANALOG  //
  };

  GpioSettings sense_set = {
    .direction = GPIO_DIR_IN,       //
    .state = GPIO_STATE_LOW,         //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE //
  };

  GpioAddress ad = {
    .pin = 5,
    .port = GPIO_PORT_A,
  };

  GpioAddress sense = CHARGER_SENSE_PIN;
  gpio_init_pin(&ad, &settings);
  gpio_init_pin(&ad, &sense_set);

  charger_controller_init();
  begin_sequence_init();
  // battery_monitor_init();

  soft_timer_start_millis(1000, prv_start_periodic_read, NULL, NULL);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      begin_sequence_process_event(&e);
      stop_sequence_process_event(&e);
    }
  }

  return 0;
}
