#include "can_rx_event_mapper.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "event_queue.h"
#include "log.h"
#include "pd_events.h"

static CanRxEventMapperConfig *s_config;

static StatusCode prv_handle_rx(const CanMessage *msg, void *context, CanAckStatus *ack) {
  CanRxEventMapperMsgSpec *spec = context;

  EventId event_id;
  if (spec->has_type) {
    uint16_t type = msg->data_u16[0];

    // make sure it exists
    bool found = false;
    for (uint8_t i = 0; i < spec->num_types; i++) {
      if (spec->all_types[i] == type) {
        found = true;
        break;
      }
    }
    if (!found) {
      // silently ignore nonmatching ones - they must not be meant for us
      return STATUS_CODE_OK;
    }

    event_id = spec->type_to_event_id[type];
  } else {
    event_id = spec->event_id;
  }

  uint16_t data = 0;
  if (spec->has_state) {
    uint16_t raw_data = msg->data_u16[spec->has_type ? 1 : 0];

    // coalesce nonzero to 1 to avoid errors due to using u8 or u16
    data = (raw_data == 0) ? 0 : 1;
  }

  StatusCode status = event_raise_priority(PD_ACTION_EVENT_PRIORITY, event_id, data);
  if (!status_ok(status)) {
    LOG_WARN("WARNING: can_rx_event_mapper failed to raise event! id=%d, data=%d, status=%d\n",
             event_id, data, status);
  }
  return status;
}

StatusCode can_rx_event_mapper_init(CanRxEventMapperConfig *config) {
  s_config = config;

  for (uint8_t i = 0; i < config->num_msg_specs; i++) {
    status_ok_or_return(can_register_rx_handler(s_config->msg_specs[i].msg_id, prv_handle_rx,
                                                &s_config->msg_specs[i]));
  }

  return STATUS_CODE_OK;
}
