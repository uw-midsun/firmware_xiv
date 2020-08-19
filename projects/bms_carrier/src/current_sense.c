#include "current_sense.h"

#include "ads1259_adc.h"
#include "bms.h"
#include "exported_enums.h"
#include "soft_timer.h"

// slightly larger than conversion time of adc
#define CONVERSION_TIME_MS 18
// placeholder - 100 amps
#define OVERCURRENT_MA (100 * 1000)

static CurrentReadings *s_readings;
static Ads1259Storage s_storage;
static uint16_t s_ring_idx = 0;

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

int16_t prv_voltage_to_current(double reading) {
  // placeholder - to be filled in after calibration
  return 0;
}

void prv_periodic_ads_read(SoftTimerId id, void *context) {
  ads1259_get_conversion_data(&s_storage);
  double reading = s_storage.reading;
  int16_t val = prv_voltage_to_current(reading);
  CurrentReadings *readings = context;
  readings->readings[s_ring_idx];
  s_ring_idx = (s_ring_idx + 1) % NUM_STORED_CURRENT_READINGS;
  soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_ads_read, context, NULL);
  
  prv_update_average();

  // check faults
  if (s_readings->average > OVERCURRENT_MA) {
    fault_bps(EE_BPS_STATE_FAULT_CURRENT_SENSE, false);
  } else {
    fault_bps(EE_BPS_STATE_FAULT_CURRENT_SENSE, true);
  }
}

bool current_sense_is_charging() {
  return s_readings->average > 0;
}

StatusCode current_sense_init(CurrentReadings *readings, SpiSettings *settings) {
  const Ads1259Settings settings = {
    .spi_port = CURRENT_SENSE_SPI_PORT,
    .spi_baudrate = settings->baudrate,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->miso,
    .cs = settings->cs,
    .handler = prv_ads_error_cb,
    .error_context = NULL,
  };
  s_readings = readings;
  status_ok_or_return(ads1259_init(&s_storage, &settings));
  prv_periodic_ads_read(SOFT_TIMER_INVALID_TIMER, NULL);
  return STATUS_CODE_OK;
}
