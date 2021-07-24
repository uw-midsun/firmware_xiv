#include "mci_broadcast.h"

#include <inttypes.h>

#include "can_transmit.h"
#include "can_unpack.h"
#include "critical_section.h"
#include "mcp2515.h"

#include "controller_board_pins.h"
#include "cruise_rx.h"
#include "log.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "soft_timer.h"
#include "mci_fan_control.h"

#define M_TO_CM_CONV 100

static const uint32_t s_base_addr_lookup[NUM_MOTOR_CONTROLLERS] = {
  [LEFT_MOTOR_CONTROLLER] = LEFT_MOTOR_CONTROLLER_BASE_ADDR,
  [RIGHT_MOTOR_CONTROLLER] = RIGHT_MOTOR_CONTROLLER_BASE_ADDR,
};

static const uint32_t s_offset_lookup[NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS] = {
  [MOTOR_CONTROLLER_BROADCAST_STATUS] = WAVESCULPTOR_MEASUREMENT_ID_STATUS,
  [MOTOR_CONTROLLER_BROADCAST_BUS] = WAVESCULPTOR_MEASUREMENT_ID_BUS,
  [MOTOR_CONTROLLER_BROADCAST_VELOCITY] = WAVESCULPTOR_MEASUREMENT_ID_VELOCITY,
  [MOTOR_CONTROLLER_BROADCAST_MOTOR_TEMP] = WAVESCULPTOR_MEASUREMENT_ID_SINK_MOTOR_TEMPERATURE,
  [MOTOR_CONTROLLER_BROADCAST_DSP_TEMP] = WAVESCULPTOR_MEASUREMENT_ID_DSP_BOARD_TEMPERATURE,
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
  mci_status_update_fan_fault(mci_fan_get_fault_bitset());
  LOG_DEBUG("fan fault updated to %d\n", mci_fan_get_fault_bitset());
  mci_status_update_over_temp(0); // Currently unimplemented
  storage->measurements.status = mci_status_get_message(); // REALLY BAD JUST REWORK THE FUNCTIONS

  MciStatusMessage message = mci_status_get_message();
  
  CAN_TRANSMIT_MOTOR_STATUS(message.mc_limit_bitset[LEFT_MOTOR_CONTROLLER], message.mc_limit_bitset[RIGHT_MOTOR_CONTROLLER],
  message.mc_error_bitset[LEFT_MOTOR_CONTROLLER], message.mc_error_bitset[RIGHT_MOTOR_CONTROLLER], message.board_fault_bitset, 
  message.mc_overtemp_bitset);
}

static void prv_handle_status_rx(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("Received status message\n");
  MotorControllerBroadcastStorage *storage = context;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    if (can_id.device_id == storage->ids[motor_id]) {
      bool disabled = critical_section_start();
      // Update status message
      mci_status_update_mc_flags(&can_data.status_info, motor_id);
      storage->measurements.status = mci_status_get_message(); // TODO FIX THIS, JUST PASS IT INTO THE FUNC
      // Update the bitset here since all other status message fields
      // are updated just prior to broadcasting
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

static MotorControllerMeasurementCallback
    s_cb_storage[NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS] = {
      [MOTOR_CONTROLLER_BROADCAST_STATUS] = prv_handle_status_rx,
      [MOTOR_CONTROLLER_BROADCAST_BUS] = prv_handle_bus_measurement_rx,
      [MOTOR_CONTROLLER_BROADCAST_VELOCITY] = prv_handle_speed_rx,
      [MOTOR_CONTROLLER_BROADCAST_MOTOR_TEMP] = prv_handle_motor_temp_rx,
      [MOTOR_CONTROLLER_BROADCAST_DSP_TEMP] = prv_handle_dsp_temp_rx,
    };

// Change the MCP2515 filter to filter for the next ID to look for
static void prv_change_filter(MotorControllerBroadcastStorage *storage) {
  if (storage->filter_measurement == NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS - 1) {
    storage->filter_measurement = MOTOR_CONTROLLER_BROADCAST_STATUS;
#ifndef RIGHT_MOTOR_CONTROLLER_UNUSED
    storage->filter_mc = !storage->filter_mc;
#endif
  } else {
    storage->filter_measurement++;
  }
  uint32_t filter =
      s_base_addr_lookup[storage->filter_mc] + s_offset_lookup[storage->filter_measurement];
  // MCP2515 requires both filters to be set, so just use the same one twice
  // Looking for multiple message IDs seems to cause issues, so we iterate through all IDs required
  // one by one
  LOG_DEBUG("Now filtering for ID 0x%" PRIx32 "\n", filter);
  Mcp2515Id filters[NUM_MCP2515_FILTER_IDS] = {
    { .raw = filter },
    { .raw = filter },
  };

  // Not in loopback mode
  mcp2515_set_filter(storage->motor_can, filters, false);
}

// CB for rx received
static void prv_process_rx(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  MotorControllerBroadcastStorage *storage = context;
  LOG_DEBUG("Received rx from id: 0x%" PRIx32 "\n", id);
  LOG_DEBUG("Data: 0x%" PRIx64 "\n", data);
  // calculate the base offset
  uint32_t cur_mc_id = s_base_addr_lookup[storage->filter_mc];
  uint32_t offset = (id - cur_mc_id);
  // map base offset to the cb array index
  uint32_t cb_index = NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS;
  for (uint32_t i = 0; i < NUM_MOTOR_CONTROLLER_BROADCAST_MEASUREMENTS; i++) {
    if (s_offset_lookup[i] == offset) {
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
  if (s_cb_storage[cb_index] != NULL) {
    GenericCanMsg msg = {
      .id = id,
      .extended = extended,
      .data = data,
      .dlc = dlc,
    };
    s_cb_storage[cb_index](&msg, context);
  }

  // Check if we received the ID we're looking for
  // Return early if not so we keep looking for that ID until we get it
  if (cb_index != storage->filter_measurement) {
    uint32_t id_lf = cur_mc_id + s_offset_lookup[storage->filter_measurement];
    LOG_WARN("WARNING - filtering for ID 0x%" PRIx32 " but processed 0x%" PRIx32 "\n", id_lf, id);
    // TODO(SOFT-139): similar error handling to above
    return;
  }

  // Change to filter for next message
  prv_change_filter(storage);
}

static void prv_setup_motor_can(MotorControllerBroadcastStorage *storage) {
  // Initial measurement to filter for
  storage->filter_measurement = MOTOR_CONTROLLER_BROADCAST_STATUS;
  storage->filter_mc = LEFT_MOTOR_CONTROLLER;

  // Initial message ID to filter for
  uint32_t filter =
      s_base_addr_lookup[storage->filter_mc] + s_offset_lookup[storage->filter_measurement];

  Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = CONTROLLER_BOARD_ADDR_SPI2_MOSI,
    .miso = CONTROLLER_BOARD_ADDR_SPI2_MISO,
    .sclk = CONTROLLER_BOARD_ADDR_SPI2_SCK,
    .cs = CONTROLLER_BOARD_ADDR_SPI2_NSS,
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
}
