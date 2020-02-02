#include "status.h"
#include "event_queue.h"
#include "exported_enums.h"

typedef struct EbrakeTxRequest {
  EventId completion_event_id;
  uint16_t completion_event_data;
  bool retry_indefinitely;
  EEEbrakeState  state;
} EbrakeTxRequest;

StatusCode ebrake_tx_init();

StatusCode ebrake_tx_state(EEEbrakeState state);

StatusCode ebrake_tx_state_and_raise_event(EbrakeTxRequest *tx_req);
