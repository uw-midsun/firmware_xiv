#include "mci_broadcast.h"

#include <inttypes.h>

#include "can_transmit.h"
#include "can_unpack.h"
#include "critical_section.h"
#include "mcp2515.h"

#include "cruise_rx.h"
#include "log.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "soft_timer.h"

#define M_TO_CM_CONV 100

static const uint16_t MOTOR_CONTROLLER_BROADCAST_MEASUREMENT_OFFSET_LOOKUP
    [NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS] = {
      [MOTOR_CONTROLLER_BROADCAST_STATUS] = WAVESCULPTOR_MEASUREMENT_ID_STATUS,
      [MOTOR_CONTROLLER_BROADCAST_BUS] = WAVESCULPTOR_MEASUREMENT_ID_BUS,
      [MOTOR_CONTROLLER_BROADCAST_VELOCITY] = WAVESCULPTOR_MEASUREMENT_ID_VELOCITY,
      [MOTOR_CONTROLLER_BROADCAST_MOTOR_TEMP] = WAVESCULPTOR_MEASUREMENT_ID_SINK_MOTOR_TEMPERATURE,
      [MOTOR_CONTROLLER_BROADCAST_DSP_TEMP] = WAVESCULPTOR_MEASUREMENT_ID_AIR_IN_CPU_TEMPERATURE
    };

// Uncomment when testing with only the left motor controller
// #define RIGHT_MOTOR_CONTROLLER_UNUSED

#ifdef RIGHT_MOTOR_CONTROLLER_UNUSED
#define NUM_MOTOR_CONTROLLERS 1
#endif

static void prv_broadcast_speed(MotorControllerBroadcastStorage *storage) {
  float *measurements = storage->measurements.vehicle_velocity;
  CAN_TRANSMIT_MOTOR_VELOCITY((uint16_t)measurements[LEFT_MOTOR_CONTROLLER],
                              (uint16_t)measurements[RIGHT_MOTOR_CONTROLLER]);
}

static void prv_broadcast_bus_measurement(MotorControllerBroadcastStorage *storage) {
  WaveSculptorBusMeasurement *measurements = storage->measurements.bus_measurements;
  CAN_TRANSMIT_MOTOR_CONTROLLER_VC((uint16_t)measurements[LEFT_MOTOR_CONTROLLER].bus_voltage_v,
                                   (uint16_t)measurements[LEFT_MOTOR_CONTROLLER].bus_current_a,
                                   (uint16_t)measurements[RIGHT_MOTOR_CONTROLLER].bus_voltage_v,
                                   (uint16_t)measurements[RIGHT_MOTOR_CONTROLLER].bus_current_a);
}

static void prv_broadcast_status(MotorControllerBroadcastStorage *storage) {
  uint32_t *measurements = storage->measurements.status;
  CAN_TRANSMIT_MOTOR_STATUS(measurements[LEFT_MOTOR_CONTROLLER],
                            measurements[RIGHT_MOTOR_CONTROLLER]);
}

// Change the MCP2515 filter to filter for the next ID to look for
static void prv_change_filter(MotorControllerBroadcastStorage *storage) {
  if (storage->cb_storage.cur_measurement == NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS - 1) {
    storage->cb_storage.cur_measurement = MOTOR_CONTROLLER_BROADCAST_STATUS;
#ifndef RIGHT_MOTOR_CONTROLLER_UNUSED
    storage->cb_storage.motor_controller = !storage->cb_storage.motor_controller;
#endif
  } else {
    storage->cb_storage.cur_measurement++;
  }
  uint32_t filter =
      (uint32_t)MOTOR_CONTROLLER_BASE_ADDR_LOOKUP(storage->cb_storage.motor_controller) +
      (uint32_t)
          MOTOR_CONTROLLER_BROADCAST_MEASUREMENT_OFFSET_LOOKUP[storage->cb_storage.cur_measurement];
  // MCP2515 requires both filters to be set, so just use the same one twice
  // Looking for multiple message IDs seems to cause issues, so we iterate through all IDs required
  // one by one
  LOG_DEBUG("Now filtering for ID 0x%x\n", (int)filter);
  uint32_t filters[2] = { filter, filter };
  mcp2515_set_filter(storage->motor_can, filters);
}

