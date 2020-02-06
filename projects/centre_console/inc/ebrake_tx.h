#pragma once

#include "status.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "can_tx_retry_wrapper.h"

typedef struct EbrakeTxStorage {
  uint8_t retries;
  EventId fault_event_id;
  uint16_t fault_event_data;
  EventId completion_event_id;
  uint16_t completion_event_data;
  bool retry_indefinitely;
  EERelayState state;
} EbrakeTxStorage;

typedef struct EbrakeTxRequest {
  EventId fault_event_id;
  uint16_t fault_event_data;
  EventId completion_event_id;
  uint16_t completion_event_data;
  bool retry_indefinitely;
  EEEbrakeState  state;
} EbrakeTxRequest;

StatusCode ebrake_tx_init();

StatusCode ebrake_tx_state(EEEbrakeState state);

StatusCode ebrake_tx_state_and_raise_event(EbrakeTxStorage *storage, EbrakeTxRequest *request);
