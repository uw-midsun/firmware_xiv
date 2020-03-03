// Uses an object pool to track the storage for ack requests, but the actual
// requests are handled through an array of request pointers to minimize copying
// ACK requests currently ordered as they were created
#include "can_ack.h"
#include <string.h>

static StatusCode prv_update_req(CanAckRequests *requests, CanMessageId msg_id,
                                 SoftTimerId timer_id, CanAckStatus status, uint16_t device);

static void prv_timeout_cb(SoftTimerId timer_id, void *context);

StatusCode can_ack_init(CanAckRequests *requests) {
  memset(requests, 0, sizeof(*requests));

  requests->num_requests = 0;

  return objpool_init(&requests->pool, requests->request_nodes, NULL, NULL);
}

StatusCode can_ack_add_request(CanAckRequests *requests, CanMessageId msg_id,
                               const CanAckRequest *ack_request) {
  if (ack_request == NULL || ack_request->expected_bitset == 0) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  CanAckPendingReq *pending_ack = objpool_get_node(&requests->pool);
  if (pending_ack == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  memset(pending_ack, 0, sizeof(*pending_ack));
  pending_ack->msg_id = msg_id;
  pending_ack->expected_bitset = ack_request->expected_bitset;
  pending_ack->callback = ack_request->callback;
  pending_ack->context = ack_request->context;
  StatusCode ret =
      soft_timer_start_millis(CAN_ACK_TIMEOUT_MS, prv_timeout_cb, requests, &pending_ack->timer);

  if (ret != STATUS_CODE_OK) {
    objpool_free_node(&requests->pool, pending_ack);
    return ret;
  }

  requests->active_requests[requests->num_requests++] = pending_ack;

  return STATUS_CODE_OK;
}

StatusCode can_ack_handle_msg(CanAckRequests *requests, const CanMessage *msg) {
  return prv_update_req(requests, msg->msg_id, SOFT_TIMER_INVALID_TIMER, msg->data, msg->source_id);
}

static StatusCode prv_update_req(CanAckRequests *requests, CanMessageId msg_id,
                                 SoftTimerId timer_id, CanAckStatus status, uint16_t device) {
  CanAckPendingReq *found_request = NULL;
  size_t index = 0;

  // Requests should be in the order that they were made, and there's a higher
  // chance that requests made first will be serviced first. In the case where
  // we're searching for a message ID, we'd like to pick the ACK request closest
  // to expiry, which should be the first one we encounter because we keep them
  // in the order they were made.

  // Essentially checks if an ACK has been received from the device already and
  // * The message ID matches given an invalid timer
  // * The timer ID matches given an invalid message ID
  // * Both message and timer match given valid values for both
  for (index = 0; index < requests->num_requests; index++) {
    CanAckPendingReq *req = requests->active_requests[index];
    if (((req->msg_id == msg_id && timer_id == SOFT_TIMER_INVALID_TIMER) ||
         (req->timer == timer_id && msg_id == CAN_MSG_INVALID_ID) ||
         (req->msg_id == msg_id && req->timer == timer_id)) &&
        (device == CAN_MSG_INVALID_DEVICE ||
         ((req->response_bitset & ((uint32_t)1 << device)) == 0 &&
          (req->expected_bitset & ((uint32_t)1 << device)) != 0))) {
      found_request = req;
      break;
    }
  }

  if (found_request == NULL) {
    return status_code(STATUS_CODE_UNKNOWN);
  }

  // We use a bitset to keep track of which devices we've received an ACK for
  // this message from
  if (device != CAN_MSG_INVALID_DEVICE) {
    found_request->response_bitset |= ((uint32_t)1 << device);
  }

  if (found_request->callback != NULL) {
    // Since we always check if a device was expected, we don't need to actually
    // mask it
    uint16_t num_remaining =
        __builtin_popcount(found_request->response_bitset ^ found_request->expected_bitset);
    if (num_remaining == 0 && device == CAN_MSG_INVALID_DEVICE) {
      // TODO(ELEC-457): Does this get cleaned up?
      return STATUS_CODE_OK;
    }
    StatusCode ret = found_request->callback(found_request->msg_id, device, status, num_remaining,
                                             found_request->context);
    // If we ran into an error and the return code was not ok,
    // we want to pretend the ACK has not been received
    if (ret != STATUS_CODE_OK && device != CAN_MSG_INVALID_DEVICE) {
      found_request->response_bitset &= ~((uint32_t)1 << device);
    }
  }

  // The response bitset should only ever be set by devices in the expected
  // bitset, so we don't need to mask the value here.
  if (found_request->response_bitset == found_request->expected_bitset ||
      status == CAN_ACK_STATUS_TIMEOUT) {
    soft_timer_cancel(found_request->timer);
    StatusCode ret = objpool_free_node(&requests->pool, found_request);
    status_ok_or_return(ret);

    requests->num_requests--;
    if (index != requests->num_requests) {
      // Shift all requests to the left by 1
      memmove(&requests->active_requests[index], &requests->active_requests[index + 1],
              sizeof(requests->active_requests[0]) * (requests->num_requests - index));
    }

    requests->active_requests[requests->num_requests] = NULL;
  }

  return STATUS_CODE_OK;
}

static void prv_timeout_cb(SoftTimerId timer_id, void *context) {
  CanAckRequests *requests = context;

  prv_update_req(requests, CAN_MSG_INVALID_ID, timer_id, CAN_ACK_STATUS_TIMEOUT,
                 CAN_MSG_INVALID_DEVICE);
}
