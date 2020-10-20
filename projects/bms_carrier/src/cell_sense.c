#include "cell_sense.h"

#include <string.h>

#include "bms.h"
#include "bms_events.h"
#include "critical_section.h"
#include "current_sense.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "ltc_afe.h"
#include "passive_balance.h"
#include "status.h"
#include "thermistor.h"

static CellSenseStorage s_storage = { 0 };

static void prv_extract_cell_result(uint16_t *result_arr, size_t len, void *context) {
  ltc_afe_request_aux_conversion(s_storage.afe);

  bool disabled = critical_section_start();
  memcpy(s_storage.readings->voltages, result_arr, sizeof(s_storage.readings->voltages));
  critical_section_end(disabled);

  bool fault = false;
  for (size_t i = 0; i < len; i++) {
    // s_storage.total_voltage += s_storage.readings->voltages[i];
    if (s_storage.readings->voltages[i] < s_storage.settings.undervoltage_dmv ||
        s_storage.readings->voltages[i] > s_storage.settings.overvoltage_dmv) {
      fault = true;
    }
  }

  // Balance cells if needed
  passive_balance(s_storage.readings->voltages, len, s_storage.afe);

  if (fault) {
    fault_bps_set(EE_BPS_STATE_FAULT_AFE_CELL);
  } else {
    fault_bps_clear(EE_BPS_STATE_FAULT_AFE_CELL);
  }
}

static void prv_extract_aux_result(uint16_t *result_arr, size_t len, void *context) {
  ltc_afe_request_cell_conversion(s_storage.afe);

  bool disabled = critical_section_start();
  memcpy(s_storage.readings->temps, result_arr, sizeof(s_storage.readings->temps));
  critical_section_end(disabled);

  uint16_t threshold = s_storage.settings.discharge_overtemp_dmv;
  if (current_sense_is_charging()) threshold = s_storage.settings.charge_overtemp_dmv;

  for (size_t i = 0; i < len; ++i) {
    if (s_storage.readings->temps[i] > threshold) {
      fault_bps_set(EE_BPS_STATE_FAULT_AFE_TEMP);
      return;
    }
  }

  fault_bps_clear(EE_BPS_STATE_FAULT_AFE_TEMP);
}

StatusCode cell_sense_init(const CellSenseSettings *settings, AfeReadings *afe_readings,
                           LtcAfeStorage *afe) {
  LtcAfeSettings afe_settings = {
    // Settings pending hardware validation
    .cs = AFE_SPI_SS,
    .mosi = AFE_SPI_MOSI,
    .miso = AFE_SPI_MISO,
    .sclk = AFE_SPI_SCK,

    .spi_port = AFE_SPI_PORT,
    .spi_baudrate = 750000,

    .adc_mode = LTC_AFE_ADC_MODE_7KHZ,

    .cell_bitset = { 0 },
    .aux_bitset = { 0 },

    .num_devices = NUM_AFES,
    .num_cells = NUM_CELL_MODULES_PER_AFE,
    .num_thermistors = NUM_CELL_MODULES_PER_AFE,

    .ltc_events = { 
      .trigger_cell_conv_event = BMS_AFE_EVENT_TRIGGER_CELL_CONV,    //
      .cell_conv_complete_event = BMS_AFE_EVENT_CELL_CONV_COMPLETE,  //
      .trigger_aux_conv_event = BMS_AFE_EVENT_TRIGGER_AUX_CONV,      //
      .aux_conv_complete_event = BMS_AFE_EVENT_AUX_CONV_COMPLETE,    //
      .callback_run_event = BMS_AFE_EVENT_CALLBACK_RUN,              //
      .fault_event = BMS_AFE_EVENT_FAULT                             //
    },

    .cell_result_cb = prv_extract_cell_result,
    .aux_result_cb = prv_extract_aux_result,
  };
  ltc_afe_init(afe, &afe_settings);
  s_storage.afe = afe;
  s_storage.readings = afe_readings;
  memset(afe_readings, 0, sizeof(AfeReadings));
  memcpy(&s_storage.settings, settings, sizeof(CellSenseSettings));
  ltc_afe_set_result_cbs(afe, prv_extract_cell_result, prv_extract_aux_result, NULL);
  return ltc_afe_request_cell_conversion(afe);
}

StatusCode cell_sense_process_event(const Event *e) {
  switch (e->id) {
    case BMS_AFE_EVENT_FAULT:
      if (s_storage.num_afe_faults > MAX_AFE_FAULTS) {
        fault_bps_set(EE_BPS_STATE_FAULT_AFE_FSM);
      } else {
        s_storage.num_afe_faults++;
      }
      ltc_afe_request_cell_conversion(s_storage.afe);
      break;

    case BMS_AFE_EVENT_CALLBACK_RUN:
      s_storage.num_afe_faults = 0;
      fault_bps_clear(EE_BPS_STATE_FAULT_AFE_FSM);
      break;

    default:
      return status_code(STATUS_CODE_EMPTY);
  }

  return STATUS_CODE_OK;
}
