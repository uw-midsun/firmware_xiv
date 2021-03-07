#pragma once

#include <stdint.h>

#include "event_queue.h"
#include "ltc_afe.h"
#include "spi.h"
#include "status.h"

// #define NUM_AFES 3
// #define NUM_CELL_MODULES_PER_AFE 6
// #define NUM_TOTAL_CELLS (NUM_AFES * NUM_CELL_MODULES_PER_AFE)
// #define NUM_THERMISTORS (NUM_TOTAL_CELLS * 2)
#define NUM_AFES 2
#define NUM_CELL_MODULES_PER_AFE 12
#define NUM_TOTAL_CELLS (NUM_AFES * NUM_CELL_MODULES_PER_AFE)
#define NUM_THERMISTORS (NUM_AFES * 32)
#define MAX_AFE_FAULTS 5

#define AFE_SPI_PORT SPI_PORT_1
#define AFE_SPI_SS \
  { .port = GPIO_PORT_A, .pin = 4 }
#define AFE_SPI_SCK \
  { .port = GPIO_PORT_A, .pin = 5 }
#define AFE_SPI_MISO \
  { .port = GPIO_PORT_A, .pin = 6 }
#define AFE_SPI_MOSI \
  { .port = GPIO_PORT_A, .pin = 7 }

typedef struct CellSenseSettings {
  // Units are 100 uV (or DeciMilliVolts)
  uint16_t undervoltage_dmv;
  uint16_t overvoltage_dmv;
  uint16_t charge_overtemp_dmv;
  uint16_t discharge_overtemp_dmv;
} CellSenseSettings;

typedef struct AfeReadings {
  // TODO(SOFT-9): total_voltage used to be stored here as well
  uint16_t voltages[NUM_TOTAL_CELLS];
  uint16_t temps[NUM_THERMISTORS];
} AfeReadings;

typedef struct CellSenseStorage {
  LtcAfeStorage *afe;
  AfeReadings *readings;
  uint16_t num_afe_faults;
  CellSenseSettings settings;
} CellSenseStorage;

StatusCode cell_sense_init(const CellSenseSettings *settings, AfeReadings *readings,
                           LtcAfeStorage *afe);

StatusCode cell_sense_process_event(const Event *e);
