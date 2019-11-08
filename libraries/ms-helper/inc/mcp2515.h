#pragma once
// MCP2515 SPI CAN controller
// Requires SPI, GPIO, GPIO interrupts, interrupts to be initialized
//
// Note that we aren't bothering to implement filtering on the controller side.
// We'll just filter in software since these are on isolated networks.
//
// Note that this is hardcoded to 500kbps and we assume that a 16MHz crystal is
// attached.
#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"

// Called on CAN messsage RX
typedef void (*Mcp2515RxCb)(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context);

typedef struct Mcp2515Settings {
  SpiPort spi_port;
  uint32_t baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;

  GpioAddress int_pin;

  bool loopback;

  Mcp2515RxCb rx_cb;
  void *context;
} Mcp2515Settings;

typedef struct Mcp2515Storage {
  SpiPort spi_port;
  Mcp2515RxCb rx_cb;
  void *context;
} Mcp2515Storage;

// Initializes the MCP2515 CAN controller.
StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings);

// Sets the CAN message RX callback.
StatusCode mcp2515_register_rx_cb(Mcp2515Storage *storage, Mcp2515RxCb rx_cb, void *context);

// Transmits a CAN message.
StatusCode mcp2515_tx(Mcp2515Storage *storage, uint32_t id, bool extended, uint64_t data,
                      size_t dlc);
