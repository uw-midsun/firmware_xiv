#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "can.h"
#include "can_transmit.h"
#include "log.h"
#include "pd_fan_ctrl.h"
#include "pd_fan_ctrl_defs.h"
#include "soft_timer.h"

static FanCtrlStorage s_fan_storage;

static bool s_is_front_pd;

#define BIAS_RESISTANCE 10000
#define FAN_ENABLE_RES 5447    // Resistance value of thermistor at 40 degrees (fans turn on)
#define FAN_OVERTEMP_RES 2985  // Resistance value of thermistor at 55 degrees (overtemp)

// Transmit fan error message if overtemp
static void prv_fan_overtemp_callback(uint8_t fan_flags, uint16_t v_dcdc, uint16_t v_enc_vent) {
  fan_flags |= FAN_OVERTEMP_TRIGGERED;
  CAN_TRANSMIT_REAR_FAN_FAULT(fan_flags << 8, v_dcdc, v_enc_vent);
}

// Interrupt callback triggered when smbalert pin goes low
static void prv_fan_err_cb(const GpioAddress *address, void *context) {
  // Read status registers
  uint8_t reg1 = 0;
  uint8_t reg2 = 0;
  adt7476a_get_status(s_fan_storage.i2c, ADT7476A_I2C_WRITE_ADDRESS, &reg1, &reg2);
  reg1 &= (VCC_EXCEEDED | VCCP_EXCEEDED);                           // Take only voltage statuses
  reg2 &= (FAN1_STATUS | FAN2_STATUS | FAN3_STATUS | FAN4_STATUS);  // Take only fan statuses
  uint16_t fan_data = reg1 << 8 | reg2;  // Compress to one uint16 fan_data
  if (s_is_front_pd) {
    CAN_TRANSMIT_FRONT_FAN_FAULT(fan_data);
  } else {
    CAN_TRANSMIT_REAR_FAN_FAULT(fan_data, 0, 0);
  }
}

// Converts voltage readings to thermistor resistance
// Checks for overtemp conditions
static uint16_t prv_temp_convert(uint16_t *overtemp_flags, uint16_t v_ref, uint16_t v_meas,
                                 uint8_t flag) {
  // calculate bias current for each thermistor
  double curr_bias = (double)(v_ref - v_meas) / BIAS_RESISTANCE;
  uint16_t therm_res = 0;
  if (curr_bias > 0.0) {  // Check that v_ref != voltage measured
    therm_res = v_meas / curr_bias;
  } else {
    therm_res = FAN_OVERTEMP_RES;
  }
  // Check overtemp conditions
  if (therm_res <= FAN_OVERTEMP_RES) {
    *overtemp_flags |= flag;
    LOG_DEBUG("FAN OVERTEMP: %s\n", (flag == DCDC_OVERTEMP) ? "DCDC" : "ENC VENT");
  }
  return therm_res;
}

// Converts adc reading of thermistor to fan speed percent
static void prv_temp_to_fan_percent(uint16_t v_ref, uint16_t v_dcdc, uint16_t v_enc_vent,
                                    uint8_t *fan_speed_dcdc, uint8_t *fan_speed_enc_vent) {
  uint16_t fan_overtemp_flags = 0;  // Flag determining if overtemp condition exists
  uint16_t dcdc_therm_res = prv_temp_convert(&fan_overtemp_flags, v_ref, v_dcdc, DCDC_OVERTEMP);
  uint16_t enc_vent_therm_res =
      prv_temp_convert(&fan_overtemp_flags, v_ref, v_enc_vent, ENC_VENT_OVERTEMP);

  if (fan_overtemp_flags) {
    prv_fan_overtemp_callback(fan_overtemp_flags, v_dcdc, v_enc_vent);
    *fan_speed_dcdc = 100;
    *fan_speed_enc_vent = 100;
    return;
  }

  // Approximate with linear temp/resistance plot of resistances
  double dcdc_fraction =
      (double)(dcdc_therm_res - FAN_OVERTEMP_RES) / (FAN_ENABLE_RES - FAN_OVERTEMP_RES);
  double enc_vent_fraction =
      (double)(enc_vent_therm_res - FAN_OVERTEMP_RES) / (FAN_ENABLE_RES - FAN_OVERTEMP_RES);
  *fan_speed_dcdc = (uint8_t)((1.00 - dcdc_fraction) * 100);
  *fan_speed_enc_vent = (uint8_t)((1.00 - enc_vent_fraction) * 100);
}

