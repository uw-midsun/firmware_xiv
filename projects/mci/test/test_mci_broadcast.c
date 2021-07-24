#include "mci_broadcast.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ms_test_helpers.h"
#include "test_helpers.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can.h"
#include "generic_can_mcp2515.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "soft_timer.h"
#include "status.h"

#include "mci_events.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "wavesculptor.h"
#include "mci_status.h"

#define TEST_CAN_DEVICE_ID 12

#define M_TO_CM_CONV 100

typedef enum {
  TEST_MCI_VELOCITY_MESSAGE = 0,
  TEST_MCI_BUS_MEASUREMENT_MESSAGE,
  TEST_MCI_STATUS_MESSAGE,
  TEST_MCI_MOTOR_TEMP_MESSAGE,
  TEST_MCI_DSP_TEMP_MESSAGE,
  NUM_TEST_MCI_MESSAGES
} TestMciMessage;

static Mcp2515Storage s_motor_can_storage;
static CanStorage s_can_storage;
static MotorControllerBroadcastStorage s_broadcast_storage;

static bool s_received_velocity = false;
static bool s_received_bus_measurement = false;
static bool s_received_status = false;

static MotorControllerBroadcastSettings s_broadcast_settings =
    { .motor_can = &s_motor_can_storage,
      .device_ids = {
          [LEFT_MOTOR_CONTROLLER] = MOTOR_CAN_ID_LEFT_MOTOR_CONTROLLER,
          [RIGHT_MOTOR_CONTROLLER] = MOTOR_CAN_ID_RIGHT_MOTOR_CONTROLLER,
      } };

typedef struct TestWaveSculptorBusMeasurement {
  uint16_t bus_voltage_v;
  uint16_t bus_current_a;
} TestWaveSculptorBusMeasurement;

typedef struct TestMotorControllerMeasurements {
  TestWaveSculptorBusMeasurement bus_measurements[NUM_MOTOR_CONTROLLERS];
  uint16_t vehicle_velocity[NUM_MOTOR_CONTROLLERS];
  MciStatusMessage status;
} TestMotorControllerMeasurements;

static TestMotorControllerMeasurements s_test_measurements = { 0 };

// To simplify frame ID map below
#define L_MTR_MSG_IDX(message) (LEFT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + (message))
#define R_MTR_MSG_IDX(message) (RIGHT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + (message))

#define L_MTR_MSG_ID(offset) (LEFT_MOTOR_CONTROLLER_BASE_ADDR + (offset))
#define R_MTR_MSG_ID(offset) (RIGHT_MOTOR_CONTROLLER_BASE_ADDR + (offset))

static MotorCanFrameId s_frame_id_map[] = {
  [L_MTR_MSG_IDX(TEST_MCI_STATUS_MESSAGE)] = L_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_STATUS),
  [R_MTR_MSG_IDX(TEST_MCI_STATUS_MESSAGE)] = R_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_STATUS),

  [L_MTR_MSG_IDX(TEST_MCI_BUS_MEASUREMENT_MESSAGE)] = L_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_BUS),
  [R_MTR_MSG_IDX(TEST_MCI_BUS_MEASUREMENT_MESSAGE)] = R_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_BUS),

  [L_MTR_MSG_IDX(TEST_MCI_VELOCITY_MESSAGE)] = L_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_VELOCITY),
  [R_MTR_MSG_IDX(TEST_MCI_VELOCITY_MESSAGE)] = R_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_VELOCITY),

  [L_MTR_MSG_IDX(TEST_MCI_MOTOR_TEMP_MESSAGE)] =
      L_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_SINK_MOTOR_TEMPERATURE),
  [R_MTR_MSG_IDX(TEST_MCI_MOTOR_TEMP_MESSAGE)] =
      R_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_SINK_MOTOR_TEMPERATURE),

  [L_MTR_MSG_IDX(TEST_MCI_DSP_TEMP_MESSAGE)] =
      L_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_DSP_BOARD_TEMPERATURE),
  [R_MTR_MSG_IDX(TEST_MCI_DSP_TEMP_MESSAGE)] =
      R_MTR_MSG_ID(WAVESCULPTOR_MEASUREMENT_ID_DSP_BOARD_TEMPERATURE),
};


