#pragma once

// Publishes current measurements over CAN as generated by power_distribution_current_measurement.
// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include <stdint.h>
#include "currents.h"
#include "status.h"

// A callback which actually transmits the data over CAN.
typedef StatusCode (*PowerDistributionDataTransmitter)(PowerDistributionCurrent current_id,
                                                       uint16_t current_data);

typedef struct {
  // Called to transmit the data, should call a CAN_TRANSMIT_POWER_DISTRIBUTION_* macro.
  PowerDistributionDataTransmitter transmitter;

  // Currents in this array will be published (in order), others will be ignored.
  PowerDistributionCurrent *currents_to_publish;
  uint16_t num_currents_to_publish;  // length of preceding array
} PowerDistributionPublishConfig;

// Initialize the module with the specified config.
StatusCode power_distribution_publish_data_init(PowerDistributionPublishConfig config);

// Publish the given set of current measurements.
// This should be called from a power_distribution_current_measurement callback.
StatusCode power_distribution_publish_data_publish(
    uint16_t current_measurements[NUM_POWER_DISTRIBUTION_CURRENTS]);