// Rear pd read temp values and adjust fan speeds on given interval
static void prv_rear_pd_fan_ctrl_update_speed(SoftTimerId id, void *context) {
  uint16_t ref_reading = 0;
  uint16_t therm_reading_enc_vent = 0;
  uint16_t therm_reading_dcdc = 0;
  uint8_t fan_speed_dcdc = 0;
  uint8_t fan_speed_enc_vent = 0;

  // read adc values
  adc_read_converted(ADC_CHANNEL_REF, &ref_reading);
  adc_read_converted_pin(s_fan_storage.enc_vent_therm_pin, &therm_reading_enc_vent);
  adc_read_converted_pin(s_fan_storage.dcdc_therm_pin, &therm_reading_dcdc);

  // convert adc temp vals to fan percentages, call error cb if either is overtemp
  prv_temp_to_fan_percent(ref_reading, therm_reading_enc_vent, therm_reading_dcdc, &fan_speed_dcdc,
                          &fan_speed_enc_vent);
  // set speed of fans based on above value
  adt7476a_set_speed(s_fan_storage.i2c, fan_speed_enc_vent, s_fan_storage.fan_pwm1,
                     ADT7476A_I2C_WRITE_ADDRESS);
  adt7476a_set_speed(s_fan_storage.i2c, fan_speed_dcdc, s_fan_storage.fan_pwm2,
                     ADT7476A_I2C_WRITE_ADDRESS);

  soft_timer_start_millis(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS,
                          &prv_rear_pd_fan_ctrl_update_speed, NULL, NULL);
}

// Rear pd read temp values and adjust fan speeds on given interval
static void prv_front_pd_fan_ctrl_update_speed(SoftTimerId id, void *context) {
  uint16_t pot_reading = 0;
  uint16_t ref_reading = 0;
  uint8_t front_fan_speed = 0;
  adc_read_converted(ADC_CHANNEL_REF, &ref_reading);
  adc_read_converted_pin(s_fan_storage.pot_pin, &pot_reading);
  double ratio = ref_reading / 100.0;
  // Convert adc reading to fan speed percent
  front_fan_speed = pot_reading / ratio;
  adt7476a_set_speed(s_fan_storage.i2c, front_fan_speed, s_fan_storage.fan_pwm1,
                     ADT7476A_I2C_WRITE_ADDRESS);
  adt7476a_set_speed(s_fan_storage.i2c, front_fan_speed, s_fan_storage.fan_pwm2,
                     ADT7476A_I2C_WRITE_ADDRESS);
  soft_timer_start_millis(FRONT_FAN_CONTROL_REFRESH_PERIOD_MILLIS,
                          &prv_front_pd_fan_ctrl_update_speed, NULL, NULL);
}

// Initialize fan control for given power distro
StatusCode pd_fan_ctrl_init(FanCtrlSettings *settings, bool is_front_pd) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_is_front_pd = is_front_pd;
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = POWER_DISTRIBUTION_I2C_SCL_PIN,
    .sda = POWER_DISTRIBUTION_I2C_SDA_PIN,
  };
  // Initialize adt7476a
  Adt7476aSettings adt_settings = {
    .callback = prv_fan_err_cb,
    .callback_context = NULL,
    .i2c = settings->i2c,
    .i2c_settings = i2c_settings,
    .i2c_read_addr = ADT7476A_I2C_READ_ADDRESS,
    .i2c_write_addr = ADT7476A_I2C_WRITE_ADDRESS,
    .smbalert_pin = is_front_pd ? (GpioAddress)FRONT_PIN_SMBALERT : (GpioAddress)REAR_PIN_SMBALERT,
  };
  Adt7476aStorage adt_storage;
  status_ok_or_return(adt7476a_init(&adt_storage, &adt_settings));

  // Initialize static fan ctrl storage
  s_fan_storage.i2c = adt_storage.i2c;
  s_fan_storage.fan_pwm1 = settings->fan_pwm1;
  s_fan_storage.fan_pwm2 = settings->fan_pwm2;

  // Initialize gpio pins and adc channel for temperature circuits
  // Front PD has one pot pin to read, rear PD has 2 therm pins
  if (s_is_front_pd) {
    s_fan_storage.pot_pin = (GpioAddress)FRONT_PIN_FAN_POT;
    status_ok_or_return(adc_set_channel_pin(s_fan_storage.pot_pin, true));
    // Start operation of fan control for front pd
    soft_timer_start_millis(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS,
                            &prv_front_pd_fan_ctrl_update_speed, NULL, NULL);
  } else {
    s_fan_storage.enc_vent_therm_pin = (GpioAddress)REAR_PIN_ENC_VENT_THERM;
    s_fan_storage.dcdc_therm_pin = (GpioAddress)REAR_PIN_DCDC_THERM;
    status_ok_or_return(adc_set_channel_pin(s_fan_storage.enc_vent_therm_pin, true));
    status_ok_or_return(adc_set_channel_pin(s_fan_storage.dcdc_therm_pin, true));
    // Start operation of fan control for rear pd
    soft_timer_start_millis(REAR_FAN_CONTROL_REFRESH_PERIOD_MILLIS,
                            &prv_rear_pd_fan_ctrl_update_speed, NULL, NULL);
  }
  return STATUS_CODE_OK;
}
