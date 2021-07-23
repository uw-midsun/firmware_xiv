#include "mci_status.h"

// Track current message values
static MciStatusMessage s_message = { 0 };

void mci_status_update_mc_flags(WaveSculptorStatusInfo *message, uint8_t motor) {
    // Limit flags don't have an offset, so just copy over directly
    if(!message) return;

    // Extract flags from message and shift down to start at 0
    uint8_t limit = (message->limit_flags.raw & MCI_LIMIT_MASK) >> MCI_LIMIT_OFFSET;
    uint8_t error = (message->error_flags.raw & MCI_ERROR_MASK) >> MCI_ERROR_OFFSET;

    s_message.mc_limit_bitset[motor] = limit;
    s_message.mc_error_bitset[motor] = error;
}

void mci_status_update_fan_fault(uint8_t fault) {
    s_message.board_fault_bitset = fault;
}

void mci_status_update_over_temp(uint8_t overtemp) {
    s_message.mc_overtemp_bitset = overtemp;
}

MciStatusMessage mci_status_get_message(void) {
    return s_message;
}