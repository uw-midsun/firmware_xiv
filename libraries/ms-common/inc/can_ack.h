#pragma once
// CAN ACK handling
// Requires soft timers and interrupts to be initialized.
//
// ACK request objects are provided with critical messages.
// They define a callback that should be run when the ACK status is updated.
// The ACK status is updated when
// a) We receive an ACK over CAN
// b) The timer expires and we timeout the ACK
//
// If the ACK has timed out or we've received the expected number of ACKs, we
// remove the ACK request from the list. We use an object pool and an array of
// pointers to do so efficiently.
#include <assert.h>
#include <limits.h>
#include "can_msg.h"
#include "objpool.h"
#include "soft_timer.h"
#include "status.h"

// ACK timeout: Should account for transit and computation time
// Note that this timeout is currently an arbitrary value, but should be
// minimized.
#define CAN_ACK_TIMEOUT_MS 25
#define CAN_ACK_MAX_REQUESTS 10

// Converts devices IDs to their bitset form. Populate ACK request bitsets using
// this. Example: ack_request.expected_bitset =
// CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_A, CAN_DEVICE_D)
#define CAN_ACK_EXPECTED_DEVICES(...)                       \
  ({                                                        \
    uint32_t device_ids[] = { __VA_ARGS__ };                \
    uint32_t bitset = 0;                                    \
    for (size_t i = 0; i < SIZEOF_ARRAY(device_ids); i++) { \
      bitset |= ((uint32_t)1 << device_ids[i]);             \
    }                                                       \
    bitset;                                                 \
  })

typedef enum {
  CAN_ACK_STATUS_OK = 0,
  CAN_ACK_STATUS_TIMEOUT,
  CAN_ACK_STATUS_INVALID,
  CAN_ACK_STATUS_UNKNOWN,
  NUM_CAN_ACK_STATUSES,
} CanAckStatus;

// If the callback was fired due to timer expiry, the device ID is invalid.
// If the return code is non-OK, it is assumed that the received ACK is invalid
// and should be ignored. If this occurs on a timer expiry, we still remove the
// ACK request.
typedef StatusCode (*CanAckRequestCb)(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                      uint16_t num_remaining, void *context);

typedef struct CanAckRequest {
  CanAckRequestCb callback;
  void *context;
  uint32_t expected_bitset;
} CanAckRequest;
static_assert(SIZEOF_FIELD(CanAckRequest, expected_bitset) * CHAR_BIT >= CAN_MSG_MAX_DEVICES,
              "CAN ACK request expected bitset field not large enough to fit "
              "all CAN devices!");

typedef struct CanAckPendingReq {
  CanAckRequestCb callback;
  void *context;
  uint32_t expected_bitset;
  uint32_t response_bitset;
  SoftTimerId timer;
  CanMessageId msg_id;
} CanAckPendingReq;
static_assert(SIZEOF_FIELD(CanAckPendingReq, expected_bitset) * CHAR_BIT >= CAN_MSG_MAX_DEVICES,
              "CAN pending ACK expected bitset field not large enough to fit "
              "all CAN devices!");
static_assert(SIZEOF_FIELD(CanAckPendingReq, expected_bitset) ==
                  SIZEOF_FIELD(CanAckPendingReq, response_bitset),
              "CAN pending ACK expected bitset size not equal to response bitset size");

typedef struct CanAckRequests {
  ObjectPool pool;
  CanAckPendingReq request_nodes[CAN_ACK_MAX_REQUESTS];
  CanAckPendingReq *active_requests[CAN_ACK_MAX_REQUESTS];
  size_t num_requests;
} CanAckRequests;

StatusCode can_ack_init(CanAckRequests *requests);

// ack_request's expected bitset should be populated using
// CAN_ACK_EXPECTED_DEVICES.
StatusCode can_ack_add_request(CanAckRequests *requests, CanMessageId msg_id,
                               const CanAckRequest *ack_request);

// Handle a received ACK, firing the callback associated with the received
// message
StatusCode can_ack_handle_msg(CanAckRequests *requests, const CanMessage *msg);
