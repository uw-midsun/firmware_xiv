#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "can.h"
#include "can_transmit.h"
#include "log.h"
#include "pd_error_defs.h"
#include "pd_fan_ctrl.h"
#include "pd_fan_ctrl_defs.h"
#include "soft_timer.h"

static FanCtrlStorage s_fan_storage;

// TODO(SOFT-373): confirm res values with hardware
// Resistance (Ohms) of the 10k thermistor under different temp conditions (Celsius)
#define BIAS_RESISTANCE 10000  // Resistance at standard conditions (25 degrees)
#define FAN_ENABLE_RES 5447    // Resistance value of thermistor at 40 degrees (fans turn on)
#define FAN_OVERTEMP_RES 2985  // Resistance value of thermistor at 55 degrees (overtemp)

// If adc reading is within this margin of max, interpret it as max reading
#define ADC_EPSILON_MV 50
#define FRONT_FAN_CTRL_MAX_VALUE_MV 1650

// Transmit fan error message if overtemp
static void prv_fan_overtemp_callback(void) {
  // Transmit stored ratio values for dcdc and enclosure based on ref reading
  CAN_TRANSMIT_REAR_PD_FAULT(s_fan_storage.fan_err_flags | FAN_OVERTEMP, s_fan_storage.dcdc_reading,
                             s_fan_storage.enclosure_reading, 0);
}

// Interrupt callback triggered when smbalert pin goes low
static void prv_fan_err_cb(const GpioAddress *address, void *context) {
  // Read status registers
  uint8_t reg1 = 0;
  uint8_t reg2 = 0;
  uint16_t *err_data = &s_fan_storage.fan_err_flags;
  adt7476a_get_status(s_fan_storage.i2c_port, ADT7476A_I2C_ADDRESS, &reg1, &reg2);
  if (reg1 & (VCC_EXCEEDED | VCCP_EXCEEDED)) {
    *err_data |= ERR_VCC_EXCEEDED;  // If overvoltage condition occured set flag
  }
  reg2 &= (FAN1_ERR | FAN2_ERR | FAN3_ERR | FAN4_ERR);  // Take only fan statuses
  *err_data |= (reg2);                                  // Compress to one uint16 fan_data
  if (s_fan_storage.is_front_pd) {
    CAN_TRANSMIT_FRONT_PD_FAULT(*err_data, 0);
  } else {
    CAN_TRANSMIT_REAR_PD_FAULT(*err_data, s_fan_storage.dcdc_reading,
                               s_fan_storage.enclosure_reading, 0);
  }
  s_fan_storage.fan_err_flags = 0;
}

// Converts front pd adc reading of potentiometer to fan speed percent
static void prv_front_temp_to_fan_percent(uint16_t v_measured, uint8_t *fan_speed) {
  if (s_fan_storage.ref_reading == 0) {
    // just to be safe to avoid divide-by-zero errors
    *fan_speed = 100;
    return;
  }
  if (v_measured == 0) {
    // just to be safe to avoid divide-by-zero errors
    *fan_speed = 0;
    return;
  }

  if (v_measured > (FRONT_FAN_CTRL_MAX_VALUE_MV + ADC_EPSILON_MV)) {
    *fan_speed = 0;
    return;
  }

  double ratio = FRONT_FAN_CTRL_MAX_VALUE_MV / 100.0;
  *fan_speed = v_measured / ratio;
}

// Converts rear pd adc reading of thermistor to fan speed percent
static void prv_rear_temp_to_fan_percent(uint16_t v_measured, uint8_t *fan_speed,
                                         uint16_t overtemp_flag) {
  uint16_t measured_res = 0;
  uint16_t v_ref = s_fan_storage.ref_reading;

  // calculate bias current for each thermistor
  double curr_bias = (double)(v_ref - v_measured) / BIAS_RESISTANCE;

  // Account for adc margin of error near max values, and account for (improbable) unsigned nonsense
  if (v_ref - v_measured < ADC_EPSILON_MV || v_ref < v_measured) {
    measured_res = FAN_OVERTEMP_RES;
  } else {
    measured_res = (double)v_measured / curr_bias;
  }

  // Check overtemp conditions
  if (measured_res <= FAN_OVERTEMP_RES) {
    s_fan_storage.fan_err_flags |= overtemp_flag;
    LOG_DEBUG("FAN OVERTEMP: %s\n", (overtemp_flag == DCDC_OVERTEMP) ? "DCDC" : "ENC VENT");
  }

  // Approximate with linear temp/resistance plot of resistances
  double fraction = (double)(measured_res - FAN_OVERTEMP_RES) / (FAN_ENABLE_RES - FAN_OVERTEMP_RES);
  *fan_speed = (uint8_t)((1.00 - fraction) * 100);
}

