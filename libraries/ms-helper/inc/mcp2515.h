#pragma once
// MCP2515 SPI CAN controller
// Requires SPI, GPIO, GPIO interrupts, interrupts to be initialized
//
// Note that we aren't bothering to implement filtering on the controller side. We'll just filter
// in software since these are on isolated networks.
//
// Note that this is hardcoded to 500kbps and we assume that a 16MHz crystal is attached.
#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "mcp2515_defs.h"
#include "soft_timer.h"
#include "spi.h"
#include "status.h"

// Called on CAN messsage RX
typedef void (*Mcp2515RxCb)(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context);

// Called on CAN bus error
struct Mcp2515Errors;
typedef void (*Mcp2515BusErrorCb)(const struct Mcp2515Errors *errors, void *context);

typedef struct Mcp2515Errors {
  uint8_t eflg;
  uint8_t tec;
  uint8_t rec;
} Mcp2515Errors;

typedef enum {
  MCP2515_BITRATE_500KBPS,
  MCP2515_BITRATE_250KBPS,
  MCP2515_BITRATE_125KBPS,
  NUM_MCP2515_BITRATES,
} Mcp2515Bitrate;

typedef enum {
  MCP2515_FILTER_ID_RXF0 = 0,
  MCP2515_FILTER_ID_RXF1,
  NUM_MCP2515_FILTER_IDS
} Mcp2515FiltersIds;

typedef struct Mcp2515Settings {
  SpiPort spi_port;
  uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;

  GpioAddress int_pin;

  bool loopback;
  Mcp2515Bitrate can_bitrate;
  // Just set the filters[i].raw, don't set individual parts of the id manually
  // Otherwise it will be handled incorrectly when setting the filters
  Mcp2515Id filters[NUM_MCP2515_FILTER_IDS];

  Mcp2515RxCb rx_cb;
  Mcp2515BusErrorCb bus_err_cb;
  void *context;
} Mcp2515Settings;

typedef struct Mcp2515Storage {
  SpiPort spi_port;
  GpioAddress int_pin;

  Mcp2515RxCb rx_cb;
  Mcp2515BusErrorCb bus_err_cb;
  void *context;

  Mcp2515Errors errors;
} Mcp2515Storage;

// Initializes the MCP2515 CAN controller.
StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings);

// Sets the CAN message RX callback.
StatusCode mcp2515_register_cbs(Mcp2515Storage *storage, Mcp2515RxCb rx_cb,
                                Mcp2515BusErrorCb bus_err_cb, void *context);

// Transmits a CAN message.
StatusCode mcp2515_tx(Mcp2515Storage *storage, uint32_t id, bool extended, uint64_t data,
                      size_t dlc);

// Change the MCP2515 filters.  Takes in a list of new filters to configure the MCP2515 with
// Note: sizeof(filters) must equal NUM_MCP2515_FILTER_IDs
// todo: look at removing mask config from this (I think we're always masking for 0xFF)
StatusCode mcp2515_set_filter(Mcp2515Storage *storage, uint32_t *filters);
