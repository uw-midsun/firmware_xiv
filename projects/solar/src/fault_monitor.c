#include "fault_monitor.h"

#include <stdbool.h>

#include "data_store.h"
#include "event_queue.h"
#include "solar_events.h"
#include "status.h"

#define RAISE_FAULT_EVENT(id, data) event_raise_priority(FAULT_EVENT_PRIORITY, (id), (data))

static FaultMonitorSettings s_settings;

static void prv_check_output_current(void) {
  bool is_set;
  data_store_get_is_set(DATA_POINT_CURRENT, &is_set);
  if (is_set) {
    int32_t value;
    data_store_get(DATA_POINT_CURRENT, (uint32_t *)&value);
    if (value >= s_settings.output_overcurrent_threshold_uA) {
      RAISE_FAULT_EVENT(SOLAR_FAULT_EVENT_OVERCURRENT, 0);
    }
    if (value < 0) {
      RAISE_FAULT_EVENT(SOLAR_FAULT_EVENT_NEGATIVE_CURRENT, 0);
    }
  }
}

static void prv_check_temperature(uint8_t thermistor) {
  bool is_set;
  data_store_get_is_set(DATA_POINT_TEMPERATURE(thermistor), &is_set);
  if (is_set) {
    uint32_t value;
    data_store_get(DATA_POINT_TEMPERATURE(thermistor), &value);
    if (value >= s_settings.overtemperature_threshold_dC) {
      RAISE_FAULT_EVENT(SOLAR_FAULT_EVENT_OVERTEMPERATURE, thermistor);
    }
  }
}

static void prv_check_output_voltage_sum(void) {
  uint32_t total = 0;
  for (Mppt mppt = 0; mppt < s_settings.mppt_count; mppt++) {
    bool is_set;
    data_store_get_is_set(DATA_POINT_VOLTAGE(mppt), &is_set);
    if (is_set) {
      uint32_t value;
      data_store_get(DATA_POINT_VOLTAGE(mppt), &value);
      // equivalent to value + total >= threshold, but avoids possible overflow if value is large
      if (s_settings.output_overvoltage_threshold_mV - total <= value) {
        RAISE_FAULT_EVENT(SOLAR_FAULT_EVENT_OVERVOLTAGE, 0);
        return;
      }
      total += value;
    }
  }
}

static void prv_check_faults(void) {
  prv_check_output_current();
  for (Mppt mppt = 0; mppt < s_settings.mppt_count; mppt++) {
    prv_check_temperature(mppt);
  }
  prv_check_output_voltage_sum();
}

StatusCode fault_monitor_init(FaultMonitorSettings *settings) {
  if (settings == NULL || settings->mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_settings = *settings;
  return STATUS_CODE_OK;
}

bool fault_monitor_process_event(Event *e) {
  if (e != NULL && e->id == DATA_READY_EVENT) {
    prv_check_faults();
    return true;
  }
  return false;
}
