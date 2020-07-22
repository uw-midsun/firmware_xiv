#include "sense_mcp3427.h"

#include <stdbool.h>
#include <stdint.h>

#include "data_store.h"
#include "event_queue.h"
#include "log.h"
#include "mcp3427_adc.h"
#include "sense.h"
#include "solar_events.h"

// We register one sense callback per MCP3427 to take advantage of the sense loop, and so that the
// entire MCP3427 sense operation doesn't stop if there is one fault in the sense cycle.

// we only use one channel on the MCP3427
#define SENSE_MCP3427_CHANNEL MCP3427_CHANNEL_1

// holds all the data for each MCP3427, so we don't have to pass around an array index
typedef struct SenseMcp3427Data {
  Mcp3427Storage mcp3427_storage;
  DataPoint mcp3427_data_point;

  int16_t value;
  bool has_value;
  uint8_t consecutive_faults;
} SenseMcp3427Data;

static SenseMcp3427Data s_mcp3427_data[MAX_SOLAR_MCP3427];
static uint8_t s_num_mcp3427s;

static void prv_mcp3427_callback(int16_t value_ch1, int16_t value_ch2, void *context) {
  SenseMcp3427Data *data = context;
  data->value = (SENSE_MCP3427_CHANNEL == MCP3427_CHANNEL_1) ? value_ch1 : value_ch2;
  data->has_value = true;
  data->consecutive_faults = 0;
}

static void prv_mcp3427_fault_callback(void *context) {
  SenseMcp3427Data *data = context;
  data->consecutive_faults++;
  if (data->consecutive_faults >= MAX_CONSECUTIVE_MCP3427_FAULTS) {
    DataPoint data_point = data->mcp3427_data_point;
    LOG_WARN(
        "sense_mcp3427 encountered too many MCP3427 faults on data point %d, raising fault event\n",
        data_point);
    event_raise(SOLAR_FAULT_EVENT_MCP3427, data_point);
    data->consecutive_faults = 0;
  }
}

static void prv_sense_callback(void *context) {
  SenseMcp3427Data *data = context;
  if (data->has_value) {
    DataPoint data_point = data->mcp3427_data_point;
    // we convert the int16_t reported by the MCP3427 directly to a uint32_t for the data store
    StatusCode status = data_store_set(data_point, (uint32_t)data->value);
    if (!status_ok(status)) {
      LOG_WARN("sense_mcp3427 could not data_store_set with data point %d\n", data_point);
    }
  }
}

StatusCode sense_mcp3427_init(SenseMcp3427Settings *settings) {
  if (settings == NULL || settings->num_mcp3427s > MAX_SOLAR_MCP3427) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_num_mcp3427s = settings->num_mcp3427s;

  for (uint8_t i = 0; i < s_num_mcp3427s; i++) {
    SenseMcp3427Data *data = &s_mcp3427_data[i];
    data->has_value = false;
    data->consecutive_faults = 0;
    data->mcp3427_data_point = settings->mcp3427s[i].data_point;
    status_ok_or_return(
        mcp3427_init(&data->mcp3427_storage, &settings->mcp3427s[i].mcp3427_settings));
    status_ok_or_return(
        mcp3427_register_callback(&data->mcp3427_storage, prv_mcp3427_callback, data));
    status_ok_or_return(
        mcp3427_register_fault_callback(&data->mcp3427_storage, prv_mcp3427_fault_callback, data));
    status_ok_or_return(sense_register(prv_sense_callback, data));
  }

  return STATUS_CODE_OK;
}

StatusCode sense_mcp3427_start(void) {
  for (uint8_t i = 0; i < s_num_mcp3427s; i++) {
    status_ok_or_return(mcp3427_start(&s_mcp3427_data[i].mcp3427_storage));
  }
  return STATUS_CODE_OK;
}
