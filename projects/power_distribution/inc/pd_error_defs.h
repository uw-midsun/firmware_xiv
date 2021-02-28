#pragma once

#include "adt7476a_fan_controller_defs.h"

// PD fault flag defs - allows transmission of 16 bits of info stored in a uint16_t

// Errors flags for PD voltage regulator
#define PD_5V_REG_ERROR 0x01 // Indicates error condition has arisen
#define PD_5V_REG_DATA  0x02 // Holds a VoltageRegulatorError enum value

// Fan Status values - indicates respective fan failure
#define FAN1_ERR FAN1_STATUS  // (0x04)
#define FAN2_ERR FAN2_STATUS  // (0x08)
#define FAN3_ERR FAN3_STATUS  // (0x10)
#define FAN4_ERR FAN4_STATUS  // (0x20)

// Fan control overtemp flags
#define FAN_OVERTEMP           0x40  // Indicates if overtemp condition triggered
#define DCDC_OVERTEMP          0x80  // DCDC over-temp condition
#define ENCLOSURE_OVERTEMP     0x100 // ENCLOSURE VENTILATION over-temp condition
#define ERR_VCC_EXCEEDED       0x200 // Indicated an overvoltage in the fan controller

// More values can be added (up to 16 possible) below as needed 