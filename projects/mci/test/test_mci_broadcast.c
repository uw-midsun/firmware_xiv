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
#include "mcp2515.h"
#include "soft_timer.h"
#include "status.h"
#include "log.h"

#include "mci_events.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "wavesculptor.h"

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

static GenericCanMcp2515 s_motor_can;
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
  uint32_t status[NUM_MOTOR_CONTROLLERS];
} TestMotorControllerMeasurements;

static TestMotorControllerMeasurements s_test_measurements = { 0 };

static MotorCanFrameId s_frame_id_map[] = {
  [LEFT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_VELOCITY_MESSAGE] =
      MOTOR_CAN_LEFT_VELOCITY_MEASUREMENT_FRAME_ID,
  [RIGHT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_VELOCITY_MESSAGE] =
      MOTOR_CAN_RIGHT_VELOCITY_MEASUREMENT_FRAME_ID,
  [LEFT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_BUS_MEASUREMENT_MESSAGE] =
      MOTOR_CAN_LEFT_BUS_MEASUREMENT_FRAME_ID,
  [RIGHT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_BUS_MEASUREMENT_MESSAGE] =
      MOTOR_CAN_RIGHT_BUS_MEASUREMENT_FRAME_ID,
  [LEFT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_STATUS_MESSAGE] = 
      LEFT_MOTOR_CONTROLLER_BASE_ADDR + MOTOR_CONTROLLER_BROADCAST_STATUS_OFFSET,
  [RIGHT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_STATUS_MESSAGE] = 
      RIGHT_MOTOR_CONTROLLER_BASE_ADDR + MOTOR_CONTROLLER_BROADCAST_STATUS_OFFSET,
  [LEFT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_MOTOR_TEMP_MESSAGE] = 
      LEFT_MOTOR_CONTROLLER_BASE_ADDR + MOTOR_CONTROLLER_BROADCAST_MOTOR_TEMP_OFFSET,
  [RIGHT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_MOTOR_TEMP_MESSAGE] = 
      RIGHT_MOTOR_CONTROLLER_BASE_ADDR + MOTOR_CONTROLLER_BROADCAST_MOTOR_TEMP_OFFSET,
  [LEFT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_DSP_TEMP_MESSAGE] = 
      LEFT_MOTOR_CONTROLLER_BASE_ADDR + MOTOR_CONTROLLER_BROADCAST_DSP_TEMP_OFFSET,
  [RIGHT_MOTOR_CONTROLLER * NUM_TEST_MCI_MESSAGES + TEST_MCI_DSP_TEMP_MESSAGE] = 
      RIGHT_MOTOR_CONTROLLER_BASE_ADDR + MOTOR_CONTROLLER_BROADCAST_DSP_TEMP_OFFSET,
};

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

static StatusCode prv_handle_status(const CanMessage *msg, void *context,
                                    CanAckStatus *ack_reply) {
  uint32_t status_left, status_right;
  CAN_UNPACK_MOTOR_STATUS(msg, &status_left, &status_right);
  s_received_status = true;
  s_test_measurements.status[LEFT_MOTOR_CONTROLLER] = status_left;
  s_test_measurements.status[RIGHT_MOTOR_CONTROLLER] = status_right;
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
    can_data.raw = measurements->status[controller];
  }

  GenericCanMsg msg = {
    .id = s_frame_id_map[controller * NUM_TEST_MCI_MESSAGES + message_type],
    .dlc = sizeof(can_data),
    .extended = false,
  };
  memcpy(&msg.data, &can_data, sizeof(can_data));
  LOG_DEBUG("before gcantx\n");
  mcp2515_tx(&s_motor_can_storage, msg.id, msg.extended, msg.data, msg.dlc);
  // generic_can_tx((GenericCan *)&s_motor_can, &msg);
  LOG_DEBUG("after\n");
}

// Mocks what the mci_broadcast does when receiving messages through the MCP2515.
StatusCode TEST_MOCK(mcp2515_tx)(Mcp2515Storage *storage, uint32_t id, bool extended, uint64_t data,
                                 size_t dlc) {
  LOG_DEBUG("in mcp2515 tx mock\n");
  if (storage->rx_cb != NULL) {
    GenericCanMsg msg = {
      .id = id,
      .data = data,
      .extended = extended,
      .dlc = dlc,
    };
    LOG_DEBUG("before rx cb\n");
    storage->rx_cb(id, extended, data, dlc, storage->context);
    LOG_DEBUG("after rx cb \n");
    /*
    if (id == MOTOR_CAN_LEFT_VELOCITY_MEASUREMENT_FRAME_ID ||
        id == MOTOR_CAN_RIGHT_VELOCITY_MEASUREMENT_FRAME_ID) {
      s_broadcast_storage.callbacks[MOTOR_CONTROLLER_BROADCAST_VELOCITY](&msg, &s_broadcast_storage);
    } else if (id == MOTOR_CAN_LEFT_BUS_MEASUREMENT_FRAME_ID ||
               id == MOTOR_CAN_RIGHT_BUS_MEASUREMENT_FRAME_ID) {
      s_broadcast_storage.callbacks[MOTOR_CONTROLLER_BROADCAST_BUS](&msg, &s_broadcast_storage);
    }
    */
  }
  return STATUS_CODE_OK;
}

