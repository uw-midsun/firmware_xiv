#pragma once

// Module for opening and closing the relays
// Requires interrupts, soft timers, CAN, and the MCP23008 to be initialized

#include <stdbool.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#define BMS_HV_RELAY_EN_PIN \
  { GPIO_PORT_A, 0 }
#define BMS_GND_RELAY_EN_PIN \
  { GPIO_PORT_A, 1 }
#define BMS_IO_EXPANDER_INT_PIN \
  { GPIO_PORT_A, 10 }

#define BMS_IO_EXPANDER_HV_SENSE_ADDR \
  { BMS_IO_EXPANDER_I2C_ADDR, 1 }
#define BMS_IO_EXPANDER_GND_SENSE_ADDR \
  { BMS_IO_EXPANDER_I2C_ADDR, 0 }

// delays given by datasheet (EV200HAANA)
// max between time to close and time to open
#define RELAY_SEQUENCE_ASSERTION_DELAY_MS 15
// max inrush time
#define RELAY_SEQUENCE_NEXT_STEP_DELAY_MS 130

typedef struct RelayStorage {
  bool hv_enabled;
  bool gnd_enabled;
  bool hv_expected_state;
  bool gnd_expected_state;
  SoftTimerId assertion_timer_id;
  SoftTimerId next_step_timer_id;
} RelayStorage;

StatusCode relay_sequence_init(RelayStorage *storage);

StatusCode relay_open_sequence(RelayStorage *storage);

StatusCode relay_close_sequence(RelayStorage *storage);

StatusCode relay_fault(RelayStorage *storage);
