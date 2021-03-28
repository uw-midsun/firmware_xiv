#pragma once

// Pin definitions for solar, kept here to avoid circular dependencies.

#define SOLAR_UNUSED_PIN \
  { GPIO_PORT_B, 1 }

// High on one board, low on the other.
#define MPPT_COUNT_DETECTION_PIN \
  { GPIO_PORT_A, 7 }

#define SOLAR_I2C1_SDA \
  { GPIO_PORT_B, 11 }
#define SOLAR_I2C1_SCL \
  { GPIO_PORT_B, 10 }
#define SOLAR_I2C2_SDA \
  { GPIO_PORT_B, 9 }
#define SOLAR_I2C2_SCL \
  { GPIO_PORT_B, 8 }

#define SOLAR_SPI2_MOSI \
  { GPIO_PORT_B, 15 }
#define SOLAR_SPI2_MISO \
  { GPIO_PORT_B, 14 }
#define SOLAR_SPI2_SCLK \
  { GPIO_PORT_B, 13 }

#define SOLAR_CAN_RX_PIN \
  { GPIO_PORT_A, 12 }
#define SOLAR_CAN_TX_PIN \
  { GPIO_PORT_A, 11 }

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
