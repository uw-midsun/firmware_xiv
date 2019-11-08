#pragma once
// Module for abstracting CAN implementation details.

#include <stdbool.h>
#include <stdint.h>

#include "generic_can_msg.h"
#include "status.h"

#define GENERIC_CAN_EMPTY_MASK UINT32_MAX
#define NUM_GENERIC_CAN_RX_HANDLERS 10

struct GenericCan;

typedef void (*GenericCanRx)(const GenericCanMsg *msg, void *context);

typedef struct GenericCanRxStorage {
  uint32_t filter;
  uint32_t mask;
  GenericCanRx rx_handler;
  void *context;
} GenericCanRxStorage;

typedef struct GenericCanInterface {
  StatusCode (*tx)(const struct GenericCan *can, const GenericCanMsg *msg);
  // Doesn't support responding with errors to ACKable messages (defaults to
  // OK).
  StatusCode (*register_rx)(struct GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                            uint32_t filter, bool extended, void *context);
} GenericCanInterface;

typedef struct GenericCan {
  GenericCanInterface *interface;
  GenericCanRxStorage rx_storage[NUM_GENERIC_CAN_RX_HANDLERS];
} GenericCan;

// Usage:
//
// GenericCan uses dynamic dispatch where each struct for an implementation
// variant of CAN carries with it an implementation of tx, and register_rx. To
// use a given implementation one simply needs to pass the specific type of
// GenericCan<Hw|Uart> to one of the following functions. Since all of these
// types use GenericCan as a base by casting to (GenericCan *). It is possible
// to derive the interface function pointers without knowing about the rest of
// the data carried in the struct. This allows an object oriented approach where
// the remaining variable for a given implementation are "private/protected"
// meanwhile the interface is consistent for all variants of GenericCan.
//
// Example:
//
// GenericCanUart can_uart = { 0 };
// generic_can_uart_init(&can_uart, UART_PORT_1);
//
// // Transmit
// GenericCanMsg my_msg = { 0 };
//
// // Populate...
//
// generic_can_tx((GenericCan *)&can_uart, &my_msg);
//
// The primary advantage of this is that if you build a function that relies on
// CAN it can accept any GenericCan implementation and be ignorant to its
// underlying behavior. For example:
//
// static uint64_t s_data;
//
// void send_internal_data(const GenericCan *can) {
//   GenericCanMsg my_msg = { .data = &s_data };
//   generic_can_tx(&can, &my_msg);
// }
//
// In this way the data can be sent via UART, CAN Hw or CAN Network without the
// caller knowing which specific CAN variant it is dealing with!

// Transmits |msg| using |can|.
StatusCode generic_can_tx(const GenericCan *can, const GenericCanMsg *msg);

// Registers a |rx_handler| to |can| for cases where (GenericCanMsg.id & |mask|)
// == |filter|. Use GENERIC_CAN_EMPTY_MASK for |mask| if an exact match is
// desired.
StatusCode generic_can_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                   uint32_t filter, bool extended, void *context);
