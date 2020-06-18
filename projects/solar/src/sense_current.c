#include "sense_current.h"
#include "mcp3427_adc.h"
#include "sense.h"
#include "solar_config.h"

static Mcp3427Storage s_mcp3427_storage;

static void prv_mcp3427_callback(int16_t value_ch1, int16_t value_ch2, void *context) {}

static void prv_mcp3427_fault_callback(void *context) {}

static void prv_sense_cycle_callback(void *context) {}

StatusCode sense_current_init(void) {
  // initialize the MCP3427
  status_ok_or_return(mcp3427_init(&s_mcp3427_storage, &CURRENT_MCP3427_SETTINGS));
  status_ok_or_return(mcp3427_register_callback(&s_mcp3427_storage, prv_mcp3427_callback, NULL));
  status_ok_or_return(
      mcp3427_register_fault_callback(&s_mcp3427_storage, prv_mcp3427_fault_callback, NULL));
  status_ok_or_return(mcp3427_start(&s_mcp3427_storage));

  status_ok_or_return(sense_register(prv_sense_cycle_callback, NULL));
  return STATUS_CODE_OK;
}