// CB for rx received
static void prv_process_rx(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  MotorControllerBroadcastStorage *storage = context;
  LOG_DEBUG("Received rx from id: 0x%x\n", (int)id);
  LOG_DEBUG("Data: 0x%" PRIx64 "\n", data);
  // calculate the base offset
  uint32_t cur_mc_id = (storage->cb_storage.motor_controller == LEFT_MOTOR_CONTROLLER
                            ? LEFT_MOTOR_CONTROLLER_BASE_ADDR
                            : RIGHT_MOTOR_CONTROLLER_BASE_ADDR);
  uint32_t offset = (id - cur_mc_id);
  // map base offset to the cb array index
  uint32_t cb_index = NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS;
  for (uint32_t i = 0; i < NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS; i++) {
    if (MOTOR_CONTROLLER_BROADCAST_MEASUREMENT_OFFSET_LOOKUP[i] == offset) {
      cb_index = i;
      break;
    }
  }
  // check if received message ID doesn't have a CB index
  if (cb_index == NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS) {
    LOG_WARN("WARNING - no cb for message ID 0x%x or received message from wrong MC\n", (int)id);
    // TODO(SOFT-139): error handling here
    // This should NEVER happen, so it may make sense to throw an error
    // However, have had this happen a few times, always with offset 9 (voltage rail measurement)
    // not sure what the cause is, doesn't happen often
    return;
  }

  // Only call CB if it exists
  if (storage->cb_storage.callbacks[cb_index] != NULL) {
    GenericCanMsg msg = {
      .id = id,
      .extended = extended,
      .data = data,
      .dlc = dlc,
    };
    storage->cb_storage.callbacks[cb_index](&msg, context);
  }

  // Check if we received the ID we're looking for
  // Return early if not so we keep looking for that ID until we get it
  if (cb_index != storage->cb_storage.cur_measurement) {
    uint32_t id_lf =
        cur_mc_id +
        MOTOR_CONTROLLER_BROADCAST_MEASUREMENT_OFFSET_LOOKUP[storage->cb_storage.cur_measurement];
    LOG_WARN("WARNING - filtering for ID 0x%x but processed 0x%x\n", (int)id_lf, (int)id);
    // TODO(SOFT-139): similar error handling to above
    return;
  }

  // Change to filter for next message
  prv_change_filter(storage);
}

static void prv_handle_status_rx(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("Received status message\n");
  MotorControllerBroadcastStorage *storage = context;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  // Cast out upper 32 bits so we can broadcast both status in one CAN message
  uint32_t status = (uint32_t)(can_data.raw);

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      bool disabled = critical_section_start();
      storage->measurements.status[motor_id] = status;
      storage->status_rx_bitset |= 1 << motor_id;
      critical_section_end(disabled);
      break;
    }
  }
}

static void prv_handle_speed_rx(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("Received speed message\n");
  MotorControllerBroadcastStorage *storage = context;
  float *measurements = storage->measurements.vehicle_velocity;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      bool disabled = critical_section_start();
      measurements[motor_id] = can_data.velocity_measurement.vehicle_velocity_ms * M_TO_CM_CONV;
      storage->velocity_rx_bitset |= 1 << motor_id;
      critical_section_end(disabled);
      if (motor_id == LEFT_MOTOR_CONTROLLER) {
        cruise_rx_update_velocity(can_data.velocity_measurement.vehicle_velocity_ms);
      }
      break;
    }
  }
}

static void prv_handle_bus_measurement_rx(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("Received bus measurement message\n");
  MotorControllerBroadcastStorage *storage = context;
  WaveSculptorBusMeasurement *measurements = storage->measurements.bus_measurements;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      bool disabled = critical_section_start();
      measurements[motor_id] = can_data.bus_measurement;
      storage->bus_rx_bitset |= 1 << motor_id;
      critical_section_end(disabled);
    }
  }
}

