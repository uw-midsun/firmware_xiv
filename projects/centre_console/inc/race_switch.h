#pragma once

// This module implements a 2-state FSM to switch between race mode and normal
// mode. In normal mode the infotainment system and fan control run, however,
// in race mode these are disabled to conserve power. Disabling and enabling
// the features is managed using the 5V regulator.

// Requires GPIO, soft timers, event queue, and interrupts to be initialized.

#include <stdint.h>

#include "fsm.h"
#include "status.h"
#include "voltage_regulator.h"

typedef enum { RACE_STATE_OFF = 0, RACE_STATE_ON, NUM_RACE_STATES } RaceState;

typedef struct RaceSwitchFsmStorage {
  Fsm race_switch_fsm;
  RaceState current_state;
  VoltageRegulatorStorage voltage_storage;
} RaceSwitchFsmStorage;

StatusCode race_switch_fsm_init(RaceSwitchFsmStorage *storage);

bool race_switch_fsm_process_event(RaceSwitchFsmStorage *storage, Event *e);

RaceState race_switch_fsm_get_current_state(RaceSwitchFsmStorage *storage);
