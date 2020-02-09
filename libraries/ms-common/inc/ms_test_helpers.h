#pragma once
// Test helpers for common test paradigms.
//
// All require event_queue to be initialized.

#include "can.h"
#include "event_queue.h"
#include "fsm.h"
#include "status.h"
#include "unity.h"

// Awaits an event and populates |e| with that event.
#define MS_TEST_HELPER_AWAIT_EVENT(e)     \
  ({                                      \
    StatusCode status = NUM_STATUS_CODES; \
    do {                                  \
      status = event_process(&(e));       \
    } while (status != STATUS_CODE_OK);   \
  })

// The following require CAN to be initialized.

// Send a TX message over CAN and RX it
#define MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event) \
  ({                                                 \
    MS_TEST_HELPER_CAN_TX(tx_event);                 \
    MS_TEST_HELPER_CAN_RX(rx_event);                 \
  })

// Send a TX message over CAN
#define MS_TEST_HELPER_CAN_TX(tx_event)      \
  ({                                         \
    Event e = { 0, 0 };                      \
    MS_TEST_HELPER_AWAIT_EVENT(e);           \
    TEST_ASSERT_EQUAL((tx_event), e.id);     \
    TEST_ASSERT_TRUE(can_process_event(&e)); \
  })

// RX a TX'd Message.
#define MS_TEST_HELPER_CAN_RX(rx_event)      \
  ({                                         \
    Event e = { 0, 0 };                      \
    MS_TEST_HELPER_AWAIT_EVENT(e);           \
    TEST_ASSERT_EQUAL((rx_event), e.id);     \
    TEST_ASSERT_TRUE(can_process_event(&e)); \
  })

// Send a TX message over CAN and RX it, then respond with an ACK.
#define MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(tx_event, rx_event) \
  ({                                                          \
    MS_TEST_HELPER_CAN_TX_RX((tx_event), (rx_event));         \
    MS_TEST_HELPER_CAN_TX_RX((tx_event), (rx_event));         \
  })

#define MS_TEST_HELPER_ASSERT_EVENT(event, e_id, e_data) \
  ({                                                     \
    TEST_ASSERT_OK(event_process(&e));                   \
    TEST_ASSERT_EQUAL((e_id), event.id);                 \
    TEST_ASSERT_EQUAL((e_data), event.data);             \
  })

// assert next event
#define MS_TEST_HELPER_ASSERT_NEXT_EVENT(event, e_id, e_data) \
  ({                                                          \
    MS_TEST_HELPER_AWAIT_EVENT(event);                        \
    TEST_ASSERT_EQUAL((e_id), event.id);                      \
    TEST_ASSERT_EQUAL((e_data), event.data);                  \
  })

// assert no events
#define MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED() \
  ({                                                          \
    Event e = { 0, 0 };                                   \
    TEST_ASSERT_NOT_OK(event_process(&(e)));        \
  })