// To allow for setting fan fault bitset
static uint8_t s_test_fan_fault_bitset = 0;
uint8_t TEST_MOCK(mci_fan_get_fault_bitset)(void) {
  return s_test_fan_fault_bitset;
}

static StatusCode prv_handle_velocity(const CanMessage *msg, void *context,
                                      CanAckStatus *ack_reply) {
  uint16_t left_velocity, right_velocity;
  CAN_UNPACK_MOTOR_VELOCITY(msg, &left_velocity, &right_velocity);
  s_test_measurements.vehicle_velocity[LEFT_MOTOR_CONTROLLER] = left_velocity;
  s_test_measurements.vehicle_velocity[RIGHT_MOTOR_CONTROLLER] = right_velocity;
  s_received_velocity = true;
  return STATUS_CODE_OK;
}

static StatusCode prv_handle_bus_measurement(const CanMessage *msg, void *context,
                                             CanAckStatus *ack_reply) {
  uint16_t left_voltage, left_current, right_voltage, right_current;
  CAN_UNPACK_MOTOR_CONTROLLER_VC(msg, &left_voltage, &left_current, &right_voltage, &right_current);
  s_test_measurements.bus_measurements[LEFT_MOTOR_CONTROLLER].bus_voltage_v = left_voltage;
  s_test_measurements.bus_measurements[LEFT_MOTOR_CONTROLLER].bus_current_a = left_current;
  s_test_measurements.bus_measurements[RIGHT_MOTOR_CONTROLLER].bus_voltage_v = right_voltage;
  s_test_measurements.bus_measurements[RIGHT_MOTOR_CONTROLLER].bus_current_a = right_current;
  s_received_bus_measurement = true;
  return STATUS_CODE_OK;
}

static StatusCode prv_handle_status(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  MciStatusMessage message;
  CAN_UNPACK_MOTOR_STATUS(msg, &message.mc_limit_bitset[LEFT_MOTOR_CONTROLLER], &message.mc_limit_bitset[RIGHT_MOTOR_CONTROLLER],
  &message.mc_error_bitset[LEFT_MOTOR_CONTROLLER], &message.mc_error_bitset[RIGHT_MOTOR_CONTROLLER], &message.board_fault_bitset, 
  &message.mc_overtemp_bitset);
  s_received_status = true;
  memcpy(&s_test_measurements.status, &message, sizeof(message));
  return STATUS_CODE_OK;
}

static void prv_send_measurements(MotorController controller, TestMciMessage message_type,
                                  MotorControllerMeasurements *measurements) {
  WaveSculptorCanData can_data = { 0 };
  if (message_type == TEST_MCI_VELOCITY_MESSAGE) {
    can_data.velocity_measurement.motor_velocity_rpm = 0;
    can_data.velocity_measurement.vehicle_velocity_ms = measurements->vehicle_velocity[controller];
  } else if (message_type == TEST_MCI_BUS_MEASUREMENT_MESSAGE) {
    can_data.bus_measurement.bus_voltage_v =
        measurements->bus_measurements[controller].bus_voltage_v;
    can_data.bus_measurement.bus_current_a =
        measurements->bus_measurements[controller].bus_current_a;
  } else if (message_type == TEST_MCI_STATUS_MESSAGE) {
    // Populate status message flags we care about
    WaveSculptorCanData ws_data = { 0 };
    ws_data.status_info.error_flags.raw = measurements->status.mc_error_bitset[controller] << MCI_ERROR_OFFSET;
    ws_data.status_info.limit_flags.raw = measurements->status.mc_limit_bitset[controller] << MCI_LIMIT_OFFSET;
    
    // Will get updated on status message broadcast
    s_test_fan_fault_bitset = measurements->status.board_fault_bitset;
    // TODO: update the overtemp bitset if needed
    
    can_data.raw = ws_data.raw;
  }

  GenericCanMsg msg = {
    .id = s_frame_id_map[controller * NUM_TEST_MCI_MESSAGES + message_type],
    .dlc = sizeof(can_data),
    .extended = false,
  };
  memcpy(&msg.data, &can_data, sizeof(can_data));

  // Mock MCP2515 message rx from WaveSculptor
  s_broadcast_storage.motor_can->rx_cb(msg.id, msg.extended, msg.data, msg.dlc,
                                       s_broadcast_storage.motor_can->context);
}

