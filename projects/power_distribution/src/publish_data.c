#include "publish_data.h"

static PowerDistributionPublishConfig s_config = { 0 };

StatusCode power_distribution_publish_data_init(PowerDistributionPublishConfig config) {
  // check that the transmitter and the currents aren't null
  if (!config.transmitter || !config.currents_to_publish) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // check that each current we're to publish is valid to avoid segfaults
  for (uint16_t c = 0; c < config.num_currents_to_publish; c++) {
    if (config.currents_to_publish[c] >= NUM_CURRENTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
  }

  s_config = config;
  return STATUS_CODE_OK;
}

StatusCode power_distribution_publish_data_publish(uint16_t current_measurements[NUM_CURRENTS]) {
  if (!s_config.currents_to_publish) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // transmit each current from the config
  for (uint16_t c = 0; c < s_config.num_currents_to_publish; c++) {
    PowerDistributionCurrent current_id = s_config.currents_to_publish[c];
    status_ok_or_return(s_config.transmitter(current_id, current_measurements[current_id]));
  }

  return STATUS_CODE_OK;
}
