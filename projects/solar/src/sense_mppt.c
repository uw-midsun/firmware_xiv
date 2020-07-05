#include "sense_mppt.h"

#include "data_store.h"
#include "event_queue.h"
#include "log.h"
#include "mppt.h"
#include "sense.h"
#include "solar_boards.h"
#include "solar_events.h"
#include "spv1020_mppt.h"
#include "status.h"

static SpiPort s_spi_port;
static uint8_t s_indices[MAX_SOLAR_BOARD_MPPTS];

static void prv_sense_cycle_callback(void *context) {
  uint8_t mppt = *(uint8_t *)context;

  uint16_t current, vin, pwm;
  uint8_t status;
  if (!status_ok(mppt_read_current(s_spi_port, &current, mppt))) {
    LOG_WARN("Error reading current from MPPT %d\n", mppt);
    return;
  }
  if (!status_ok(mppt_read_voltage_in(s_spi_port, &vin, mppt))) {
    LOG_WARN("Error reading voltage from MPPT %d\n", mppt);
    return;
  }
  if (!status_ok(mppt_read_pwm(s_spi_port, &pwm, mppt))) {
    LOG_WARN("Error reading PWM from MPPT %d\n", mppt);
    return;
  }
  if (!status_ok(mppt_read_status(s_spi_port, &status, mppt))) {
    LOG_WARN("Error reading status from MPPT %d\n", mppt);
    return;
  }

  // this is almost certainly not the best way to get the |mppt|th data point
  data_store_set(DATA_POINT_MPPT_CURRENT_1 + mppt, current);
  data_store_set(DATA_POINT_MPPT_VOLTAGE_1 + mppt, vin);
  data_store_set(DATA_POINT_MPPT_PWM_1 + mppt, pwm);
  data_store_set(DATA_POINT_CR_BIT_1 + mppt, spv1020_is_cr_bit_set(status));

  uint8_t ovc_branch_bitmask;
  if (spv1020_is_overcurrent(status, &ovc_branch_bitmask)) {
    uint16_t ovc_data = (ovc_branch_bitmask << 8) | mppt;
    event_raise(SOLAR_FAULT_EVENT_MPPT_OVERCURRENT, ovc_data);
  }
  if (spv1020_is_overvoltage(status)) {
    event_raise(SOLAR_FAULT_EVENT_MPPT_OVERVOLTAGE, mppt);
  }
  if (spv1020_is_overtemperature(status)) {
    event_raise(SOLAR_FAULT_EVENT_MPPT_OVERTEMPERATURE, mppt);
  }
}

StatusCode sense_mppt_init(SenseMpptSettings *settings) {
  if (settings->mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  status_ok_or_return(mppt_init());

  s_spi_port = settings->spi_port;
  for (uint8_t mppt = 0; mppt < settings->mppt_count; mppt++) {
    s_indices[mppt] = mppt;
    status_ok_or_return(sense_register(prv_sense_cycle_callback, &s_indices[mppt]));
  }

  return STATUS_CODE_OK;
}