static void prv_setup_system_can(void) {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = MCI_CAN_EVENT_RX,
    .tx_event = MCI_CAN_EVENT_TX,
    .fault_event = MCI_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_can_storage, &can_settings);
}

static void prv_assert_num_broadcasts(uint16_t broadcasts) {
  for (uint16_t i = 0; i < broadcasts; i++) {
    MS_TEST_HELPER_CAN_TX(MCI_CAN_EVENT_TX);
  }
  for (uint16_t i = 0; i < broadcasts; i++) {
    MS_TEST_HELPER_CAN_RX(MCI_CAN_EVENT_RX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Helper functions to check measurements stored in storage against expected measurements:
// Bus voltage
static void prv_assert_eq_expected_storage_bv(MotorControllerMeasurements expected_measurements,
                                              MotorController controller) {
  TEST_ASSERT_EQUAL(
      (uint16_t)expected_measurements.bus_measurements[controller].bus_voltage_v,
      (uint16_t)s_broadcast_storage.measurements.bus_measurements[controller].bus_voltage_v);
}

// Bus current
static void prv_assert_eq_expected_storage_bi(MotorControllerMeasurements expected_measurements,
                                              MotorController controller) {
  TEST_ASSERT_EQUAL(
      (uint16_t)expected_measurements.bus_measurements[controller].bus_current_a,
      (uint16_t)s_broadcast_storage.measurements.bus_measurements[controller].bus_current_a);
}

// Velocity
static void prv_assert_eq_expected_storage_vel(MotorControllerMeasurements expected_measurements,
                                               MotorController controller) {
  // Need to convert expected measurement here -- it's in M, but values are stored in CM
  TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[controller] * M_TO_CM_CONV),
                    (uint16_t)s_broadcast_storage.measurements.vehicle_velocity[controller]);
}

// // Status
// static void prv_assert_eq_expected_storage_status(MotorControllerMeasurements expected_measurements,
//                                                   MotorController controller) {
//   TEST_ASSERT_EQUAL(expected_measurements.status.mc_limit_bitset[controller], s_broadcast_storage.measurements.status.mc_limit_bitset[controller]);
//   TEST_ASSERT_EQUAL(expected_measurements.status.mc_error_bitset[controller], s_broadcast_storage.measurements.status.mc_error_bitset[controller]);
//   TEST_ASSERT_EQUAL(expected_measurements.status.board_fault_bitset, s_broadcast_storage.measurements.status.board_fault_bitset);
//   TEST_ASSERT_EQUAL(expected_measurements.status.mc_overtemp_bitset, s_broadcast_storage.measurements.status.mc_overtemp_bitset);
// }

// Use macros so we know what line they fail on
#define ASSERT_EQ_STATUS_EXPECTED(expected_measurements, controller) do { \
  TEST_ASSERT_EQUAL(expected_measurements.status.mc_limit_bitset[controller], s_broadcast_storage.measurements.status.mc_limit_bitset[controller]); \
  TEST_ASSERT_EQUAL(expected_measurements.status.mc_error_bitset[controller], s_broadcast_storage.measurements.status.mc_error_bitset[controller]); \
  TEST_ASSERT_EQUAL(expected_measurements.status.board_fault_bitset, s_broadcast_storage.measurements.status.board_fault_bitset); \
  TEST_ASSERT_EQUAL(expected_measurements.status.mc_overtemp_bitset, s_broadcast_storage.measurements.status.mc_overtemp_bitset); \
} while(0)

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  prv_setup_system_can();

  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_VC,
                                         prv_handle_bus_measurement, NULL));
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY, prv_handle_velocity, NULL));

  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_STATUS, prv_handle_status, NULL));
}