// Empty function pointer to be used with mcp2515_init
static void prv_rx_handler(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {}

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
    .loopback = true,
  };

  // Will throw unimplemented errors on x86
  generic_can_mcp2515_init(&s_motor_can, &mcp2515_settings);
  // On x86 the rx handler doesn't get registered either :|
  mcp2515_register_cbs(s_motor_can.mcp2515, prv_rx_handler, NULL, &s_motor_can);
}

static void prv_setup_system_can() {
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

/*
static void prv_assert_double_broadcast() {
  MS_TEST_HELPER_CAN_TX(MCI_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_TX(MCI_CAN_EVENT_TX);
  MS_TEST_HELPER_CAN_RX(MCI_CAN_EVENT_RX);
  MS_TEST_HELPER_CAN_RX(MCI_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
*/

static void prv_assert_num_broadcasts(uint16_t broadcasts) {
  for(uint16_t i = 0; i < broadcasts; i++) {
    MS_TEST_HELPER_CAN_TX(MCI_CAN_EVENT_TX);
  }
  for(uint16_t i = 0; i < broadcasts; i++) {
    MS_TEST_HELPER_CAN_RX(MCI_CAN_EVENT_RX);
  }
}

/*
static void prv_assert_single_broadcast() {
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
*/

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  prv_setup_system_can();
  prv_setup_motor_can();

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
void test_left_all_right_all() {
  LOG_DEBUG("start of test all\n");
  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);
  LOG_DEBUG("after broadcast init\n");
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
    .status =
        {
          [LEFT_MOTOR_CONTROLLER] = 0xDEADBEEF,
          [RIGHT_MOTOR_CONTROLLER] = 0xDEADBEEF,
        },
  };
  LOG_DEBUG("before prv send\n");
  // need to send in this order because of how the filter works
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE,
                        &expected_measurements);

  LOG_DEBUG("after, before delay\n");
  delay_ms(2 * MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  // prv_assert_num_broadcasts(1);
  // Velocity + bus measurement + status
  prv_assert_num_broadcasts(3);
  TEST_ASSERT_TRUE(s_received_velocity);
  TEST_ASSERT_TRUE(s_received_bus_measurement);
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_voltage_v,
                      s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_current_a,
                      s_test_measurements.bus_measurements[motor_id].bus_current_a);
    TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[motor_id] * 100),
                      s_test_measurements.vehicle_velocity[motor_id]);
    TEST_ASSERT_EQUAL(expected_measurements.status[motor_id], s_test_measurements.status[motor_id]);
  }
}

// Test 2: Only send left side measurements and check no output
void test_left_all_right_none() {
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
    .status =
        {
          [LEFT_MOTOR_CONTROLLER] = 0xDEADBEEF,
          [RIGHT_MOTOR_CONTROLLER] = 0xDEADBEEF,
        },
  };

  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE,
                        &expected_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  TEST_ASSERT_FALSE(s_received_velocity);
  TEST_ASSERT_FALSE(s_received_bus_measurement);
  TEST_ASSERT_FALSE(s_received_status);
  
  // Make sure left measurements stored correctly still
  TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[LEFT_MOTOR_CONTROLLER].bus_voltage_v, (uint16_t)s_broadcast_storage.measurements.bus_measurements[LEFT_MOTOR_CONTROLLER].bus_voltage_v);
  TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[LEFT_MOTOR_CONTROLLER].bus_current_a, (uint16_t)s_broadcast_storage.measurements.bus_measurements[LEFT_MOTOR_CONTROLLER].bus_current_a);
  TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[LEFT_MOTOR_CONTROLLER] * M_TO_CM_CONV), (uint16_t)s_broadcast_storage.measurements.vehicle_velocity[LEFT_MOTOR_CONTROLLER]);
  TEST_ASSERT_EQUAL((uint16_t)expected_measurements.status[LEFT_MOTOR_CONTROLLER], (uint16_t)s_broadcast_storage.measurements.status[LEFT_MOTOR_CONTROLLER]);
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

    .status =
        {
          [LEFT_MOTOR_CONTROLLER] = 0xDEADBEEF,
          [RIGHT_MOTOR_CONTROLLER] = 0xDEADBEEF,
        },
  };

  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_MOTOR_TEMP_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_DSP_TEMP_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_STATUS_MESSAGE,
                        &expected_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  prv_assert_num_broadcasts(1);

  TEST_ASSERT_FALSE(s_received_velocity);
  TEST_ASSERT_FALSE(s_received_bus_measurement);
  TEST_ASSERT_TRUE(s_received_status);
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL(expected_measurements.status[motor_id], s_test_measurements.status[motor_id]);
  }
}

