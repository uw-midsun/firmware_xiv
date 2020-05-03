#pragma once
// System config/setup for Plutus
//
// Initializes all board modules for the given board type
//
// Master:
// * CAN device ID: BPS Master
// * Sequenced relays: Main battery
// * BPS Heartbeat TX
// * Powertrain Heartbeat RX
// * AFE + fault monitoring
// * ADC + fault monitoring
// * Killswitch monitoring
//
// Slave:
// * CAN device ID: BPS Slave
// * Sequenced relays: Slave battery
// * BPS Heartbeat RX
// * Killswitch bypass
#include "can.h"
#include "current_sense.h"
#include "heartbeat_rx.h"
#include "ltc_afe.h"

typedef enum {
  PLUTUS_SYS_TYPE_MASTER = 0,
  PLUTUS_SYS_TYPE_SLAVE,
  NUM_PLUTUS_SYS_TYPES,
} PlutusSysType;

typedef struct PlutusSysStorage {
  CanStorage can;
  LtcAfeStorage ltc_afe;
  CurrentSenseStorage current_sense;

  PlutusSysType type;
} PlutusSysStorage;

// Loads the configured type
PlutusSysType plutus_sys_get_type(void);

// Initializes all modules associated with the system type
StatusCode plutus_sys_init(PlutusSysStorage *storage, PlutusSysType type);
