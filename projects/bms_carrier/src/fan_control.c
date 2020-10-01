#include "fan_control.h"

#include "bms.h"
#include "log.h"
#include "math.h"

static Adt7476aStorage s_adt7476a_storage;
static uint32_t s_interval_ms;

static void prv_measure_temps(SoftTimerId timer_id, void *context) {
  FanStorage *storage = (FanStorage *)context;

  uint16_t max = 0;
  uint8_t fan_speed = 0;
  StatusCode pwm_status_1;
  StatusCode pwm_status_2;

  for (int i = 0; i < NUM_THERMISTORS; i++) {
    if (storage->readings->temps[i] > max) {
      max = storage->readings->temps[i];
    }
  }

  // calculate fan speed

  (max > MAX_BATTERY_TEMP)
      ? (fan_speed = MAX_FAN_SPEED)
      : (fan_speed = (max * ((double)MAX_FAN_SPEED / (double)MAX_BATTERY_TEMP)));

  pwm_status_1 = adt7476a_set_speed(BMS_FAN_CTRL_I2C_PORT_1, fan_speed, ADT_PWM_PORT_1,
                                    storage->i2c_write_addr);
  pwm_status_2 = adt7476a_set_speed(BMS_FAN_CTRL_I2C_PORT_1, fan_speed, ADT_PWM_PORT_2,
                                    storage->i2c_write_addr);

  for (int i = 0; i < ADT_7476A_NUM_FANS; i++) {
    (i < NUM_FANS_PER_OUTPUT) ? (storage->statuses[i] = pwm_status_1)
                              : (storage->statuses[i] = pwm_status_2);
  }

  if (pwm_status_1 != STATUS_CODE_OK || pwm_status_2 != STATUS_CODE_OK) {
    storage->status = STATUS_CODE_UNKNOWN;
  } else {
    storage->status = STATUS_CODE_OK;
  }

  storage->speed = fan_speed;

  soft_timer_start_millis(s_interval_ms, &prv_measure_temps, storage, NULL);
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

  s_interval_ms = settings->poll_interval_ms;

  status_ok_or_return(adt7476a_init(&s_adt7476a_storage, &adt7476a_settings));

  prv_measure_temps(SOFT_TIMER_INVALID_TIMER, storage);

  return STATUS_CODE_OK;
}
