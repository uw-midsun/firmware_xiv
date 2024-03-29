#include "sense_mppt.h"

#include "data_store.h"
#include "exported_enums.h"
#include "fault_handler.h"
#include "log.h"
#include "mppt.h"
#include "sense.h"
#include "solar_boards.h"
#include "spv1020_mppt.h"
#include "status.h"

static SpiPort s_spi_port;
static Mppt s_indices[MAX_SOLAR_BOARD_MPPTS];
static float s_current_scaling_factor;
static float s_vin_scaling_factor;

static void prv_check_status_for_faults(Mppt mppt, uint8_t status) {
  uint8_t ovc_branch_bitmask;
  if (spv1020_is_overcurrent(status, &ovc_branch_bitmask)) {
    uint8_t ovc_data = (ovc_branch_bitmask << 4) | mppt;
    fault_handler_raise_fault(EE_SOLAR_FAULT_MPPT_OVERCURRENT, ovc_data);
  }
  if (spv1020_is_overvoltage(status)) {
    fault_handler_raise_fault(EE_SOLAR_FAULT_MPPT_OVERVOLTAGE, mppt);
  }
  if (spv1020_is_overtemperature(status)) {
    fault_handler_raise_fault(EE_SOLAR_FAULT_MPPT_OVERTEMPERATURE, mppt);
  }
}

static uint32_t prv_scale_raw(uint16_t raw, float scaling_factor) {
  return (uint32_t)(raw * scaling_factor);
}

static void prv_sense_cycle_callback(void *context) {
  Mppt mppt = *(Mppt *)context;

  uint16_t current, vin, pwm;
  uint8_t status;

  if (status_ok(mppt_read_current(s_spi_port, &current, mppt))) {
    data_store_set(DATA_POINT_MPPT_CURRENT(mppt), prv_scale_raw(current, s_current_scaling_factor));
  } else {
    LOG_WARN("Error reading current from MPPT %d\n", mppt);
  }

  if (status_ok(mppt_read_voltage_in(s_spi_port, &vin, mppt))) {
    data_store_set(DATA_POINT_MPPT_VOLTAGE(mppt), prv_scale_raw(vin, s_vin_scaling_factor));
  } else {
    LOG_WARN("Error reading voltage from MPPT %d\n", mppt);
  }

  if (status_ok(mppt_read_pwm(s_spi_port, &pwm, mppt))) {
    data_store_set(DATA_POINT_MPPT_PWM(mppt), pwm);
  } else {
    LOG_WARN("Error reading PWM from MPPT %d\n", mppt);
  }

  if (status_ok(mppt_read_status(s_spi_port, &status, mppt))) {
    data_store_set(DATA_POINT_MPPT_STATUS(mppt), status);
    prv_check_status_for_faults(mppt, status);
  } else {
    LOG_WARN("Error reading status from MPPT %d\n", mppt);
  }
}

StatusCode sense_mppt_init(SenseMpptSettings *settings) {
  if (settings == NULL || settings->mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_spi_port = settings->spi_port;
  s_current_scaling_factor = settings->mppt_current_scaling_factor;
  s_vin_scaling_factor = settings->mppt_vin_scaling_factor;

  for (Mppt mppt = 0; mppt < settings->mppt_count; mppt++) {
    s_indices[mppt] = mppt;
    status_ok_or_return(sense_register(prv_sense_cycle_callback, &s_indices[mppt]));
  }

  return STATUS_CODE_OK;
}
