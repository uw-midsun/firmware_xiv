#include "can.h"
#include "can_rx_event_mapper.h"
#include "delay.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_DEVICE_ID 0x1

#define PROCESS_DELAY_US 2000

#define TEST_RX_EVENT_COMMON(msg, can_msg_id, can_data_0, can_data_1) \
  ({                                                                  \
    delay_us(PROCESS_DELAY_US);                                       \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();                          \
    (msg).msg_id = (can_msg_id);                                      \
    (msg).data_u16[0] = (can_data_0);                                 \
    (msg).data_u16[1] = (can_data_1);                                 \
    TEST_ASSERT_OK(can_transmit(&(msg), NULL));                       \
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);   \
  })

#define TEST_RX_TO_EVENT(msg, can_msg_id, can_data_0, can_data_1, event_id, event_data) \
  ({                                                                                    \
    TEST_RX_EVENT_COMMON(msg, can_msg_id, can_data_0, can_data_1);                      \
    Event e = { 0, 0 };                                                                 \
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (event_id), (event_data));                      \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();                                            \
  })

#define TEST_RX_NO_EVENT(msg, can_msg_id, can_data_0, can_data_1)  \
  ({                                                               \
    TEST_RX_EVENT_COMMON(msg, can_msg_id, can_data_0, can_data_1); \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();                       \
  })

#define TEST_RX_TO_EVENT_WITH_ACK(msg, can_msg_id, can_data_0, can_data_1, event_id, event_data) \
  ({                                                                                             \
    TEST_RX_EVENT_COMMON(msg, can_msg_id, can_data_0, can_data_1);                               \
    Event e = { 0, 0 };                                                                          \
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (event_id), (event_data));                               \
    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX); /* for the ack */            \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();                                                     \
  })
typedef enum {
  TEST_CAN_MESSAGE_0 = 20,
  TEST_CAN_MESSAGE_1,
  TEST_CAN_MESSAGE_2,
  TEST_CAN_MESSAGE_3_ACKABLE = 0,  // 0 because we can only ack small-ID messages
} TestCanMessage;

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

typedef enum {
  TEST_EVENT_0 = NUM_TEST_CAN_EVENTS + 1,
  TEST_EVENT_1,
  NUM_TEST_EVENTS,
} TestEvent;

typedef enum {
  TEST_TYPE_0 = 0,
  TEST_TYPE_1,
  NUM_TEST_TYPES,
} TestType;

static CanStorage s_can_storage;

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },  // probably fine for testing
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}
void teardown_test(void) {}

// Test that we can specify types but no states and no ack.
void test_can_rx_event_mapper_type_state_no_ack_works() {
  PowerDistributionCanRxEventMapperConfig test_config = {
    .msg_specs =
        (PowerDistributionCanRxEventMapperMsgSpec[]){
            {
                // type and state, no ack
                .msg_id = TEST_CAN_MESSAGE_0,
                .has_type = true,
                .has_state = true,
                .all_types =
                    (uint16_t[]){
                        TEST_TYPE_0,
                        TEST_TYPE_1,
                    },
                .num_types = 2,
                .type_to_event_id =
                    (EventId[]){
                        [TEST_TYPE_0] = TEST_EVENT_0,
                        [TEST_TYPE_1] = TEST_EVENT_1,
                    },
                .ack = false,
            },
        },
    .num_msg_specs = 1,
  };

  TEST_ASSERT_OK(power_distribution_can_rx_event_mapper_init(test_config));

  CanMessage msg = {
    .source_id = TEST_CAN_DEVICE_ID,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 8,
  };

  // try with type and state
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_0, TEST_TYPE_0, 0, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_0, TEST_TYPE_0, 1, TEST_EVENT_0, 1);
  // coalesces nonzero state to 1
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_0, TEST_TYPE_0, 20, TEST_EVENT_0, 1);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_0, TEST_TYPE_1, 0, TEST_EVENT_1, 0);
  // doesn't respond to unknown types
  TEST_RX_NO_EVENT(msg, TEST_CAN_MESSAGE_0, NUM_TEST_TYPES, 0);
}

