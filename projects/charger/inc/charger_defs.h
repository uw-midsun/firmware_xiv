#pragma once

// Definitions file for definitions shared between modules

#define CONTROL_PILOT_SEL_PIN \
  { GPIO_PORT_A, 2 }

#define CHARGER_SENSE_PIN \
  { GPIO_PORT_B, 1 }

#define RELAY_EN_PIN \
  { GPIO_PORT_B, 0 }

#define LOAD_SW_EN_PIN \
  { GPIO_PORT_B, 8 }

#define CHARGER_BATTERY_THRESHOLD 1350  // decivolts, = 135.0 volts
