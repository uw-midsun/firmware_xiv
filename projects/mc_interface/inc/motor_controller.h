#pragma once
#include <stdbool.h>

#include "precharge_control.h"
#include "drive_fsm.h"
#include "wavesculptor.h"
#include "generic_can.h"

#define MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS 200
#define MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS 200

#define MOTOR_CAN_ID_MC_LEFT 0x03
#define MOTOR_CAN_ID_MC_RIGHT 0x04

//TODO: add defines for gpio pins

typedef enum {
  MOTOR_CONTROLLER_LEFT = 0,
  MOTOR_CONTROLLER_RIGHT,
  NUM_MOTOR_CONTROLLERS
} MotorController;

typedef struct MotorControllerSettings {
    GenericCan *motor_can;
    uint32_t motor_controller_ids[NUM_MOTOR_CONTROLLERS];
} MotorControllerSettings;

typedef struct MotorControllerStorage {
    MotorControllerSettings settings;

    uint16_t throttle;
    float vehicle_velocity[NUM_MOTOR_CONTROLLERS];
    WaveSculptorBusMeasurement bus_measurements[NUM_MOTOR_CONTROLLERS];
        //^ float bus_voltage_v and float bus_current_a

    PrechargeStates precharge_state;
    DriveState drive_state;
    bool is_drive;

    uint8_t speed_rx_bitset;
    uint8_t bus_rx_bitset;

    size_t watchdog_counter;
}