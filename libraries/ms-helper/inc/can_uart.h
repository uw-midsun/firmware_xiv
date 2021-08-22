#pragma once
// Generic CAN HW <-> UART protocol
// Requires CAN HW, UART to be initialized.
// If can_uart_enable_auto_rx is used, requires CAN hooks to be initialized.
//
// Uses COBS encoding to frame packets - 0x00 is used as a delimiter
#include "can.h"
#include "can_hw.h"
#include "status.h"
#include "uart.h"

// Process RX'd CAN message from UART
struct CanUart;
typedef void (*CanUartRxCb)(const struct CanUart *can_uart, uint32_t id, bool extended,
                            const uint64_t *data, size_t dlc, void *context);

typedef struct CanUart {
  UartPort uart;
  CanUartRxCb rx_cb;
  void *context;
} CanUart;

// Module init: Note that the provided UART port will have its RX handler
// overwritten.
StatusCode can_uart_init(CanUart *can_uart);

// Expects an initialized module.
// Overrides CAN HW's RX handler and passes requested TX's directly to CAN HW
StatusCode can_uart_enable_passthrough(CanUart *can_uart);

// Intended to request a TX on the receiver
// i.e. from Master to Slave
StatusCode can_uart_req_slave_tx(const CanUart *can_uart, uint32_t id, bool extended,
                                 const uint64_t *data, size_t dlc);

// Enables auto-TX of sent CAN messages over UART as well.
// Overrides the CAN TX hook.
StatusCode can_uart_enable_auto_tx(const CanUart *can_uart);

// Enables auto-RX of received CAN messages over UART by the regular CAN system.
// Overrides the RX callback. Requires CAN hooks to be initialized.
StatusCode can_uart_enable_auto_rx(CanUart *can_uart);