// Reads adc value for designated pin and sets fan speed at pwm accordingly
static StatusCode prv_set_fan_speed(GpioAddress read_pin, AdtPwmPort pwm, uint16_t overtemp_flag) {
  uint16_t reading;
  uint8_t fan_speed;

  // Get adc reading
  status_ok_or_return(adc_read_converted_pin(read_pin, &reading));

  // Convert to fan speed
  if (s_fan_storage.is_front_pd) {
    prv_front_temp_to_fan_percent(reading, &fan_speed);
  } else {
    prv_rear_temp_to_fan_percent(reading, &fan_speed, overtemp_flag);

    // Check overtemp conditions
    if (s_fan_storage.fan_err_flags & overtemp_flag) {
      fan_speed = 100;
      // Store reading as a ratio of ref_reading for error tx
      uint16_t reading_ratio = (reading * 1000) / s_fan_storage.ref_reading;
      if (overtemp_flag == DCDC_OVERTEMP) {
        s_fan_storage.dcdc_reading = reading_ratio;
      } else {
        s_fan_storage.enclosure_reading = reading_ratio;
      }
    }
  }

  // Write fan speed to specified pwm port
  return adt7476a_set_speed(s_fan_storage.i2c_port, fan_speed, pwm, ADT7476A_I2C_ADDRESS);
}

// Rear pd read temp values and adjust fan speeds on given interval
static void prv_rear_pd_fan_ctrl_update_speed(SoftTimerId id, void *context) {
  s_fan_storage.fan_err_flags = 0;  // Reset flags

  // Store reference adc value
  adc_read_converted(ADC_CHANNEL_REF, &s_fan_storage.ref_reading);

  // Set fan speed
  prv_set_fan_speed(s_fan_storage.enclosure_therm_pin, s_fan_storage.fan_pwm1, ENCLOSURE_OVERTEMP);
  prv_set_fan_speed(s_fan_storage.dcdc_therm_pin, s_fan_storage.fan_pwm2, DCDC_OVERTEMP);

  if (s_fan_storage.fan_err_flags) {
    prv_fan_overtemp_callback();
  }

  soft_timer_start_millis(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS, prv_rear_pd_fan_ctrl_update_speed,
                          NULL, NULL);
}

// Front pd read temp values and adjust fan speeds on given interval
static void prv_front_pd_fan_ctrl_update_speed(SoftTimerId id, void *context) {
  s_fan_storage.fan_err_flags = 0;  // Reset flags

  // Store reference adc value
  adc_read_converted(ADC_CHANNEL_REF, &s_fan_storage.ref_reading);

  // Convert adc reading to fan speed percent
  // A potentiometer is used to control driver ventilation manually
  prv_set_fan_speed(s_fan_storage.pot_pin, s_fan_storage.fan_pwm1, 0);
  prv_set_fan_speed(s_fan_storage.pot_pin, s_fan_storage.fan_pwm2, 0);

  soft_timer_start_millis(FRONT_FAN_CONTROL_REFRESH_PERIOD_MILLIS,
                          prv_front_pd_fan_ctrl_update_speed, NULL, NULL);
}

// Initialize fan control for given power distro
StatusCode pd_fan_ctrl_init(FanCtrlSettings *settings, bool is_front_pd) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_fan_storage.is_front_pd = is_front_pd;

  // Initialize adt7476a
  Adt7476aSettings adt_settings = {
    .callback = prv_fan_err_cb,
    .callback_context = NULL,
    .i2c = settings->i2c_port,
    .i2c_read_addr = settings->i2c_address,
    .i2c_write_addr = settings->i2c_address,
    .smbalert_pin = PD_SMBALERT_PIN,
  };
  Adt7476aStorage adt_storage;
  status_ok_or_return(adt7476a_init(&adt_storage, &adt_settings));

  // Initialize static fan ctrl storage
  s_fan_storage.i2c_port = settings->i2c_port;
  s_fan_storage.fan_pwm1 = settings->fan_pwm1;
  s_fan_storage.fan_pwm2 = settings->fan_pwm2;

  // Initialize gpio pins and adc channel for temperature circuits
  // Front PD has one pot pin to read, rear PD has 2 therm pins
  if (is_front_pd) {
    s_fan_storage.pot_pin = (GpioAddress)FRONT_PIN_FAN_POT;
    status_ok_or_return(adc_set_channel_pin(s_fan_storage.pot_pin, true));
    // Start operation of fan control for front pd
    soft_timer_start_millis(FRONT_FAN_CONTROL_REFRESH_PERIOD_MILLIS,
                            prv_front_pd_fan_ctrl_update_speed, NULL, NULL);
  } else {
    s_fan_storage.enclosure_reading = 1;  // Make sure tx values non-zero
    s_fan_storage.dcdc_reading = 1;
    s_fan_storage.enclosure_therm_pin = (GpioAddress)REAR_PIN_ENC_VENT_THERM;
    s_fan_storage.dcdc_therm_pin = (GpioAddress)REAR_PIN_DCDC_THERM;
    status_ok_or_return(adc_set_channel_pin(s_fan_storage.enclosure_therm_pin, true));
    status_ok_or_return(adc_set_channel_pin(s_fan_storage.dcdc_therm_pin, true));
    // Start operation of fan control for rear pd
    soft_timer_start_millis(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS,
                            prv_rear_pd_fan_ctrl_update_speed, NULL, NULL);
  }
  return STATUS_CODE_OK;
}
