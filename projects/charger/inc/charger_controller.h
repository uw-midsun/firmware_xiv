#pragma once

// Acts as the charger driver, handling detailed interaction
// requires gpio, soft timers, CAN, and the event queue to be initialized

#include <stdint.h>

#include "status.h"

#define CHARGER_TX_PERIOD_MS 1000

// values defined by J1772 standard
#define CHARGER_TX_CAN_ID 0x1806E5F4
#define CHARGER_RX_CAN_ID 0x18FF50E5

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

typedef union RxMsgData {
  struct {
    uint8_t out_voltage_high;
    uint8_t out_voltage_low;
    uint8_t out_current_high;
    uint8_t out_current_low;
    union {
      struct {
        uint8_t hardware_failure : 1;
        uint8_t over_temp : 1;
        uint8_t wrong_voltage : 1;
        uint8_t polarity_failure : 1;
        uint8_t communication_timeout : 1;
        uint8_t reserved : 3;
      } flags;
      uint8_t raw;
    } status_flags;
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
