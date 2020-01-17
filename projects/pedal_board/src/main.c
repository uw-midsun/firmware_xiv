#include "ads1015.h"
#include "can.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
// include all the modules
#include "brake_fsm.h"
#include "brake_monitor.h"
#include "drive_fsm.h"
#include "pedal_events.h"
#include "pedal_can.h"
#include "status.h"
#include "test_helpers.h"
#include "throttle.h"
#include "unity.h"

#define CAN_DEVICE_ID 0x1

static Fsm drive_fsm;
static Fsm brake_fsm; 
static Ads1015Storage ads1015_storage;
static CanStorage can_storage;
static ThrottleStorage throttle_storage;

int main() {
  LOG_DEBUG("WORKING\n");
  // initialize all the modules
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  LOG_DEBUG("Initialized modules\n");

  const CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PEDAL_CAN_RX,
    .tx_event = PEDAL_CAN_TX,
    .fault_event = PEDAL_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },  // CHANGE
    .rx = { GPIO_PORT_A, 11 },  // CHANGE
  };
  pedal_can_init(&can_storage, &can_settings);

  drive_fsm_init(&drive_fsm, &throttle_storage);
  brake_fsm_init(&brake_fsm);

  // setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                   //
    .scl = { .port = GPIO_PORT_B, .pin = 5 },  // figure out later
    .sda = { .port = GPIO_PORT_B, .pin = 5 },  // figure out later
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 5 };  // CHANGE
  ads1015_init(&ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  
  brake_monitor_init(&ads1015_storage);
  throttle_init(&throttle_storage);

  Event e = { 0 };
  while (true) {
    event_process(&e);

    drive_fsm_process_event(&drive_fsm, &e);
    brake_fsm_process_event(&e);
    
    // perhaps distinguish which events are actually for can
    can_process_event(&e);
    pedal_can_process_event(&e);
  }
  return 0;
}
