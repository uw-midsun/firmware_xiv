#include "current_sense.h"

#include <string.h>

#include "ads1259_adc.h"
#include "bms.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "log.h"
#include "soft_timer.h"

static Ads1259Storage s_ads1259_storage;
static bool s_is_charging;

static void prv_ads_error_cb(Ads1259StatusCode code, void *context) {
  if (code == ADS1259_STATUS_CODE_OUT_OF_RANGE) {
    LOG_WARN("ADS1259 ERROR: OUT OF RANGE\n");
  } else if (code == ADS1259_STATUS_CODE_CHECKSUM_FAULT) {
    LOG_WARN("ADS1259 ERROR: CHECKSUM FAULT\n");
  }

  if (code != ADS1259_STATUS_CODE_OK) {
    fault_bps_set(EE_BPS_STATE_FAULT_CURRENT_SENSE);
  } else {
    fault_bps_clear(EE_BPS_STATE_FAULT_CURRENT_SENSE);
  }
}

// returns value in centiamps
static int16_t prv_voltage_to_current(double reading) {
  // current = voltage * 100, see confluence
  return (int16_t)(100 * 100 * reading);
}

static void prv_periodic_ads_read(SoftTimerId id, void *context) {
  double reading = s_ads1259_storage.reading;
  int16_t val = prv_voltage_to_current(reading);
  CurrentStorage *storage = context;
  storage->readings_ring[storage->ring_idx] = val;
  storage->ring_idx = (storage->ring_idx + 1) % NUM_STORED_CURRENT_READINGS;
  ads1259_get_conversion_data(&s_ads1259_storage);
  soft_timer_start_millis(storage->conv_period_ms, prv_periodic_ads_read, context, NULL);

  // update average
  int32_t sum = 0;
  for (uint16_t i = 0; i < NUM_STORED_CURRENT_READINGS; i++) {
    sum += storage->readings_ring[i];
  }
  storage->average = sum / NUM_STORED_CURRENT_READINGS;

  // update s_is_charging
  // note that a negative value indicates the battery is charging
  s_is_charging = storage->average < 0;

  // check faults
  if (storage->average > DISCHARGE_OVERCURRENT_CA || storage->average < CHARGE_OVERCURRENT_CA) {
    fault_bps_set(EE_BPS_STATE_FAULT_CURRENT_SENSE);
  } else {
    fault_bps_clear(EE_BPS_STATE_FAULT_CURRENT_SENSE);
  }
}

bool current_sense_is_charging() {
  return s_is_charging;
}

StatusCode current_sense_init(CurrentStorage *storage, SpiSettings *settings,
                              uint32_t conv_period_ms) {
  memset(storage, 0, sizeof(CurrentStorage));
  const Ads1259Settings ads_settings = {
    .spi_port = CURRENT_SENSE_SPI_PORT,
    .spi_baudrate = settings->baudrate,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->sclk,
    .cs = settings->cs,
    .handler = prv_ads_error_cb,
    .error_context = NULL,
  };
  storage->conv_period_ms = conv_period_ms;
  status_ok_or_return(ads1259_init(&s_ads1259_storage, &ads_settings));
  ads1259_get_conversion_data(&s_ads1259_storage);
  soft_timer_start_millis(storage->conv_period_ms, prv_periodic_ads_read, storage, NULL);
  return STATUS_CODE_OK;
}
