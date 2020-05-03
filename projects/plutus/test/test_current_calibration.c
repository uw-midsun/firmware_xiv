#include <inttypes.h>
#include "calib.h"
#include "crc32.h"
#include "current_calibration.h"
#include "current_sense.h"
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "plutus_calib.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// The module does not know what current the adc readings correspond to, so keep an arbitrary
// max point for testing
#define TEST_CURRENT_CALIBRATION_MAX 5

#define TEST_CURRENT_CALIBRATION_DELAY_SECONDS 4

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();
  flash_init();
}

void teardown_test(void) {}

void test_current_calibration_sample(void) {
  // Calibration routing demonstration
  CurrentCalibrationStorage storage = { 0 };
  PlutusCalibBlob calib_blob = { 0 };

  TEST_ASSERT_OK(calib_init(&calib_blob, sizeof(calib_blob), true));

  const LtcAdcSettings adc_settings = {
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_B, 12 },
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 750000,
    .filter_mode = LTC_ADC_FILTER_50HZ_60HZ,
  };

  // Reset calibration and obtain zero point
  TEST_ASSERT_OK(current_calibration_init(&storage, &adc_settings));
  LOG_DEBUG("Set current to 0 A\n");
  delay_s(TEST_CURRENT_CALIBRATION_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(
      current_calibration_sample_point(&storage, &calib_blob.current_calib.zero_point, 0));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            calib_blob.current_calib.zero_point.voltage,
            calib_blob.current_calib.zero_point.current);

  // Reset calibration and obtain max point
  LOG_DEBUG("Set current to %d A\n", TEST_CURRENT_CALIBRATION_MAX);
  delay_s(TEST_CURRENT_CALIBRATION_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(current_calibration_sample_point(&storage, &calib_blob.current_calib.max_point,
                                                  TEST_CURRENT_CALIBRATION_MAX * 1000000));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            calib_blob.current_calib.max_point.voltage, calib_blob.current_calib.max_point.current);

  calib_commit();
}
