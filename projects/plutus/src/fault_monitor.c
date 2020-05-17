#include "fault_monitor.h"

#include <string.h>

#include "critical_section.h"
#include "exported_enums.h"
#include "log.h"
#include "plutus_event.h"
#include "thermistor.h"

static void prv_extract_cell_result(uint16_t *result_arr, size_t len, void *context) {
  FaultMonitorStorage *storage = context;

  // read voltages
  ltc_afe_request_aux_conversion(storage->settings.ltc_afe);

  // Extract results
  bool disabled = critical_section_start();
  memcpy(storage->result.cell_voltages, result_arr, sizeof(storage->result.cell_voltages));
  critical_section_end(disabled);

  // output results
  bool fault = false;
  for (size_t i = 0; i < len; i++) {
    LOG_DEBUG("CELL_VOLTAGE #%zu = %u", i, result_arr[i]);
  }
}

static void prv_extract_aux_result(uint16_t *result_arr, size_t len, void *context) {
  FaultMonitorStorage *storage = context;

  // read temperatures
  ltc_afe_request_cell_conversion(storage->settings.ltc_afe);

  bool disabled = critical_section_start();
  memcpy(storage->result.temp_voltages, result_arr, sizeof(storage->result.temp_voltages));
  critical_section_end(disabled);

  // output results
  bool fault = false;
  for (size_t i = 0; i < len; i++) {
    LOG_DEBUG("CELL_TEMP #%zu = %u", i, result_arr[i]);
  }
}

static void prv_extract_current(int32_t value, void *context) {
  FaultMonitorStorage *storage = context;

  storage->result.current = value;
  LOG_DEBUG("CELL_CURRENT = %ld", value);
}

static void prv_handle_adc_timeout(void *context) {}

StatusCode fault_monitor_init(FaultMonitorStorage *storage, const FaultMonitorSettings *settings) {
  storage->settings = *settings;
  storage->num_afe_faults = 0;
  // Convert mA to uA
  storage->charge_current_limit = settings->overcurrent_charge * 1000;
  storage->discharge_current_limit = settings->overcurrent_discharge * -1000;
  storage->min_charge_current = -1 * settings->charge_current_deadzone;

  current_sense_register_callback(storage->settings.current_sense, prv_extract_current,
                                  prv_handle_adc_timeout, storage);

  ltc_afe_set_result_cbs(storage->settings.ltc_afe, prv_extract_cell_result, prv_extract_aux_result,
                         storage);

  return ltc_afe_request_cell_conversion(storage->settings.ltc_afe);
}

bool fault_monitor_process_event(FaultMonitorStorage *storage, const Event *e) {
  ltc_afe_request_cell_conversion(storage->settings.ltc_afe);
  return true;
  }
