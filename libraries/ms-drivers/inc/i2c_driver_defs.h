#pragma once

// Common definitions for the I2C drivers (i.e. MCP23008 and PCA9539R).

#define I2C_PORT I2C_PORT_2

#define CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }
