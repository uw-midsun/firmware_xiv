#pragma once
// Module to abstract the MCP2515 version of CAN away from the caller.
// Requires the MCP2515 to be initialized.
#include "generic_can.h"
#include "mcp2515.h"
#include "status.h"

typedef struct GenericCanMcp2515 {
  GenericCan base;
  Mcp2515Storage *mcp2515;
} GenericCanMcp2515;

// Initialize |can_mcp2515| to use CAN through the MCP2515.
StatusCode generic_can_mcp2515_init(GenericCanMcp2515 *can_mcp2515,
                                    const Mcp2515Settings *settings);
