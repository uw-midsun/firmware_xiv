#pragma once

// Pin definitions for solar, kept here to avoid circular dependencies.

#define SOLAR_UNUSED_PIN \
  { GPIO_PORT_B, 1 }

// High on one board, low on the other.
#define MPPT_COUNT_DETECTION_PIN \
  { GPIO_PORT_A, 7 }

#define MUX_SEL_PIN_0 \
  { GPIO_PORT_B, 0 }
#define MUX_SEL_PIN_1 \
  { GPIO_PORT_B, 1 }
#define MUX_SEL_PIN_2 \
  { GPIO_PORT_B, 2 }

#define RELAY_EN_PIN \
  { GPIO_PORT_B, 4 }

#define OVERTEMP_PIN \
  { GPIO_PORT_B, 5 }
#define FULL_SPEED_PIN \
  { GPIO_PORT_B, 6 }
#define FAN_FAIL_PIN \
  { GPIO_PORT_B, 7 }

#define DISCONNECTED_MUX_OUTPUT 7
