#pragma once

#include "controller_board_pins.h"

// Aliases of pin addresses on front and rear power distribution.

#define PD_I2C_PORT I2C_PORT_1

#define PD_PCA9539R_I2C_ADDRESS_0 0x74
#define PD_PCA9539R_I2C_ADDRESS_1 0x76

#define PD_I2C_SCL_PIN CONTROLLER_BOARD_ADDR_I2C1_SCL
#define PD_I2C_SDA_PIN CONTROLLER_BOARD_ADDR_I2C1_SDA

#define PD_CAN_TX_PIN CONTROLLER_BOARD_ADDR_CAN_TX
#define PD_CAN_RX_PIN CONTROLLER_BOARD_ADDR_CAN_RX

#define PD_5V_REG_MONITOR_PIN \
  { GPIO_PORT_B, 1 }
#define PD_5V_REG_ENABLE \
  { GPIO_PORT_B, 2 }

// used in output config for a hack to specify an output with only current sense enabled
#define NONEXISTENT_PCA9539R_PIN \
  { 0xff, PCA9539R_PIN_IO0_0 }

// Front power distribution
#define FRONT_PIN_CENTRE_CONSOLE_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_4 }
#define FRONT_PIN_STEERING_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_2 }
#define FRONT_PIN_PEDAL_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_4 }
#define FRONT_PIN_SPEAKER_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_5 }
#define FRONT_PIN_MAIN_PI_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_0 }  // driver display + telemetry pi
#define FRONT_PIN_DRIVER_DISPLAY_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_2 }
#define FRONT_PIN_INFOTAINMENT_DISPLAY_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_0 }  // aka main display
#define FRONT_PIN_LEFT_CAMERA_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_6 }
#define FRONT_PIN_RIGHT_CAMERA_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_7 }
#define FRONT_PIN_LEFT_DISPLAY_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_5 }
#define FRONT_PIN_RIGHT_DISPLAY_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_7 }
#define FRONT_PIN_REAR_DISPLAY_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_6 }
#define FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_0 }
#define FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_2 }
#define FRONT_PIN_DAYTIME_RUNNING_LIGHTS_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_3 }
#define FRONT_PIN_5V_SPARE_1_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_3 }
#define FRONT_PIN_5V_SPARE_2_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_5 }
#define FRONT_PIN_SPARE_1_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_1 }
#define FRONT_PIN_SPARE_2_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_1 }
#define FRONT_PIN_SPARE_3_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_0 }
#define FRONT_PIN_SPARE_4_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_6 }  // on MCI's BTS7040
#define FRONT_PIN_SPARE_5_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_7 }  // on rear fan 1's BTS7200 channel
#define FRONT_PIN_SPARE_6_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_3 }  // on rear fan 2's BTS7200 channel

// note these are on GPIO rather than PCA9539R since they're on UV cutoff
#define FRONT_PIN_HORN_EN \
  { GPIO_PORT_B, 11 }
#define FRONT_PIN_FAN_EN \
  { GPIO_PORT_B, 12 }

#define FRONT_PIN_CENTRE_CONSOLE_REAR_DISPLAY_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_5 }
#define FRONT_PIN_PEDAL_STEERING_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_3 }
#define FRONT_PIN_MAIN_PI_DRIVER_DISPLAY_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_1 }
#define FRONT_PIN_LEFT_RIGHT_CAMERA_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_7 }
#define FRONT_PIN_LEFT_RIGHT_DISPLAY_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_2 }
#define FRONT_PIN_FRONT_LEFT_RIGHT_TURN_LIGHT_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_1 }
#define FRONT_PIN_5V_SPARE_1_2_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_4 }
#define FRONT_PIN_SPARE_2_3_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_4 }
#define FRONT_PIN_SPARE_5_6_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_6 }

#define FRONT_UV_COMPARATOR_PIN \
  { GPIO_PORT_B, 0 }

#define FRONT_MUX_SEL_CENTRE_CONSOLE_REAR_DISPLAY 0
#define FRONT_MUX_SEL_MAIN_PI_DRIVER_DISPLAY 1
#define FRONT_MUX_SEL_5V_SPARES 2
#define FRONT_MUX_SEL_SPARE_4 3  // on MCI's BTS7040
#define FRONT_MUX_SEL_LEFT_RIGHT_DISPLAY 4
#define FRONT_MUX_SEL_SPEAKER 5
#define FRONT_MUX_SEL_PEDAL_STEERING 6
#define FRONT_MUX_SEL_SPARE_2_3 7
#define FRONT_MUX_SEL_DAYTIME_RUNNING_LIGHTS 8
#define FRONT_MUX_SEL_FRONT_LEFT_RIGHT_TURN_LIGHT 9
#define FRONT_MUX_SEL_SPARE_1 11
#define FRONT_MUX_SEL_LEFT_RIGHT_CAMERA 12
#define FRONT_MUX_SEL_INFOTAINMENT_DISPLAY 13  // aka main display
#define FRONT_MUX_SEL_SPARE_5_6 14             // on rear fan 1 & 2's BTS7200
#define FRONT_MUX_SEL_UV_VBAT 15

