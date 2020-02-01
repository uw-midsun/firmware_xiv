#pragma once
// MCP2515 SPI CAN controller
// Requires SPI, GPIO, GPIO interrupts, interrupts to be initialized
//
// We assume that a 16MHz crystal is attached.
#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"
#include "soft_timer.h"

// Called on CAN messsage RX
typedef void (*Mcp2515RxCb)(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context);

typedef enum  { 
  MCP_2515_CAN_BITRATE_125KBPS = 0,
  MCP_2515_CAN_BITRATE_250KBPS,
  MCP_2515_CAN_BITRATE_500KBPS,
  NUM_MCP_2515_CAN_BITRATE_KBPS  
} Mcp2515CanBitrate;

// Called on CAN bus error
struct Mcp2515Errors;
typedef void (*Mcp2515BusErrorCb)(const struct Mcp2515Errors *errors, void *context);

typedef struct Mcp2515Errors {
  uint8_t eflg;
  uint8_t tec;
  uint8_t rec;
} Mcp2515Errors;

typedef struct Mcp2515Settings {
  SpiPort spi_port;
  uint32_t spi_baudrate;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;

  GpioAddress int_pin;
  Mcp2515Bitrate can_bitrate;
  bool loopback;

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

// Poll interrupt pin for updates
void mcp2515_poll(Mcp2515Storage *storage);

void mcp2515_watchdog(SoftTimerId timer_id, void *context);
