// A simple smoke test for BTS7200

// Periodically take reading from user selected channels and log the result
// Configurable items: wait time, FRONT or REAR power distro selection, channels to be tested

#include "adc.h"
#include "bts_7200_current_sense.h"
#include "current_measurement_config.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"
#include "wait.h"

// Smoke test settings. Can be modified to fit testing purpose.
#define CURRENT_MEASURE_INTERVAL_MS 500  // Set wait time between each set of readings
#define IS_FRONT_POWER_DISTRO true       // Set whether to test FRONT or REAR power distro

// Maximum number of channels that can be tested,
// which is the same number of bts7200 used in front/rear power distribution
#define MAX_TEST_CHANNELS 8
// Set of channels to be tested, the array contains the indices of the BTS7200 in the front/rear HW
// config Details at
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/1419740028/BTS7200+smoke+test+user+guide
// static uint8_t s_test_channels[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

#define I2C_PORT I2C_PORT_2

#define CHANNEL 7

// I2C_PORT_1 has SDA on PB9 and SCL on PB8
// I2C_PORT_2 has SDA on PB11 and SCL on PB10
#define PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

static Bts7200Storage s_bts7200_storage;
static PowerDistributionCurrentHardwareConfig *s_hw_config;

static void prv_start_read(SoftTimerId, void *);

// static void prv_log(uint16_t meas0, uint16_t meas1, void *context) {
//   uintptr_t i = (uintptr_t)context;
//   LOG_DEBUG("Channel: %d; current_0: %d, current_1: %d\n", s_test_channels[i], meas0, meas1);
//   i++;
//   if (i == SIZEOF_ARRAY(s_test_channels)) {
//     soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_start_read, (void *)0, NULL);
//   } else {
//     prv_start_read(SOFT_TIMER_INVALID_TIMER, (void *)i);
//   }
// }

// static void prv_start_read(SoftTimerId timer_id, void *context) {
//   uintptr_t i = (uintptr_t)context;
//   mux_set(&s_hw_config->mux_address, s_hw_config->bts7200s[s_test_channels[i]].mux_selection);
//   s_bts7200_storages[i].callback = prv_log;
//   s_bts7200_storages[i].callback_context = (void *)i;
//   bts_7200_get_measurement_with_delay(&s_bts7200_storages[i]);
// }

static void prv_callback(uint16_t meas0, uint16_t meas1, void *context) {
  LOG_DEBUG("meas0=%d, meas1=%d\n", meas0, meas1);
}

int main() {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = PIN_I2C_SDA,
    .scl = PIN_I2C_SCL,
  };
  i2c_init(I2C_PORT, &i2c_settings);

  s_hw_config = IS_FRONT_POWER_DISTRO ? &FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG
                                      : &REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;

  status_ok_or_return(mux_init(&s_hw_config->mux_address));

  // // Initialize and start the BTS7200s
  // Bts7200Pca9539rSettings bts_7200_settings = {
  //   .sense_pin = &s_hw_config->mux_address.mux_output_pin,
  //   .i2c_port = s_hw_config->i2c_port,
  // };

  // for (uint8_t i = 0; i < SIZEOF_ARRAY(s_test_channels); i++) {
  //   bts_7200_settings.select_pin = &s_hw_config->bts7200s[s_test_channels[i]].dsel_pin;
  //   status_ok_or_return(bts_7200_init_pca9539r(&s_bts7200_storages[i], &bts_7200_settings));
  // }

  Bts7200Pca9539rSettings bts_7200_settings = {
    .sense_pin = &s_hw_config->mux_address.mux_output_pin,
    .select_pin = &s_hw_config->bts7200s[CHANNEL].dsel_pin,
    .i2c_port = s_hw_config->i2c_port,
    .callback = prv_callback,
  };
  status_ok_or_return(bts_7200_init_pca9539r(&s_bts7200_storage, &bts_7200_settings));

  // enabling steering for convenience in smoke testing
  Pca9539rGpioAddress steering_en = { .i2c_address = 0x76, .pin = PCA9539R_PIN_IO1_3 };
  pca9539r_gpio_set_state(&steering_en, PCA9539R_GPIO_STATE_HIGH);
  mux_set(&s_hw_config->mux_address, s_hw_config->bts7200s[CHANNEL].mux_selection);

  bts_7200_get_measurement_with_delay(&s_bts7200_storage);

  // soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_start_read, (void *)0, NULL);
  while (true) {
    wait();
  }

  return 0;
}
