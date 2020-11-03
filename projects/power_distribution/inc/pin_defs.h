#pragma once

// Aliases of pin addresses on front and rear power distribution.

#define POWER_DISTRIBUTION_I2C_PORT I2C_PORT_2

#define POWER_DISTRIBUTION_I2C_ADDRESS_0 0x74
#define POWER_DISTRIBUTION_I2C_ADDRESS_1 0x76

#define POWER_DISTRIBUTION_I2C_SCL_PIN \
  { GPIO_PORT_B, 10 }
#define POWER_DISTRIBUTION_I2C_SDA_PIN \
  { GPIO_PORT_B, 11 }

#define POWER_DISTRIBUTION_CAN_TX_PIN \
  { GPIO_PORT_A, 12 }
#define POWER_DISTRIBUTION_CAN_RX_PIN \
  { GPIO_PORT_A, 11 }

// Front power distribution
#define FRONT_PIN_CTR_CONSL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_5 }
#define FRONT_PIN_STEERING_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_3 }
#define FRONT_PIN_PEDAL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_1 }
#define FRONT_PIN_MAIN_DISP_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_4 }
#define FRONT_PIN_LEFT_DISPLAY_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_4 }
#define FRONT_PIN_RIGHT_DISPLAY_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_6 }
#define FRONT_PIN_DVR_DISP_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_7 }
#define FRONT_PIN_REAR_DISP_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_5 }
#define FRONT_PIN_LEFT_CAMERA_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_1 }
#define FRONT_PIN_RIGHT_CAMERA_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_3 }
#define FRONT_PIN_MAIN_PI_B_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_0 }
#define FRONT_PIN_REAR_PI_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_2 }
#define FRONT_PIN_HORN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_7 }
#define FRONT_PIN_SPEAKER_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_0 }
#define FRONT_PIN_FRONT_LEFT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_4 }
#define FRONT_PIN_FRONT_RIGHT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_2 }
#define FRONT_PIN_DAYTIME_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_5 }
#define FRONT_PIN_PARKING_BRAKE_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_7 }
#define FRONT_PIN_SPARE_1_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_6 }
#define FRONT_PIN_SPARE_2_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_1 }
#define FRONT_PIN_5V_SPARE_1_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_4 }
#define FRONT_PIN_5V_SPARE_2_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_2 }

#define FRONT_PIN_SPARE_2_CTR_CONSL_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_0 }
#define FRONT_PIN_STR_PDL_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_2 }
#define FRONT_PIN_L_R_DVR_DISP_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_5 }
#define FRONT_PIN_DVR_REAR_DISP_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_6 }
#define FRONT_PIN_LEFT_RIGHT_CAM_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_0 }
#define FRONT_PIN_MAIN_REAR_PI_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_1 }
#define FRONT_PIN_FRONT_TURN_LIGHT_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_3 }
#define FRONT_PIN_5V_SPARE_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_3 }

// fan ctrl pins - must be updated by HW
#define FRONT_PIN_FAN_POT \
  { GPIO_PORT_A, 0 }
#define FRONT_PIN_SMBALERT \
  { GPIO_PORT_A, 1 }

// Rear power distribution
#define REAR_PIN_MOTOR_INTERFACE_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_4 }
#define REAR_PIN_BMS_CARRIER_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_5 }
#define REAR_PIN_SOLAR_SENSE_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_1 }
#define REAR_PIN_TELEMETRY_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_5 }
#define REAR_PIN_CHARGER_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_6 }
#define REAR_PIN_REAR_CAMERA_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_1 }
#define REAR_PIN_REAR_LEFT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_7 }
#define REAR_PIN_REAR_RIGHT_TURN_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_5 }
#define REAR_PIN_REAR_LEFT_BRAKEL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_4 }
#define REAR_PIN_REAR_RIGHT_BRAKEL_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_6 }
#define REAR_PIN_CENTER_BRAKE_LIGHT_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_4 }
#define REAR_PIN_STROBE_LIGHT_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_2 }
#define REAR_PIN_SPARE_1_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_0 }
#define REAR_PIN_SPARE_2_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_2 }
#define REAR_PIN_SPARE_3_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_4 }
#define REAR_PIN_SPARE_4_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_2 }
#define REAR_PIN_SPARE_5_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_7 }
#define REAR_PIN_SPARE_6_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_7 }
#define REAR_PIN_SPARE_7_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_0 }
#define REAR_PIN_SPARE_8_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_1 }
#define REAR_PIN_SPARE_9_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_3 }
#define REAR_PIN_SPARE_10_EN \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_3 }

#define REAR_PIN_SOLAR_TELEMETRY_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_0 }
#define REAR_PIN_REAR_BRAKE_LIGHT_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_5 }
#define REAR_PIN_STROBE_CTR_BRK_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO0_3 }
#define REAR_PIN_REAR_TURN_LIGHT_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_6 }
#define REAR_PIN_CAM_SPARE_10_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_0 }
#define REAR_PIN_SPARE_1_2_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO1_1 }
#define REAR_PIN_SPARE_3_4_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_0, .pin = PCA9539R_PIN_IO0_3 }
#define REAR_PIN_SPARE_8_9_DSEL \
  { .i2c_address = POWER_DISTRIBUTION_I2C_ADDRESS_1, .pin = PCA9539R_PIN_IO1_2 }

// fan ctrl pins - must be updated by HW
#define REAR_PIN_ENC_VENT_THERM \
  { GPIO_PORT_A, 0 }
#define REAR_PIN_DCDC_THERM \
  { GPIO_PORT_A, 1 }
#define REAR_PIN_SMBALERT \
  { GPIO_PORT_A, 2 }
