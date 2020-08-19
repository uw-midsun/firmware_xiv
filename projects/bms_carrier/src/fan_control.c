#include "fan_control.h"

#define FAN_TEMP_POLL_INTERVAL_US 500000
static Adt7476aStorage adt7476a_storage;
static SoftTimerId s_timer_id;
static uint32_t s_interval_us = FAN_TEMP_POLL_INTERVAL_US;

static void prv_measure_temps(SoftTimerId timer_id, void *context) {
  soft_timer_start(s_interval_us, &prv_measure_temps, NULL, &s_timer_id);
}

StatusCode fan_control_init(I2CSettings *i2c_settings, FanStorage *storage) {
  const Adt7476aSettings adt7476a_settings = {
    .smbalert_pin = SMBALERT_ADDR,
    .callback = (GpioItCallback)prv_callback,  // set to NULL for no callback
    .callback_context = NULL,
    .i2c_write_addr = I2C_WRITE_ADDR,
    .i2c_read_addr = I2C_READ_ADDR,
    .i2c = I2C_PORT,
    .i2c_settings = i2c_settings,
  };

  status_ok_or_return(adt7476a_init(&adt7476a_storage, &adt7476a_settings));

  prv_measure_currents(SOFT_TIMER_INVALID_TIMER, NULL);

  return STATUS_CODE_OK;
}