void teardown_test(void) {
  s_received_velocity = false;
  s_received_bus_measurement = false;
  s_received_status = false;
  memset(&s_test_measurements, 0, sizeof(s_test_measurements));
  memset(&s_broadcast_storage, 0, sizeof(s_broadcast_storage));
}

// lv - left velocity
// rv - right velocity
// lb - left bus measurement
// rb - right bus measurement

// Test 1: General broadcast flow.
void test_left_all_right_all(void) {
  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 12.345,
                    .bus_current_a = 5.9876,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 4.8602,
                    .bus_current_a = 1.3975,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 1.0101,
            [RIGHT_MOTOR_CONTROLLER] = 56.5665,
        },
    .status = {
      // Note that, due to the reserved bit 0, error bitsets can't have their MSB high
      .mc_limit_bitset[LEFT_MOTOR_CONTROLLER] = 0x11,
      .mc_limit_bitset[RIGHT_MOTOR_CONTROLLER] = 0x13,
      .mc_error_bitset[LEFT_MOTOR_CONTROLLER] = 0x32,
      .mc_error_bitset[RIGHT_MOTOR_CONTROLLER] = 0x5F,
      .board_fault_bitset = 0xCA,
      .mc_overtemp_bitset = 0x00, // currently always 0
    }
  };

  // need to send in this order because of how the filter works
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);

  delay_ms(2 * MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);

  // Velocity + bus measurement + status
  prv_assert_num_broadcasts(3);
  TEST_ASSERT_TRUE(s_received_velocity);
  TEST_ASSERT_TRUE(s_received_bus_measurement);
  TEST_ASSERT_TRUE(s_received_status);
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_voltage_v,
                      s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_current_a,
                      s_test_measurements.bus_measurements[motor_id].bus_current_a);
    TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[motor_id] * 100),
                      s_test_measurements.vehicle_velocity[motor_id]);
    ASSERT_EQ_STATUS_EXPECTED(expected_measurements, motor_id);
  }
}

// Test 2: Only send left side measurements and check no output
void test_left_all_right_none(void) {
  // motor_can tx and rx should happen immedietly

  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 12.345,
                    .bus_current_a = 5.9876,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 4.8602,
                    .bus_current_a = 1.3975,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 1.0101,
            [RIGHT_MOTOR_CONTROLLER] = 56.5665,
        },
    .status = {
      // Note that, due to the reserved bit 0, error bitsets can't have their MSB high
      .mc_limit_bitset[LEFT_MOTOR_CONTROLLER] = 0x11,
      .mc_limit_bitset[RIGHT_MOTOR_CONTROLLER] = 0x13,
      .mc_error_bitset[LEFT_MOTOR_CONTROLLER] = 0x32,
      .mc_error_bitset[RIGHT_MOTOR_CONTROLLER] = 0x5F,
      .board_fault_bitset = 0xCA,
      .mc_overtemp_bitset = 0x00, // currently always 0
    }
  };

  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  TEST_ASSERT_FALSE(s_received_velocity);
  TEST_ASSERT_FALSE(s_received_bus_measurement);
  TEST_ASSERT_FALSE(s_received_status);

  // Make sure left measurements stored correctly still
  prv_assert_eq_expected_storage_bv(expected_measurements, LEFT_MOTOR_CONTROLLER);
  prv_assert_eq_expected_storage_bi(expected_measurements, LEFT_MOTOR_CONTROLLER);
  prv_assert_eq_expected_storage_vel(expected_measurements, LEFT_MOTOR_CONTROLLER);
  ASSERT_EQ_STATUS_EXPECTED(expected_measurements, LEFT_MOTOR_CONTROLLER);
}