// TODO tests:
// same as Test 3 but up to bus measurement
// Same as Test 1 but twice
// Send messages out of filter order and make sure they aren't processed

/*
// Test 5: rv lv (check only 1 output)
void test_velocity_measurements_rv_lv() {
  // motor_can tx and rx should happen immedietly

  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 0,
                    .bus_current_a = 0,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 0,
                    .bus_current_a = 0,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 1.0101,
            [RIGHT_MOTOR_CONTROLLER] = 56.5665,
        },
  };

  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  prv_assert_num_broadcasts(1);

  TEST_ASSERT_TRUE(s_received_velocity);
  TEST_ASSERT_FALSE(s_received_bus_measurement);
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_voltage_v,
                      s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_current_a,
                      s_test_measurements.bus_measurements[motor_id].bus_current_a);
    TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[motor_id] * 100),
                      s_test_measurements.vehicle_velocity[motor_id]);
  }
}

// Test 6: rb lv (check no output)
void test_no_measurements_rb_lv() {
  // motor_can tx and rx should happen immedietly

  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 0,
                    .bus_current_a = 0,
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
            [RIGHT_MOTOR_CONTROLLER] = 0,
        },
  };

  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE, &expected_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  TEST_ASSERT_FALSE(s_received_velocity);
  TEST_ASSERT_FALSE(s_received_bus_measurement);

  expected_measurements.bus_measurements[RIGHT_MOTOR_CONTROLLER].bus_voltage_v = 0;
  expected_measurements.bus_measurements[RIGHT_MOTOR_CONTROLLER].bus_current_a = 0;
  expected_measurements.vehicle_velocity[LEFT_MOTOR_CONTROLLER] = 0;
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_voltage_v,
                      s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL((uint16_t)expected_measurements.bus_measurements[motor_id].bus_current_a,
                      s_test_measurements.bus_measurements[motor_id].bus_current_a);
    TEST_ASSERT_EQUAL((uint16_t)(expected_measurements.vehicle_velocity[motor_id] * 100),
                      s_test_measurements.vehicle_velocity[motor_id]);
  }
}

// Test 5: rv lv (check only 1 output) rb lb (check only 1 output)
void test_all_measurements_rv_lv_then_rb_lb() {
  // motor_can tx and rx should happen immedietly

  MotorControllerBroadcastStorage broadcast_storage = { 0 };
  mci_broadcast_init(&s_broadcast_storage, &s_broadcast_settings);

  MotorControllerMeasurements expected_first_measurements = {
    .bus_measurements =
        {
            [LEFT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 0,
                    .bus_current_a = 0,
                },
            [RIGHT_MOTOR_CONTROLLER] =
                {
                    .bus_voltage_v = 0,
                    .bus_current_a = 0,
                },
        },
    .vehicle_velocity =
        {
            [LEFT_MOTOR_CONTROLLER] = 1.0101,
            [RIGHT_MOTOR_CONTROLLER] = 56.5665,
        },
  };

  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE,
                        &expected_first_measurements);
  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_VELOCITY_MESSAGE,
                        &expected_first_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  prv_assert_num_broadcasts(1);

  TEST_ASSERT_TRUE(s_received_velocity);
  TEST_ASSERT_FALSE(s_received_bus_measurement);

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL(
        (uint16_t)expected_first_measurements.bus_measurements[motor_id].bus_voltage_v,
        s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL(
        (uint16_t)expected_first_measurements.bus_measurements[motor_id].bus_current_a,
        s_test_measurements.bus_measurements[motor_id].bus_current_a);
    TEST_ASSERT_EQUAL((uint16_t)(expected_first_measurements.vehicle_velocity[motor_id] * 100),
                      s_test_measurements.vehicle_velocity[motor_id]);
  }

  s_received_velocity = false;
  s_received_bus_measurement = false;
  memset(&s_test_measurements, 0, sizeof(s_test_measurements));

  MotorControllerMeasurements expected_second_measurements = {
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
  };

  prv_send_measurements(LEFT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_second_measurements);
  prv_send_measurements(RIGHT_MOTOR_CONTROLLER, TEST_MCI_BUS_MEASUREMENT_MESSAGE,
                        &expected_second_measurements);

  delay_ms(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS + 50);
  prv_assert_num_broadcasts(1);

  TEST_ASSERT_FALSE(s_received_velocity);
  TEST_ASSERT_TRUE(s_received_bus_measurement);

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    TEST_ASSERT_EQUAL(
        (uint16_t)expected_second_measurements.bus_measurements[motor_id].bus_voltage_v,
        s_test_measurements.bus_measurements[motor_id].bus_voltage_v);
    TEST_ASSERT_EQUAL(
        (uint16_t)expected_second_measurements.bus_measurements[motor_id].bus_current_a,
        s_test_measurements.bus_measurements[motor_id].bus_current_a);
    TEST_ASSERT_EQUAL((uint16_t)(expected_second_measurements.vehicle_velocity[motor_id] * 100),
                      s_test_measurements.vehicle_velocity[motor_id]);
  }
}
*/