#pragma once

#include "adt7476a_fan_controller_defs.h"

// See https://uwmidsun.atlassian.net/l/c/normaJUS.
// PD fault flag defs - allows transmission of 16 bits of info stored in a uint16_t

// Errors flags for PD voltage regulator
#define PD_5V_REG_ERROR (1 << 0)  // Indicates error condition has arisen
#define PD_5V_REG_DATA (1 << 1)   // Holds a VoltageRegulatorError enum value

// Fan Status values - indicates respective fan failure
#define FAN1_ERR FAN1_STATUS  // (1 << 2)
#define FAN2_ERR FAN2_STATUS  // (1 << 3)
#define FAN3_ERR FAN3_STATUS  // (1 << 4)
#define FAN4_ERR FAN4_STATUS  // (1 << 5)

// Fan control overtemp flags
#define FAN_OVERTEMP (1 << 6)        // Indicates if overtemp condition triggered
#define DCDC_OVERTEMP (1 << 7)       // DCDC over-temp condition
#define ENCLOSURE_OVERTEMP (1 << 8)  // ENCLOSURE VENTILATION over-temp condition
#define ERR_VCC_EXCEEDED (1 << 9)    // Indicated an overvoltage in the fan controller

// BTS7200/BTS7040 output fault flags, set on overtemperature/overvoltage on a load switch output
#define BTS7200_FAULT (1 << 10)
#define BTS7040_FAULT (1 << 11)

// More values can be added (up to 16 possible) below as needed