// Test 3: Send left all + right status and check that only status outputs
void test_left_all_right_status(void) {
  // motor_can tx and rx should happen immedietly

  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 12.345,
                    .bus_current_a = 5.9876,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 4.8602,
                    .bus_current_a = 1.3975,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 0,
            [RIGHT_MOTOR_CONTROLLER] = 0,
        },

    .status = {
      // Note that, due to the reserved bit 0, error bitsets can't have their MSB high
      .mc_limit_bitset[LEFT_MOTOR_CONTROLLER] = 0x11,
      .mc_limit_bitset[RIGHT_MOTOR_CONTROLLER] = 0x13,
      .mc_error_bitset[LEFT_MOTOR_CONTROLLER] = 0x32,
      .mc_error_bitset[RIGHT_MOTOR_CONTROLLER] = 0x5F,
      .board_fault_bitset = 0xCA,
      .mc_overtemp_bitset = 0x00, // currently always 0
    }
  };

  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  prv_assert_num_broadcasts(1);

  TEST_ASSERT_FALSE(s_received_velocity);
  TEST_ASSERT_FALSE(s_received_bus_measurement);
  TEST_ASSERT_TRUE(s_received_status);
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    ASSERT_EQ_STATUS_EXPECTED(expected_measurements, motor_id);
  }
}

// Test 4: Send left all + right up to bus measurement
// and check that only status + bus measurement output
void test_left_all_right_status_bus(void) {
  // motor_can tx and rx should happen immedietly

  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 12.345,
                    .bus_current_a = 5.9876,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 4.8602,
                    .bus_current_a = 1.3975,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 0,
            [RIGHT_MOTOR_CONTROLLER] = 0,
        },

    .status = {
      // Note that, due to the reserved bit 0, error bitsets can't have their MSB high
      .mc_limit_bitset[LEFT_MOTOR_CONTROLLER] = 0x11,
      .mc_limit_bitset[RIGHT_MOTOR_CONTROLLER] = 0x13,
      .mc_error_bitset[LEFT_MOTOR_CONTROLLER] = 0x32,
      .mc_error_bitset[RIGHT_MOTOR_CONTROLLER] = 0x5F,
      .board_fault_bitset = 0xCA,
      .mc_overtemp_bitset = 0x00, // currently always 0
    }
  };

  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  prv_assert_num_broadcasts(2);

  TEST_ASSERT_FALSE(s_received_velocity);
  TEST_ASSERT_TRUE(s_received_bus_measurement);
  TEST_ASSERT_TRUE(s_received_status);
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    ASSERT_EQ_STATUS_EXPECTED(expected_measurements, motor_id);
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_voltage_v,
                      s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_current_a,
                      s_test_measurements.bus_measurements[motor_id].bus_current_a);
  }
}

// Test 5: General broadcast flow, repeated thrice.
void test_3x_left_all_right_all(void) {
  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 12.345,
                    .bus_current_a = 5.9876,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 4.8602,
                    .bus_current_a = 1.3975,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 1.0101,
            [RIGHT_MOTOR_CONTROLLER] = 56.5665,
        },
    .status = {
      // Note that, due to the reserved bit 0, error bitsets can't have their MSB high
      .mc_limit_bitset[LEFT_MOTOR_CONTROLLER] = 0x11,
      .mc_limit_bitset[RIGHT_MOTOR_CONTROLLER] = 0x13,
      .mc_error_bitset[LEFT_MOTOR_CONTROLLER] = 0x32,
      .mc_error_bitset[RIGHT_MOTOR_CONTROLLER] = 0x5F,
      .board_fault_bitset = 0xCA,
      .mc_overtemp_bitset = 0x00, // currently always 0
    }
  };

  // Repeat full sequence 3 times
  for (uint8_t i = 0; i < 3; i++) {
    // need to send in this order because of how the filter works
    prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
    prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                          &expected_measurements);
    prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
    prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                          &expected_measurements);
    prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);
    prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
    prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                          &expected_measurements);
    prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE,
                          &expected_measurements);
    prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                          &expected_measurements);
    prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE,
                          &expected_measurements);

    delay_ms(2 * MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);

    // Velocity + bus measurement + status
    prv_assert_num_broadcasts(3);
    TEST_ASSERT_TRUE(s_received_velocity);
    TEST_ASSERT_TRUE(s_received_bus_measurement);
    TEST_ASSERT_TRUE(s_received_status);
    for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
      TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_voltage_v,
                        s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
      TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_current_a,
                        s_test_measurements.bus_measurements[motor_id].bus_current_a);
      TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[motor_id] * 100),
                        s_test_measurements.vehicle_velocity[motor_id]);
      ASSERT_EQ_STATUS_EXPECTED(expected_measurements, motor_id);
    }

    // Reset before next iteration
    s_received_velocity = false;
    s_received_bus_measurement = false;
    s_received_status = false;
  }
}

