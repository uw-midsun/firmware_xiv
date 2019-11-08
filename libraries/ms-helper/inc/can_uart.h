#pragma once
// Generic CAN HW <-> UART protocol
// Requires CAN HW, UART to be initialized
//
// Uses COBS encoding to frame packets - 0x00 is used as a delimiter
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
