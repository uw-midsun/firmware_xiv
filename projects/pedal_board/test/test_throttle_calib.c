
// Note : Although this is structured as a test, it is the actual calibration sequence for the
// throttle. It is meant to be used only once to retreive the values when the throttle is
// pressed and unpressed.

#include "ads1015.h"
#include "calib.h"
#include "crc32.h"
#include "delay.h"
#include "event_queue.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_calib.h"
#include "pedal_events.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "throttle_calib.h"
#include "throttle_data.h"
#include "unity.h"

static Ads1015Storage s_ads1015_storage;
static PedalCalibBlob s_calib_blob;
static ThrottleCalibrationStorage s_calibration_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  crc32_init();
  flash_init();
  event_queue_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };
  ads1015_init(&s_ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  TEST_ASSERT_OK(calib_init(&s_calib_blob, sizeof(s_calib_blob), true));
  throttle_calib_init(&s_calibration_storage);
}

void teardown_test(void) {}

void test_mech_throttle_calibration_run(void) {
  LOG_DEBUG("Please ensure the throttle is not being pressed.\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  throttle_calib_sample(&s_calibration_storage, &s_calib_blob.throttle_calib, PEDAL_UNPRESSED);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please press and hold the throttle\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  throttle_calib_sample(&s_calibration_storage, &s_calib_blob.throttle_calib, PEDAL_PRESSED);
  LOG_DEBUG("Completed sampling\n");

  calib_commit();
}
