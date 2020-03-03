#include "ads1015.h"
#include "can.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
// include all the modules
#include "calib.h"
#include "pedal_calib.h"
#include "pedal_shared_resources_provider.h"
#include "pedal_data_tx.h"
#include "pedal_events.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define CAN_DEVICE_ID 0x1

static Ads1015Storage s_ads1015_storage = { 0 };
static PedalCalibBlob s_calib_blob = { 0 };

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = PEDAL_CAN_RX,
  .tx_event = PEDAL_CAN_TX,
  .fault_event = PEDAL_CAN_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
};

int main() {
  LOG_DEBUG("WORKING\n");
  // initialize all the modules
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &can_settings);
  // this is need for x86 but not for the stm32s
  flash_init();
  LOG_DEBUG("Initialized modules\n");

  // setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };
  ads1015_init(&s_ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  // we expect calibration blog to be there already
  status_ok_or_return(calib_init(&s_calib_blob, sizeof(s_calib_blob), false));
  PedalCalibBlob *pedal_calib_blob = calib_blob();
  pedal_resources_init(&s_ads1015_storage, pedal_calib_blob);
  pedal_data_tx_init();

  LOG_DEBUG("Starting...\n");
  Event e = { 0 };
  while (true) {
    event_process(&e);
    can_process_event(&e);
  }
  return 0;
}