// Test that we can specify a type without a state and no ack.
void test_can_rx_event_mapper_type_no_state_no_ack_works() {
  PowerDistributionCanRxEventMapperConfig test_config = {
    .msg_specs =
        (PowerDistributionCanRxEventMapperMsgSpec[]){
            {
                // type, no state, no ack
                .msg_id = TEST_CAN_MESSAGE_1,
                .has_type = true,
                .has_state = false,
                .all_types =
                    (uint16_t[]){
                        TEST_TYPE_0,
                        TEST_TYPE_1,
                    },
                .num_types = 2,
                .type_to_event_id =
                    (EventId[]){
                        [TEST_TYPE_0] = TEST_EVENT_0,
                        [TEST_TYPE_1] = TEST_EVENT_1,
                    },
                .ack = false,
            },
        },
    .num_msg_specs = 1,
  };

  TEST_ASSERT_OK(power_distribution_can_rx_event_mapper_init(test_config));

  CanMessage msg = {
    .source_id = TEST_CAN_DEVICE_ID,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 8,
  };

  // try with type and no state - always sets state to 0
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_1, TEST_TYPE_0, 20, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_1, TEST_TYPE_0, 1, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_1, TEST_TYPE_1, 0, TEST_EVENT_1, 0);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_1, TEST_TYPE_1, 30, TEST_EVENT_1, 0);
  TEST_RX_NO_EVENT(msg, TEST_CAN_MESSAGE_1, NUM_TEST_TYPES, 0);
}

// Test that we can specify a state with no type and no ack.
void test_can_rx_event_mapper_state_no_type_no_ack_works() {
  PowerDistributionCanRxEventMapperConfig test_config = {
    .msg_specs =
        (PowerDistributionCanRxEventMapperMsgSpec[]){
            {
                // state, no type, no ack
                .msg_id = TEST_CAN_MESSAGE_2,
                .has_type = false,
                .has_state = true,
                .event_id = TEST_EVENT_0,
                .ack = false,
            },
        },
    .num_msg_specs = 1,
  };

  TEST_ASSERT_OK(power_distribution_can_rx_event_mapper_init(test_config));

  CanMessage msg = {
    .source_id = TEST_CAN_DEVICE_ID,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 8,
  };

  // try with state and no type - in first u16 spot
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_2, 0, 0, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_2, 1, 0, TEST_EVENT_0, 1);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_2, 20, 0, TEST_EVENT_0, 1);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_2, 1, 30, TEST_EVENT_0, 1);
}

// Test that we can specify no state, no type, but test ack true and false.
void test_can_rx_event_mapper_no_type_no_state_with_varying_ack_works() {
  PowerDistributionCanRxEventMapperConfig test_config = {
    .msg_specs =
        (PowerDistributionCanRxEventMapperMsgSpec[]){
            {
                // no state or type, no ack
                .msg_id = TEST_CAN_MESSAGE_2,
                .has_type = false,
                .has_state = false,
                .event_id = TEST_EVENT_0,
                .ack = false,
            },
            {
                // no state or type, with ack
                .msg_id = TEST_CAN_MESSAGE_3_ACKABLE,
                .has_type = false,
                .has_state = false,
                .event_id = TEST_EVENT_0,
                .ack = true,
            },
        },
    .num_msg_specs = 2,
  };

  TEST_ASSERT_OK(power_distribution_can_rx_event_mapper_init(test_config));

  CanMessage msg = {
    .source_id = TEST_CAN_DEVICE_ID,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 8,
  };

  // try with no state or type - doesn't respond to data, state is 0
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_2, 0, 0, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_2, 1, 4, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT(msg, TEST_CAN_MESSAGE_2, 10, 5, TEST_EVENT_0, 0);

  // try with acking now
  TEST_RX_TO_EVENT_WITH_ACK(msg, TEST_CAN_MESSAGE_3_ACKABLE, 0, 0, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT_WITH_ACK(msg, TEST_CAN_MESSAGE_3_ACKABLE, 1, 4, TEST_EVENT_0, 0);
  TEST_RX_TO_EVENT_WITH_ACK(msg, TEST_CAN_MESSAGE_3_ACKABLE, 10, 5, TEST_EVENT_0, 0);
}
