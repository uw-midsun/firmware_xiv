#pragma once

// Acts as the charger driver, handling detailed interaction
// requires gpio, soft timers, CAN, and the event queue to be initialized

#include <stdint.h>

#include "status.h"

#define CHARGER_TX_PERIOD_MS 1000

// values defined by J1772 standard
#define CHARGER_TX_CAN_ID 0x1806E5F4
#define CHARGER_RX_CAN_ID 0x18FF50E5

typedef union ChargerCanJ1939Id {
  uint32_t raw_id;
  struct {
    uint32_t source_address : 8;  // Source
    uint32_t pdu_specifics : 8;   // Destination
    uint32_t pdu_format : 8;      // Packet Format
    uint32_t dp : 1;              // Always 0
    uint32_t r : 1;               // Always 0
    uint32_t priority : 3;        // Anything
  };
} ChargerCanJ1939Id;


typedef union TxMsgData {
  struct {
    uint8_t max_voltage_high;
    uint8_t max_voltage_low;
    uint8_t max_current_high;
    uint8_t max_current_low;
    uint8_t control;
    uint32_t reserved : 24;
  } fields;
  uint64_t raw;
} TxMsgData;


// Use Bitmasks instead of bitfield
#define ELCON_STATUS_HW_FAULT 0x1
#define ELCON_STATUS_OVERTEMP 0x2
#define ELCON_STATUS_INP_VOLTAGE_WRONG 0x4
#define ELCON_STATUS_REVERSE_POLARITY 0x8
#define ELCON_STATUS_COMMS_TIMEOUT 0x10

typedef union RxMsgData {
  struct {
    uint8_t out_voltage_high;
    uint8_t out_voltage_low;
    uint8_t out_current_high;
    uint8_t out_current_low;
    uint8_t status_flags;
    uint32_t reserved : 24;
  } fields;
  uint64_t raw;
} RxMsgData;

// we use a struct to help with endianness
typedef union ChargerVC {
  union {
    struct {
      uint16_t voltage;
      uint16_t current;
    } complete;
    struct {
      uint8_t voltage_low;
      uint8_t voltage_high;
      uint8_t current_low;
      uint8_t current_high;
    } bytes;
  } values;
  uint32_t raw;
} ChargerVC;

StatusCode charger_controller_activate(uint16_t max_allowable_voltage);

StatusCode charger_controller_deactivate();

StatusCode charger_controller_init();
