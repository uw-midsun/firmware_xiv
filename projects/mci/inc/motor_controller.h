#include "precharge_control.h"

typedef struct MotorControllerSettings {
    GpioAddress precharge_control;
    GpioAddress precharge_monitor;
} MotorControllerSettings;

typedef struct MotorControllerStorage {
    MotorControllerSettings settings;
    PrechargeState precharge_state;
} MotorControllerStorage;