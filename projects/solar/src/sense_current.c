#include "sense_current.h"
#include "data_store.h"
#include "log.h"
#include "mcp3427_adc.h"
#include "sense.h"
#include "solar_config.h"

// we only use one channel on the MCP3427
#define CURRENT_MCP3427_CHANNEL MCP3427_CHANNEL_1

static Mcp3427Storage s_mcp3427_storage;
static int16_t s_value;
static bool s_has_value;  // to prevent sending uninitialized garbage to data_store

static void prv_mcp3427_callback(int16_t value_ch1, int16_t value_ch2, void *context) {
  s_value = (CURRENT_MCP3427_CHANNEL == MCP3427_CHANNEL_1) ? value_ch1 : value_ch2;
  s_has_value = true;
}

static void prv_mcp3427_fault_callback(void *context) {
  LOG_WARN("sense_current encountered MCP3427 fault\n");
}

static void prv_sense_cycle_callback(void *context) {
  if (s_has_value) {
    StatusCode status = data_store_set(DATA_POINT_CURRENT, (uint16_t)s_value);
    if (!status_ok(status)) {
      LOG_WARN("sense_current could not data_store_set with DATA_POINT_CURRENT\n");
    }
  }
}

StatusCode sense_current_init(SenseCurrentSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // initialize the MCP3427
  status_ok_or_return(mcp3427_init(&s_mcp3427_storage, &settings->current_mcp3427_settings));
  status_ok_or_return(mcp3427_register_callback(&s_mcp3427_storage, prv_mcp3427_callback, NULL));
  status_ok_or_return(
      mcp3427_register_fault_callback(&s_mcp3427_storage, prv_mcp3427_fault_callback, NULL));
  status_ok_or_return(mcp3427_start(&s_mcp3427_storage));

  status_ok_or_return(sense_register(prv_sense_cycle_callback, NULL));
  s_has_value = false;
  return STATUS_CODE_OK;
}
