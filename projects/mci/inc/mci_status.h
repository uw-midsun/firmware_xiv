#pragma once
// Status message definitions for MCI
#include <inttypes.h>
#include "wavesculptor.h"

// Limit flags -- see WS22 user manual for more info
typedef enum {
    MCI_LIMIT_OUTPUT_VOLTAGE = 0,
    MCI_LIMIT_MOTOR_CURRENT,
    MCI_LIMIT_VELOCITY,
    MCI_LIMIT_BUS_CURRENT,
    MCI_LIMIT_VOLTAGE_UPPER,
    MCI_LIMIT_VOLTAGE_LOWER,
    MCI_LIMIT_TEMPERATURE,
    NUM_MCI_LIMITS,
} MciLimit;
static_assert(NUM_MCI_LIMITS <= sizeof(uint8_t)*8, "MciLimit is too large to store in a uint8_t");

// Error flags -- see WS22 user manual for more info
typedef enum {
    MCI_ERROR_SW_OVERCURRENT = 0,
    MCI_ERROR_BUS_OVERVOLTAGE,
    MCI_ERROR_BAD_POSITION,
    MCI_ERROR_WATCHDOG,
    MCI_ERROR_CONFIG_READ,
    MCI_ERROR_UVLO,
    MCI_ERROR_OVERSPEED,
    NUM_MCI_ERRORS,
} MciError;
static_assert(NUM_MCI_ERRORS <= sizeof(uint8_t)*8, "MciError is too large to store in a uint8_t");

// Offset from 0 when broadcast from the WaveSculptor
#define MCI_LIMIT_OFFSET 0
#define MCI_ERROR_OFFSET 1

// To mask out reserved bits
#define MCI_LIMIT_MASK ((1 << NUM_MCI_LIMITS) - 1)
#define MCI_ERROR_MASK (((1 << NUM_MCI_ERRORS) - 1) << MCI_ERROR_OFFSET)

typedef struct {
    // TODO UPDATE AFTER FIXING CYCLIC IMPORTS FROM MCI BROADCAST
    uint8_t mc_limit_bitset[2]; // from each WS
    uint8_t mc_error_bitset[2]; // from each WS
    uint8_t board_fault_bitset; // from mci_fan_control
    uint8_t mc_overtemp_bitset; // from temp messages
} MciStatusMessage;
static_assert(sizeof(MciStatusMessage) <= sizeof(uint64_t), "MciStatusMessage is too large to store in a uint64_t");

// Update the MCI status with a message from the WaveSculptors.
void mci_status_update_mc_flags(WaveSculptorStatusInfo *message, uint8_t motor);

// Update the MCI status with a fan control fault bitset.
void mci_status_update_fan_fault(uint8_t fault);

// Update the MCI status with an over-temperature bitset.
void mci_status_update_over_temp(uint8_t overtemp);

// Return the current status message for broadcast.
MciStatusMessage mci_status_get_message(void);