static void prv_handle_motor_temp_rx(const GenericCanMsg *msg, void *context) {
  // TODO(SOFT-421): send this over CAN and store it
  LOG_DEBUG("Received motor temp message\n");
}

static void prv_handle_dsp_temp_rx(const GenericCanMsg *msg, void *context) {
  // TODO(SOFT-421): send this over CAN and store it
  LOG_DEBUG("Received DSP temp message\n");
}

static void prv_setup_motor_can(MotorControllerBroadcastStorage *storage) {
  // Set up callbacks
  storage->cb_storage.callbacks[MOTOR_CONTROLLER_BROADCAST_STATUS] = prv_handle_status_rx;
  storage->cb_storage.callbacks[MOTOR_CONTROLLER_BROADCAST_BUS] = prv_handle_bus_measurement_rx;
  storage->cb_storage.callbacks[MOTOR_CONTROLLER_BROADCAST_VELOCITY] = prv_handle_speed_rx;
  storage->cb_storage.callbacks[MOTOR_CONTROLLER_BROADCAST_MOTOR_TEMP] = prv_handle_motor_temp_rx;
  storage->cb_storage.callbacks[MOTOR_CONTROLLER_BROADCAST_DSP_TEMP] = prv_handle_dsp_temp_rx;

  storage->cb_storage.cur_measurement = MOTOR_CONTROLLER_BROADCAST_STATUS;
  storage->cb_storage.motor_controller = LEFT_MOTOR_CONTROLLER;

  // Initial message ID to filter for
  uint32_t filter =
      (uint32_t)MOTOR_CONTROLLER_BASE_ADDR_LOOKUP(storage->cb_storage.motor_controller) +
      (uint32_t)
          MOTOR_CONTROLLER_BROADCAST_MEASUREMENT_OFFSET_LOOKUP[storage->cb_storage.cur_measurement];

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
    .filters =
        {
            [MCP2515_FILTER_ID_RXF0] = { .raw = filter },

            [MCP2515_FILTER_ID_RXF1] = { .raw = filter },
        },
  };
  mcp2515_init(storage->motor_can, &mcp2515_settings);
  mcp2515_register_cbs(storage->motor_can, prv_process_rx, NULL, storage);
}

static void prv_periodic_broadcast_tx(SoftTimerId timer_id, void *context) {
  MotorControllerBroadcastStorage *storage = context;
  if (storage->velocity_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received speed from all motor controllers - clear bitset and broadcast
    storage->velocity_rx_bitset = 0;
    LOG_DEBUG("Sending velocity periodic broadcast\n");
    prv_broadcast_speed(storage);
  }
  if (storage->bus_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received bus measurements from all motor controllers - clear bitset and broadcast
    storage->bus_rx_bitset = 0;
    LOG_DEBUG("Sending bus periodic broadcast\n");
    prv_broadcast_bus_measurement(storage);
  }
  if (storage->status_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received status info from all motor controllers - clear bitset and broadcast
    storage->status_rx_bitset = 0;
    LOG_DEBUG("Sending status periodic broadcast\n");
    prv_broadcast_status(storage);
  }
  // TODO(SOFT-421): broadcast temp measurements
  soft_timer_start_millis(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS, prv_periodic_broadcast_tx,
                          storage, NULL);
}

StatusCode mci_broadcast_init(MotorControllerBroadcastStorage *storage,
                              MotorControllerBroadcastSettings *settings) {
  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    storage->ids[motor_id] = settings->device_ids[motor_id];
  }
  storage->velocity_rx_bitset = 0;
  storage->bus_rx_bitset = 0;
  storage->motor_can = settings->motor_can;
  prv_setup_motor_can(storage);
  return soft_timer_start_millis(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS, prv_periodic_broadcast_tx,
                                 storage, NULL);
  return STATUS_CODE_OK;
}
