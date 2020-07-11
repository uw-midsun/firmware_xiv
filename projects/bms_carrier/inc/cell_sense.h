#pragma once

#include <stdint.h>

#include "event_queue.h"
#include "spi.h"
#include "status.h"
#include "ltc_afe.h"

// Requires ltc_afe to be initialized

#define NUM_AFES 3
#define NUM_CELL_MODULES_PER_AFE 6
#define NUM_THERMISTORS_PER_AFE 30
#define MAX_AFE_FAULTS 5

typedef struct CellSenseSettings {
  // Units are 100 uV (or DeciMilliVolts)
  uint16_t undervoltage_dmv;
  uint16_t overvoltage_dmv;
  uint16_t charge_overtemp_dmv;
  uint16_t discharge_overtemp_dmv;
} CellSenseSettings;

typedef struct AfeReadings {
  // TODO(SOFT-9): total_voltage used to be stored here as well
  uint16_t voltages[NUM_AFES*NUM_CELL_MODULES_PER_AFE];
  uint16_t temps[NUM_AFES*NUM_THERMISTORS_PER_AFE];
} AfeReadings;

typedef struct CellSenseStorage {
  LtcAfeStorage *afe;
  AfeReadings *readings;
  uint16_t num_afe_faults;
  CellSenseSettings settings;
} CellSenseStorage;

StatusCode cell_sense_init(const CellSenseSettings *settings, AfeReadings *readings, LtcAfeStorage *afe);

StatusCode cell_sense_process_event(const Event *e);
