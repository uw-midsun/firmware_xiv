#pragma once
// Module to abstract the UART version of CAN away from the caller.
// Requires UART to be enabled.

#include <stdint.h>

#include "can_uart.h"
#include "generic_can.h"
#include "status.h"

typedef struct GenericCanUart {
  GenericCan base;
  CanUart *can_uart;
} GenericCanUart;

// Initialize |can_uart| to use CAN UART on |port|.
StatusCode generic_can_uart_init(GenericCanUart *can_uart, UartPort port);
