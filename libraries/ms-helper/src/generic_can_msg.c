#include "generic_can_msg.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "can_msg.h"
#include "status.h"

StatusCode generic_can_msg_to_can_message(const GenericCanMsg *src, CanMessage *dst) {
  if (src->extended || src->id != (uint16_t)src->id) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  CAN_MSG_SET_RAW_ID(dst, (uint16_t)src->id);
  dst->data = src->data;
  dst->dlc = src->dlc;
  return STATUS_CODE_OK;
}

StatusCode can_message_to_generic_can_message(const CanMessage *src, GenericCanMsg *dst) {
  // Make this volatile to bypass a bug with -Wclobbered
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65041
  volatile CanId raw_id = {
    .source_id = src->source_id,
    .msg_id = src->msg_id,
    .type = src->type,
  };
  dst->id = raw_id.raw;
  dst->extended = false;
  dst->data = src->data;
  dst->dlc = src->dlc;
  return STATUS_CODE_OK;
}
