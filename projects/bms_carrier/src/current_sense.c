#include "current_sense.h"

#include "ads1259_adc.h"
#include "bms.h"
#include "exported_enums.h"
#include "log.h"
#include "soft_timer.h"

static CurrentReadings *s_readings;
static Ads1259Storage s_storage;
static uint16_t s_ring_idx = 0;
static uint32_t s_conv_delay_ms = 0;

static void prv_ads_error_cb(Ads1259StatusCode code, void *context) {
  if (code == ADS1259_STATUS_CODE_OUT_OF_RANGE) {
    LOG_WARN("ADS1259 ERROR: OUT OF RANGE\n");
  } else if (code == ADS1259_STATUS_CODE_CHECKSUM_FAULT) {
    LOG_WARN("ADS1259 ERROR: CHECKSUM FAULT\n");
  }
  fault_bps(EE_BPS_STATE_FAULT_CURRENT_SENSE, false);
}

void prv_update_average(void) {
  int32_t sum = 0;
  for (uint16_t i = 0; i < NUM_STORED_CURRENT_READINGS; i++) {
    sum += s_readings->readings[i];
  }
  s_readings->average = sum / NUM_STORED_CURRENT_READINGS;
}

// returns value in centiamps
int16_t prv_voltage_to_current(double reading) {
  // current = voltage * 100, see confluence
  return (int16_t)(100 * 100 * reading);
}

void prv_periodic_ads_read(SoftTimerId id, void *context) {
  double reading = s_storage.reading;
  int16_t val = prv_voltage_to_current(reading);
  CurrentReadings *reads = context;
  reads->readings[s_ring_idx] = val;
  s_ring_idx = (s_ring_idx + 1) % NUM_STORED_CURRENT_READINGS;
  ads1259_get_conversion_data(&s_storage);
  soft_timer_start_millis(s_conv_delay_ms, prv_periodic_ads_read, context, NULL);

  prv_update_average();

  // check faults
  if (s_readings->average > DISCHARGE_OVERCURRENT_CA) {
    fault_bps(EE_BPS_STATE_FAULT_CURRENT_SENSE, false);
  } else if (s_readings->average < CHARGE_OVERCURRENT_CA) {
    fault_bps(EE_BPS_STATE_FAULT_CURRENT_SENSE, false);
  } else {
    fault_bps(EE_BPS_STATE_FAULT_CURRENT_SENSE, true);
  }
}

bool current_sense_is_charging() {
  return s_readings->average < 0;
}

StatusCode current_sense_init(CurrentReadings *readings, SpiSettings *settings,
                              uint32_t conv_delay) {
  const Ads1259Settings ads_settings = {
    .spi_port = CURRENT_SENSE_SPI_PORT,
    .spi_baudrate = settings->baudrate,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->miso,
    .cs = settings->cs,
    .handler = prv_ads_error_cb,
    .error_context = NULL,
  };
  s_conv_delay_ms = conv_delay;
  s_readings = readings;
  status_ok_or_return(ads1259_init(&s_storage, &ads_settings));
  ads1259_get_conversion_data(&s_storage);
  soft_timer_start_millis(s_conv_delay_ms, prv_periodic_ads_read, readings, NULL);
  return STATUS_CODE_OK;
}
