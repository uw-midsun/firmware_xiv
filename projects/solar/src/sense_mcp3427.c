#include "sense_mcp3427.h"
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

// after this many MCP3427 faults in a row, we raise a fault event
#define MAX_CONSECUTIVE_MCP3427_FAULTS 3

static uint8_t s_num_mcp3427s;
static Mcp3427Storage s_mcp3427_storages[MAX_SOLAR_MCP3427];
static DataPoint s_mcp3427_data_points[MAX_SOLAR_MCP3427];
static uint8_t s_indices[MAX_SOLAR_MCP3427];  // for contexts

static int16_t s_values[MAX_SOLAR_MCP3427];
static bool s_has_value[MAX_SOLAR_MCP3427];
static uint8_t s_consecutive_faults[MAX_SOLAR_MCP3427];

static void prv_mcp3427_callback(int16_t value_ch1, int16_t value_ch2, void *context) {
  uint8_t idx = *(uint8_t *)context;
  s_values[idx] = (SENSE_MCP3427_CHANNEL == MCP3427_CHANNEL_1) ? value_ch1 : value_ch2;
  s_has_value[idx] = true;
  s_consecutive_faults[idx] = 0;
}

static void prv_mcp3427_fault_callback(void *context) {
  uint8_t idx = *(uint8_t *)context;
  s_consecutive_faults[idx]++;
  if (s_consecutive_faults[idx] >= MAX_CONSECUTIVE_MCP3427_FAULTS) {
    DataPoint data_point = s_mcp3427_data_points[idx];
    LOG_WARN(
        "sense_mcp3427 encountered too many MCP3427 faults on data point %d, raising fault event\n",
        data_point);
    event_raise(SOLAR_FAULT_EVENT_MCP3427, data_point);
    s_consecutive_faults[idx] = 0;
  }
}

static void prv_sense_callback(void *context) {
  uint8_t idx = *(uint8_t *)context;
  if (s_has_value[idx]) {
    DataPoint data_point = s_mcp3427_data_points[idx];
    StatusCode status = data_store_set(data_point, (uint16_t)s_values[idx]);
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
    s_has_value[i] = false;
    s_consecutive_faults[i] = 0;
    s_indices[i] = i;
    s_mcp3427_data_points[i] = settings->mcp3427s[i].data_point;
    status_ok_or_return(
        mcp3427_init(&s_mcp3427_storages[i], &settings->mcp3427s[i].mcp3427_settings));
    status_ok_or_return(
        mcp3427_register_callback(&s_mcp3427_storages[i], prv_mcp3427_callback, &s_indices[i]));
    status_ok_or_return(mcp3427_register_fault_callback(&s_mcp3427_storages[i],
                                                        prv_mcp3427_fault_callback, &s_indices[i]));
    status_ok_or_return(sense_register(prv_sense_callback, &s_indices[i]));
  }

  return STATUS_CODE_OK;
}

StatusCode sense_mcp3427_start(void) {
  for (uint8_t i = 0; i < s_num_mcp3427s; i++) {
    status_ok_or_return(mcp3427_start(&s_mcp3427_storages[i]));
  }
  return STATUS_CODE_OK;
}
