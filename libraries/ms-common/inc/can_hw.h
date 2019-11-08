#pragma once
// CAN HW Interface
// Requires GPIO and interrupts to be initialized.
//
// Used to initiate CAN TX and RX directly through the MCU, without any
// preprocessing or queues. Note that none of our systems currently support more
// than one CAN interface natively.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gpio.h"
#include "status.h"

// Used to process HW events within the CAN ISR, ideally as short as possible.
typedef void (*CanHwEventHandlerCb)(void *context);

typedef enum {
  CAN_HW_EVENT_TX_READY = 0,
  CAN_HW_EVENT_MSG_RX,
  CAN_HW_EVENT_BUS_ERROR,
  NUM_CAN_HW_EVENTS
} CanHwEvent;

typedef enum {
  CAN_HW_BUS_STATUS_OK = 0,
  CAN_HW_BUS_STATUS_ERROR,
  CAN_HW_BUS_STATUS_OFF
} CanHwBusStatus;

typedef enum {
  CAN_HW_BITRATE_125KBPS,
  CAN_HW_BITRATE_250KBPS,
  CAN_HW_BITRATE_500KBPS,
  CAN_HW_BITRATE_1000KBPS,
  NUM_CAN_HW_BITRATES
} CanHwBitrate;

typedef struct CanHwSettings {
  GpioAddress tx;
  GpioAddress rx;
  CanHwBitrate bitrate;
  bool loopback;
} CanHwSettings;

// Initializes CAN using the specified settings.
StatusCode can_hw_init(const CanHwSettings *settings);

// Registers a callback for the given event
StatusCode can_hw_register_callback(CanHwEvent event, CanHwEventHandlerCb callback, void *context);

StatusCode can_hw_add_filter(uint32_t mask, uint32_t filter, bool extended);

CanHwBusStatus can_hw_bus_status(void);

StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len);

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len);
