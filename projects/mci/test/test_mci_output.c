#include "mci_output.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "test_helpers.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "exported_enums.h"
#include "event_queue.h"
#include "generic_can_mcp2515.h"
#include "generic_can.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "status.h"
#include "soft_timer.h"

#include "wavesculptor.h"
#include "motor_can.h"


#define TEST_CAN_DEVICE_ID 12
#define TEST_MCI_OUTPUT_THRESHOLD 0.01f

typedef enum {
  TEST_MCI_OUTPUT_PEDAL_EVENT_RX = 0,
  TEST_MCI_OUTPUT_PEDAL_EVENT_TIMEOUT,
  NUM_TEST_MCI_OUTPUT_PEDAL_EVENTS,
} TestMciOutputPedalRxEvent;

typedef struct TestMciOutputStorage {
  MotorCanDriveCommand expected_value;
  bool pedal_sent;
} TestMciOutputStorage;

static MotorControllerStorage s_mci_storage;
static GenericCanMcp2515 s_can_mcp2515;
static TestMciOutputStorage s_test_mci_output_storage;


static inline uint32_t unpack_left_shift_u32(
    uint8_t value,
    uint8_t shift,
    uint8_t mask)
{
    return (uint32_t)((uint32_t)(value & mask) << shift);
}

static inline uint32_t unpack_right_shift_u32(
    uint8_t value,
    uint8_t shift,
    uint8_t mask)
{
    return (uint32_t)((uint32_t)(value & mask) >> shift);
}

int motor_can_drive_command_unpack(
    struct MotorCanDriveCommand *dst_p,
    const uint8_t *src_p,
    size_t size)
{
    uint32_t motor_current;
    uint32_t motor_velocity;

    if (size < 8u) {
        return (-EINVAL);
    }

    memset(dst_p, 0, sizeof(*dst_p));

    motor_velocity = 0u;
    motor_velocity |= unpack_right_shift_u32(src_p[0], 0u, 0xffu);
    motor_velocity |= unpack_left_shift_u32(src_p[1], 8u, 0xffu);
    motor_velocity |= unpack_left_shift_u32(src_p[2], 16u, 0xffu);
    motor_velocity |= unpack_left_shift_u32(src_p[3], 24u, 0xffu);
    memcpy(&dst_p->motor_velocity, &motor_velocity, sizeof(dst_p->motor_velocity));
    motor_current = 0u;
    motor_current |= unpack_right_shift_u32(src_p[4], 0u, 0xffu);
    motor_current |= unpack_left_shift_u32(src_p[5], 8u, 0xffu);
    motor_current |= unpack_left_shift_u32(src_p[6], 16u, 0xffu);
    motor_current |= unpack_left_shift_u32(src_p[7], 24u, 0xffu);
    memcpy(&dst_p->motor_current, &motor_current, sizeof(dst_p->motor_current));

    return (0);
}

static void prv_setup_motor_can(void) {
  Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },

    .can_bitrate = MCP2515_BITRATE_500KBPS,
    .loopback = false,
  };

  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
}

StatusCode TEST_MOCK(mcp2515_tx)(Mcp2515Storage *storage, uint32_t id, bool extended,
                                 uint64_t data, size_t dlc) {
  // unpack data
  if(!s_test_mci_output_storage.pedal_sent) {
    return STATUS_CODE_OK;
  }
  LOG_DEBUG("DID TX\n");
  MotorCanDriveCommand *expected_value = &s_test_mci_output_storage.expected_value;
  MotorCanDriveCommand actual_value;
  motor_can_drive_command_unpack(&actual_value, (uint8_t *)&data, dlc);
  LOG_DEBUG("CURRENT(AvE): %.4f vs %.4f\n", actual_value.motor_current, expected_value->motor_current);
  LOG_DEBUG("VELOCITY(AvE): %.4f vs %.4f %.4f %.4f\n", actual_value.motor_velocity, expected_value->motor_velocity, fabs(actual_value.motor_velocity - expected_value->motor_velocity), TEST_MCI_OUTPUT_THRESHOLD);
  TEST_ASSERT_TRUE(
    id == MOTOR_CAN_LEFT_DRIVE_COMMAND_FRAME_ID ||
    id == MOTOR_CAN_RIGHT_DRIVE_COMMAND_FRAME_ID);
  TEST_ASSERT_TRUE(fabs(actual_value.motor_velocity - expected_value->motor_velocity) <
                     TEST_MCI_OUTPUT_THRESHOLD);
  TEST_ASSERT_TRUE(fabs(actual_value.motor_current - expected_value->motor_current) <
                     TEST_MCI_OUTPUT_THRESHOLD);
  s_test_mci_output_storage.pedal_sent = false;
  // verify id and dlc are as expected
  return STATUS_CODE_OK;
}

static void prv_do_tx_rx_pedal_values(TestMciOutputStorage *storage, PedalValues *pedal_values) {
  // Send pedal value
  s_mci_storage.pedal_storage.pedal_values = *pedal_values;
  storage->pedal_sent = true;
  LOG_DEBUG("START DELAY\n");
  delay_ms(300);
  LOG_DEBUG("END DELAY\n");
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  prv_setup_motor_can();
  PedalRxSettings pedal_settings = {
    .rx_event = TEST_MCI_OUTPUT_PEDAL_EVENT_RX,
    .timeout_event = TEST_MCI_OUTPUT_PEDAL_EVENT_TIMEOUT,
  };
  pedal_rx_init(&s_mci_storage.pedal_storage, &pedal_settings);
  s_mci_storage.motor_can = (GenericCan *)&s_can_mcp2515;
  mci_output_init(&s_mci_storage);
}

void teardown_test(void) {}


void test_mci_output_off_no_pedals(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 0.0f,
    .brake = 0.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.0f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_OFF;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_off_only_throttle(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 50.0f,
    .brake = 0.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.0f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_OFF;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_off_only_brake(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 0.0f,
    .brake = 50.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.0f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_OFF;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_off_both_pedals(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 50.0f,
    .brake = 50.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.0f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_OFF;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_drive_no_pedals(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 0.0f,
    .brake = 0.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.0f,
    .motor_velocity = 100.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_DRIVE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_drive_only_throttle(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 50.0f,
    .brake = 0.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.5f,
    .motor_velocity = 100.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_DRIVE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_drive_only_brake(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 0.0f,
    .brake = 50.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.5f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_DRIVE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_drive_both_pedals(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 50.0f,
    .brake = 50.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.5f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_DRIVE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_reverse_no_pedals(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 0.0f,
    .brake = 0.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.0f,
    .motor_velocity = -100.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_REVERSE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_reverse_only_throttle(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 50.0f,
    .brake = 0.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.5f,
    .motor_velocity = -100.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_REVERSE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_reverse_only_brake(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 0.0f,
    .brake = 50.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.5f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_REVERSE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}

void test_mci_output_reverse_both_pedals(void) {
  LOG_DEBUG("DOING %s\n", __func__);
  PedalValues test_values = {
    .throttle = 50.0f,
    .brake = 50.0f,
  };
  MotorCanDriveCommand expected_value = {
    .motor_current = 0.5f,
    .motor_velocity = 0.0f,
  };
  s_test_mci_output_storage.expected_value = expected_value;
  s_mci_storage.drive_state = EE_DRIVE_OUTPUT_DRIVE;
  prv_do_tx_rx_pedal_values(&s_test_mci_output_storage, &test_values);
  TEST_ASSERT_FALSE(s_test_mci_output_storage.pedal_sent);
}
