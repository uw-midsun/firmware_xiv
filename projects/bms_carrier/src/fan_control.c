#include "bms.h"
#include "math.h"

static Adt7476aStorage adt7476a_storage;
static SoftTimerId s_timer_id;
static uint32_t s_interval_us = FAN_TEMP_POLL_INTERVAL_US;

static void prv_measure_temps(SoftTimerId timer_id, void *context) {
  FanStorage *storage = (FanStorage *)context;

  uint16_t max = 0;
  uint8_t fan_speed = 0;

  for (int i = 0; i < NUM_THERMISTORS; i++) {
    if (storage->readings->temps[i] > max) {
      max = storage->readings->temps[i];
    }
  }

  // calculate fan speed

  (max > MAX_BATTERY_TEMP) ? (fan_speed = MAX_FAN_SPEED)
                           : (fan_speed = floor(max * (MAX_FAN_SPEED / MAX_BATTERY_TEMP)));

  status_ok_or_return(adt7476a_set_speed(BMS_FAN_CTRL_I2C_PORT_1, fan_speed, ADT_PWM_PORT_1,
                                         storage->i2c_write_addr));
  status_ok_or_return(adt7476a_set_speed(BMS_FAN_CTRL_I2C_PORT_1, fan_speed, ADT_PWM_PORT_2,
                                         storage->i2c_write_addr));

  storage->speed = fan_speed;
  storage->status = STATUS_CODE_OK;

  soft_timer_start(s_interval_us, &prv_measure_temps, storage, &s_timer_id);
}

StatusCode fan_control_init(FanControlSettings *settings, FanStorage *storage) {
  const Adt7476aSettings adt7476a_settings = {
    .smbalert_pin = BMS_FAN_ALERT_PIN,
    .callback = (GpioItCallback)settings->callback,  // set to NULL for no callback
    .callback_context = settings->callback_context,
    .i2c_write_addr = storage->i2c_write_addr,
    .i2c_read_addr = storage->i2c_read_addr,
    .i2c = BMS_FAN_CTRL_I2C_PORT_1,
    .i2c_settings = settings->i2c_settings,
  };

  status_ok_or_return(adt7476a_init(&adt7476a_storage, &adt7476a_settings));

  prv_measure_temps(SOFT_TIMER_INVALID_TIMER, storage);

  return STATUS_CODE_OK;
}