// Rear power distribution
#define REAR_PIN_MCI_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_6 }
#define REAR_PIN_BMS_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_0 }
#define REAR_PIN_CHARGER_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_5 }
#define REAR_PIN_SOLAR_SENSE_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_5 }
#define REAR_PIN_REAR_CAMERA_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_6 }
#define REAR_PIN_FAN_1_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_7 }
#define REAR_PIN_FAN_2_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_3 }
#define REAR_PIN_STROBE_LIGHT_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_7 }
#define REAR_PIN_REAR_LEFT_TURN_LIGHT_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_0 }
#define REAR_PIN_REAR_RIGHT_TURN_LIGHT_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_2 }
#define REAR_PIN_BRAKE_LIGHT_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_3 }
#define REAR_PIN_5V_SPARE_1_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_3 }
#define REAR_PIN_5V_SPARE_2_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_5 }
#define REAR_PIN_SPARE_1_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_1 }
#define REAR_PIN_SPARE_2_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_1 }
#define REAR_PIN_SPARE_3_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_0 }
#define REAR_PIN_SPARE_4_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_4 }  // pedal BTS7200 channel
#define REAR_PIN_SPARE_5_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_2 }  // steering BTS7200 channel
#define REAR_PIN_SPARE_6_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_7 }  // right camera BTS7200 channel
#define REAR_PIN_SPARE_7_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_0 }  // main pi BTS7200 channel
#define REAR_PIN_SPARE_8_EN \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_2 }  // driver display BTS7200 channel
#define REAR_PIN_SPARE_9_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_4 }  // centre console BTS7200 channel
#define REAR_PIN_SPARE_10_EN \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_6 }  // rear display BTS7200 channel

#define REAR_PIN_CHARGER_STROBE_LIGHT_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_2 }
#define REAR_PIN_REAR_LEFT_RIGHT_TURN_LIGHT_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_1 }
#define REAR_PIN_REAR_CAMERA_SPARE_6_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_7 }
#define REAR_PIN_FAN_1_2_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_6 }
#define REAR_PIN_5V_SPARE_1_2_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_4 }
#define REAR_PIN_SPARE_2_3_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO0_4 }
#define REAR_PIN_SPARE_4_5_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO0_3 }  // pedal/steering BTS7200
#define REAR_PIN_SPARE_7_8_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_0, PCA9539R_PIN_IO1_1 }  // main pi/driver display BTS7200
#define REAR_PIN_SPARE_9_10_DSEL \
  { PD_PCA9539R_I2C_ADDRESS_1, PCA9539R_PIN_IO1_5 }  // centre console/rear display BTS7200

#define REAR_MUX_SEL_SPARE_9_10 0
#define REAR_MUX_SEL_SPARE_7_8 1  // on main pi/driver display BTS7200
#define REAR_MUX_SEL_5V_SPARES 2
#define REAR_MUX_SEL_MCI 3
#define REAR_MUX_SEL_CHARGER_STROBE_LIGHT 4
#define REAR_MUX_SEL_SOLAR_SENSE 5
#define REAR_MUX_SEL_SPARE_4_5 6  // on steering/pedal BTS7200
#define REAR_MUX_SEL_SPARE_2_3 7
#define REAR_MUX_SEL_BRAKE_LIGHT 8
#define REAR_MUX_SEL_REAR_LEFT_RIGHT_TURN_LIGHT 9
#define REAR_MUX_SEL_SPARE_1 11
#define REAR_MUX_SEL_REAR_CAMERA_SPARE_6 12
#define REAR_MUX_SEL_BMS 13
#define REAR_MUX_SEL_FAN_1_2 14

// Mux pins
#define PD_MUX_ENABLE_PIN \
  { GPIO_PORT_A, 2 }
#define PD_MUX_OUTPUT_PIN \
  { GPIO_PORT_A, 7 }
#define PD_MUX_SEL1_PIN \
  { GPIO_PORT_A, 6 }
#define PD_MUX_SEL2_PIN \
  { GPIO_PORT_A, 5 }
#define PD_MUX_SEL3_PIN \
  { GPIO_PORT_A, 4 }
#define PD_MUX_SEL4_PIN \
  { GPIO_PORT_A, 3 }

// Fan control
#define PD_SMBALERT_PIN \
  { GPIO_PORT_B, 10 }

#define FRONT_PIN_FAN_POT \
  { GPIO_PORT_A, 0 }

#define REAR_PIN_ENC_VENT_THERM \
  { GPIO_PORT_A, 0 }
#define REAR_PIN_DCDC_THERM \
  { GPIO_PORT_A, 1 }

#define FRONT_OR_REAR_RECOGNITION_PIN \
  { GPIO_PORT_A, 8 }