// Test 6: Same as test 1, but with broadcasts with the wrong ID sprinkled in.
void test_message_id_filter(void) {
  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);
  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 12.345,
                    .bus_current_a = 5.9876,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 4.8602,
                    .bus_current_a = 1.3975,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 1.0101,
            [RIGHT_MOTOR_CONTROLLER] = 56.5665,
        },
    .status = {
      // Note that, due to the reserved bit 0, error bitsets can't have their MSB high
      .mc_limit_bitset[LEFT_MOTOR_CONTROLLER] = 0x11,
      .mc_limit_bitset[RIGHT_MOTOR_CONTROLLER] = 0x13,
      .mc_error_bitset[LEFT_MOTOR_CONTROLLER] = 0x32,
      .mc_error_bitset[RIGHT_MOTOR_CONTROLLER] = 0x5F,
      .board_fault_bitset = 0xCA,
      .mc_overtemp_bitset = 0x00, // currently always 0
    }
  };

  // Should skip
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);

  // Should process and store
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  ASSERT_EQ_STATUS_EXPECTED(expected_measurements, LEFT_MOTOR_CONTROLLER);

  // Should process and store
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_assert_eq_expected_storage_bv(expected_measurements, LEFT_MOTOR_CONTROLLER);
  prv_assert_eq_expected_storage_bi(expected_measurements, LEFT_MOTOR_CONTROLLER);

  // Should skip
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);

  // Should process and store
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_assert_eq_expected_storage_vel(expected_measurements, LEFT_MOTOR_CONTROLLER);

  // Should now skip
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  // Should still skip
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  // Should process
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE, &expected_measurements);
  // Should skip
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);
  // Should process
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);
  // Should skip
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);

  // Should process and store
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE, &expected_measurements);
  ASSERT_EQ_STATUS_EXPECTED(expected_measurements, RIGHT_MOTOR_CONTROLLER);

  // Should skip
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);

  // Should process and store
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_assert_eq_expected_storage_bv(expected_measurements, RIGHT_MOTOR_CONTROLLER);
  prv_assert_eq_expected_storage_bi(expected_measurements, RIGHT_MOTOR_CONTROLLER);

  // Should process and store
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_assert_eq_expected_storage_vel(expected_measurements, RIGHT_MOTOR_CONTROLLER);

  // Should process
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);
  // Should process
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE, &expected_measurements);

  delay_ms(2 * MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);

  // Velocity + bus measurement + status
  prv_assert_num_broadcasts(3);
  TEST_ASSERT_TRUE(s_received_velocity);
  TEST_ASSERT_TRUE(s_received_bus_measurement);
  TEST_ASSERT_TRUE(s_received_status);
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_voltage_v,
                      s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_current_a,
                      s_test_measurements.bus_measurements[motor_id].bus_current_a);
    TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[motor_id] * 100),
                      s_test_measurements.vehicle_velocity[motor_id]);
    ASSERT_EQ_STATUS_EXPECTED(expected_measurements, motor_id);
  }

  // Stored measurements should be unchanged
  for (uint8_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    prv_assert_eq_expected_storage_bv(expected_measurements, i);
    prv_assert_eq_expected_storage_bi(expected_measurements, i);
    prv_assert_eq_expected_storage_vel(expected_measurements, i);
    ASSERT_EQ_STATUS_EXPECTED(expected_measurements, i);
  }
}
