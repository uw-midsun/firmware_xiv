#include "logger.h"

#include <stdio.h>

#include "data_store.h"
#include "event_queue.h"
#include "log.h"
#include "solar_boards.h"
#include "solar_events.h"
#include "status.h"

static SolarMpptCount s_mppt_count;

// the order in which the data will be displayed
static DataPointType s_data_point_type_order[] = {
  DATA_POINT_TYPE_MPPT_VOLTAGE,  //
  DATA_POINT_TYPE_VOLTAGE,       //
  DATA_POINT_TYPE_MPPT_CURRENT,  //
  DATA_POINT_TYPE_CURRENT,       //
  DATA_POINT_TYPE_TEMPERATURE,   //
  DATA_POINT_TYPE_MPPT_PWM,      //
  DATA_POINT_TYPE_MPPT_STATUS,   //
};

static char *s_data_point_types_to_key_format[] = {
  [DATA_POINT_TYPE_MPPT_VOLTAGE] = "MPPT %ld input voltage",     //
  [DATA_POINT_TYPE_VOLTAGE] = "MPPT %ld output voltage",         //
  [DATA_POINT_TYPE_MPPT_CURRENT] = "MPPT %ld input current",     //
  [DATA_POINT_TYPE_CURRENT] = "Total output current",            //
  [DATA_POINT_TYPE_TEMPERATURE] = "Thermistor %ld temperature",  //
  [DATA_POINT_TYPE_MPPT_PWM] = "MPPT %ld PWM duty cycle",        //
  [DATA_POINT_TYPE_MPPT_STATUS] = "MPPT %ld status",             //
};

static char *s_data_point_types_to_value_format[] = {
  [DATA_POINT_TYPE_MPPT_VOLTAGE] = "%lu mV",  //
  [DATA_POINT_TYPE_VOLTAGE] = "%lu mV",       //
  [DATA_POINT_TYPE_MPPT_CURRENT] = "%lu uA",  //
  [DATA_POINT_TYPE_CURRENT] = "%ld uA",       //
  [DATA_POINT_TYPE_TEMPERATURE] = "%lu dC",   //
  [DATA_POINT_TYPE_MPPT_PWM] = "%lu/1000",    //
  [DATA_POINT_TYPE_MPPT_STATUS] = "0x%lx",    //
};

static bool s_data_point_types_to_signed[] = {
  [DATA_POINT_TYPE_MPPT_VOLTAGE] = false,  //
  [DATA_POINT_TYPE_VOLTAGE] = false,       //
  [DATA_POINT_TYPE_MPPT_CURRENT] = false,  //
  [DATA_POINT_TYPE_CURRENT] = true,        //
  [DATA_POINT_TYPE_TEMPERATURE] = false,   //
  [DATA_POINT_TYPE_MPPT_PWM] = false,      //
  [DATA_POINT_TYPE_MPPT_STATUS] = false,   //
};

static void prv_log_data_point(DataPointType type, Mppt mppt, DataPoint data_point) {
  char key[32], value[32] = "unset";
  snprintf(key, SIZEOF_ARRAY(key), s_data_point_types_to_key_format[type], mppt);

  bool is_set;
  data_store_get_is_set(data_point, &is_set);

  if (is_set) {
    uint32_t data_value;
    data_store_get(data_point, &data_value);

    // casting to long int is to resolve inconsistent int sizes between STM32 and x86
    if (s_data_point_types_to_signed[type]) {
      int32_t signed_value = (int32_t)data_value;
      snprintf(value, SIZEOF_ARRAY(value), s_data_point_types_to_value_format[type],
               (long int)signed_value);  // NOLINT(runtime/int)
    } else {
      snprintf(value, SIZEOF_ARRAY(value), s_data_point_types_to_value_format[type],
               (long unsigned int)data_value);  // NOLINT(runtime/int)
    }
  }

  LOG_DEBUG("%s: %s\n", key, value);
}

static void prv_log_data(void) {
  for (uint16_t type_idx = 0; type_idx < NUM_DATA_POINT_TYPES; type_idx++) {
    DataPointType type = s_data_point_type_order[type_idx];
    if (type == DATA_POINT_TYPE_CURRENT) {
      // special case: only 1 current data point
      prv_log_data_point(type, 0, DATA_POINT_CURRENT);
    } else {
      for (Mppt mppt = 0; mppt < s_mppt_count; mppt++) {
        prv_log_data_point(type, mppt, NTH_DATA_POINT(type, mppt));
      }
    }
  }
}

StatusCode logger_init(SolarMpptCount mppt_count) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_mppt_count = mppt_count;
  return STATUS_CODE_OK;
}

bool logger_process_event(Event *e) {
  if (e != NULL && e->id == DATA_READY_EVENT) {
    prv_log_data();
    return true;
  }
  return false;
}
