#include "gpio.h"

#include "precharge_control.h"

typedef enum {
    MCI_CAN_EVENT_RX = 0,
    MCI_CAN_EVENT_TX,
    MCI_CAN_EVENT_FAULT,
    NUM_MCI_CAN_EVENTS
} MciCanEvent;

typedef struct MotorControllerSettings {
    // MCI Rev5: precharge_control = Pin A9
    GpioAddress precharge_control;
    // Thanks Hardware :angry:
    // MCI Rev5: precharge_control2 = Pin B1
    GpioAddress precharge_control2;
    // MCI Rev5: precharge_monitor = Pin B0
    GpioAddress precharge_monitor;
} MotorControllerSettings;

typedef struct MotorControllerStorage {
    MotorControllerSettings settings;
    PrechargeState precharge_state;
} MotorControllerStorage;