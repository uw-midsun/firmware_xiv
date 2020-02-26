#include "ads1015.h"
#include "pedal_calib.h"

static Ads1015Storage s_ads1015_storage = { 0 };
static PedalCalibBlob s_pedal_calib_blob = { 0 };

StatusCode brake_calib_init() {
  LOG_DEBUG("WORKING\n");
  // initialize all the modules
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
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
  status_ok_or_return(calib_init(&s_pedal_calib_blob, sizeof(s_pedal_calib_blob), false));
  PedalCalibBlob *pedal_calib_blob = calib_blob();
  // this should also take in the calib blob
  pedal_data_init(&s_ads1015_storage);
  return STATUS_CODE_OK;
}