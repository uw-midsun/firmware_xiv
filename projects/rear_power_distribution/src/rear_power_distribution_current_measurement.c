#include "rear_power_distribution_current_measurement.h"

#include <stddef.h>
#include "bts_7200_current_sense.h"
#include "mcp23008_gpio_expander.h"
#include "sn74_mux.h"

#define DSEL_MCP23008_I2C 0x20

// These are global constants declared in the header file
const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SDA_ADDRESS = { .port = GPIO_PORT_B,
                                                                      .pin = 11 };
const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SCL_ADDRESS = { .port = GPIO_PORT_B,
                                                                      .pin = 10 };

typedef enum {
  // first thing named is when DSEL is 0, then when DSEL is 1
  REAR_POWER_DISTRIBUTION_BTS7200_CTR_BRK_STROBE = 0,
  REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_BRK_LIGHT,
  REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_TURN,
  REAR_POWER_DISTRIBUTION_BTS7200_SOLAR_TELEMETRY,
  // there's also a CAM_CHARGER bts7200 on the rear board (?)
  NUM_REAR_POWER_DISTRIBUTION_BTS7200,
} RearPowerDistributionBts7200;

static const Mcp23008GpioAddress s_bts7200_addresses[] = {
  [REAR_POWER_DISTRIBUTION_BTS7200_CTR_BRK_STROBE] = { .i2c_address = DSEL_MCP23008_I2C, .pin = 1 },
  [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_BRK_LIGHT] = { .i2c_address = DSEL_MCP23008_I2C,
                                                             .pin = 2 },
  [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_TURN] = { .i2c_address = DSEL_MCP23008_I2C,
                                                        .pin = 3 },
  [REAR_POWER_DISTRIBUTION_BTS7200_SOLAR_TELEMETRY] = { .i2c_address = DSEL_MCP23008_I2C,
                                                        .pin = 7 },
};

static const uint8_t s_bts7200_to_mux_sel[] = {
  [REAR_POWER_DISTRIBUTION_BTS7200_CTR_BRK_STROBE] = 2,
  [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_BRK_LIGHT] = 3,
  [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_TURN] = 0,
  [REAR_POWER_DISTRIBUTION_BTS7200_SOLAR_TELEMETRY] = 7,
};

// again, rear power distribution mux address
static const Sn74MuxAddress s_mux_address = {
  .sel_pins =
      {
          { .port = GPIO_PORT_A, .pin = 6 },  //
          { .port = GPIO_PORT_A, .pin = 5 },  //
          { .port = GPIO_PORT_A, .pin = 4 },  //
      },
  .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },  //
};

static RearPowerDistributionCurrentStorage s_storage = { 0 };
static Bts7200Storage s_bts7200_storages[NUM_REAR_POWER_DISTRIBUTION_BTS7200];
static SoftTimerId s_timer_id;

// is this ok both as the BTS7200 update interval and this module's update interval?
static uint16_t s_interval_us;
static RearPowerDistributionCurrentCallback s_callback;
static void *s_callback_context;

static void prv_measure_currents(SoftTimerId timer_id, void *context) {
  // read from all the BTS7200s in order
  for (RearPowerDistributionBts7200 i = 0; i < NUM_REAR_POWER_DISTRIBUTION_BTS7200; i++) {
    sn74_mux_set(&s_mux_address, s_bts7200_to_mux_sel[i]);
    bts_7200_get_measurement(&s_bts7200_storages[i], &s_storage.measurements[2 * i],
                             &s_storage.measurements[2 * i + 1]);
  }

  if (s_callback) {
    s_callback(s_callback_context);
  }

  soft_timer_start(s_interval_us, &prv_measure_currents, NULL, &s_timer_id);
}

StatusCode rear_power_distribution_current_measurement_init(
    RearPowerDistributionCurrentSettings *settings) {
  s_interval_us = settings->interval_us;
  s_callback = settings->callback;
  s_callback_context = settings->callback_context;

  status_ok_or_return(mcp23008_gpio_init(DSEL_MCP23008_I2C));
  status_ok_or_return(sn74_mux_init_mux(&s_mux_address));

  // enable ADC on the output pin
  AdcChannel channel;
  adc_get_channel(s_mux_address.mux_output_pin, &channel);
  adc_set_channel(channel, true);

  // initialize and start the BTS7200s
  Bts7200Mcp23008Settings bts_7200_settings = {
    .sense_pin = &s_mux_address.mux_output_pin,
    .interval_us = s_interval_us,
  };
  for (RearPowerDistributionBts7200 i = 0; i < NUM_REAR_POWER_DISTRIBUTION_BTS7200; i++) {
    bts_7200_settings.select_pin = &s_bts7200_addresses[i];
    bts_7200_init_mcp23008(&s_bts7200_storages[i], &bts_7200_settings);
    bts_7200_start(&s_bts7200_storages[i]);
  }

  // measure the currents immediately; the callback doesn't use the timer it's passed
  prv_measure_currents(SOFT_TIMER_INVALID_TIMER, NULL);

  return STATUS_CODE_OK;
}

// should this pass a RearPowerDistributionCurrentStorage** maybe?
StatusCode rear_power_distribution_current_measurement_get_storage(
    RearPowerDistributionCurrentStorage *storage) {
  *storage = s_storage;
  return STATUS_CODE_OK;
}

StatusCode rear_power_distribution_stop(void) {
  for (RearPowerDistributionBts7200 i = 0; i < NUM_REAR_POWER_DISTRIBUTION_BTS7200; i++) {
    bts_7200_stop(&s_bts7200_storages[i]);
  }
  soft_timer_cancel(s_timer_id);
  return STATUS_CODE_OK;
}
