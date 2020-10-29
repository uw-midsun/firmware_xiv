#include "mci_broadcast.h"

#include "can_transmit.h"
#include "can_unpack.h"
#include "critical_section.h"
#include "cruise_rx.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "soft_timer.h"
#include "mcp2515.h"

#include "log.h"
#include "cruise_rx.h"
#include "motor_can.h"
#include "motor_controller.h"
#include "soft_timer.h"

#define M_TO_CM_CONV 100

static Mcp2515Storage s_mcp2515_storage;

static MotorControllerCallbackStorage s_cb_storage;

// Uncomment to test with only one motor controller
//#define RIGHT_MOTOR_CONTROLLER_UNUSED

// get rid of local mcp2515 stuff
//#define TESTING

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

#ifndef TESTING
// Change the MCP2515 filter to filter for the next ID to look for
static void prv_change_filter(void) {
  if(s_cb_storage.offset == NUM_MCI_BROADCAST_MEASUREMENTS) {
    s_cb_storage.offset = MCI_BROADCAST_STATUS;
    #ifndef RIGHT_MOTOR_CONTROLLER_UNUSED 
      s_cb_storage.motor_controller = !s_cb_storage.motor_controller;
    #endif
  } else {
    s_cb_storage.offset++;
  }
  uint32_t filter = (uint32_t)motor_controller_base_addr_lookup(s_cb_storage.motor_controller) + s_cb_storage.offset;
  LOG_DEBUG("Changing filter to %x\n", (int)filter);
  uint32_t filters[2] = {MCI_ID_UNUSED, filter};
  LOG_DEBUG("Change filter result %d\n", mcp2515_set_filter(&s_mcp2515_storage, filters));
}

// CB for rx received
static void prv_process_rx(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  LOG_DEBUG("real prv process rx\n");
  LOG_DEBUG("received rx from id: %d\n", (int)id);
  LOG_DEBUG("Data: 0x%x%x\n", (int)data, (int)(data >> 32));
  // this is kinda weird since the enum starts at 1, figure out a better way of doing
  uint32_t cb_offset = s_cb_storage.motor_controller + s_cb_storage.offset - MCI_BROADCAST_MEASUREMENT_OFFSET;
  

  // Only call CB if it exists
  if(s_cb_storage.callbacks[cb_offset] != NULL) {
    // consider reworking so we don't need to use this
    GenericCanMsg msg = {
      .id = id, 
      .extended = extended, 
      .data = data, 
      .dlc = dlc,
    };
    s_cb_storage.callbacks[cb_offset](&msg, context);
  }
  // Change to filter for next message
  prv_change_filter();
}

static void prv_handle_status_rx(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("got status message\n");
}
static void prv_handle_speed_rx(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("got speed message\n");
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
  LOG_DEBUG("got bus measurement message\n");
  MotorControllerBroadcastStorage *storage = context;
  WaveSculptorBusMeasurement *measurements = storage->measurements.bus_measurements;

  WaveSculptorCanId can_id = { .raw = msg->id };
  WaveSculptorCanData can_data = { .raw = msg->data };

  for (size_t motor_id = 0; motor_id < NUM_MOTOR_CONTROLLERS; motor_id++) {
    LOG_DEBUG("can device id %d storage device id %d\n", (int)(can_id.device_id), (int)(storage->ids[motor_id]));
    if (can_id.device_id == storage->ids[motor_id]) {
      LOG_DEBUG("logging bus rx from motor id %d\n", (int)motor_id);
      bool disabled = critical_section_start();
      measurements[motor_id] = can_data.bus_measurement;
      storage->bus_rx_bitset |= 1 << motor_id;
      critical_section_end(disabled);
    }
  }
}

static void prv_setup_motor_can(MotorControllerBroadcastStorage *storage) {
  // Set up callback storage
  if(s_cb_storage.callbacks[MCI_BROADCAST_STATUS - MCI_BROADCAST_MEASUREMENT_OFFSET] == NULL) {
    s_cb_storage.callbacks[MCI_BROADCAST_STATUS - MCI_BROADCAST_MEASUREMENT_OFFSET] = prv_handle_status_rx;
  }
  if(s_cb_storage.callbacks[MCI_BROADCAST_BUS - MCI_BROADCAST_MEASUREMENT_OFFSET] == NULL) {
    s_cb_storage.callbacks[MCI_BROADCAST_BUS - MCI_BROADCAST_MEASUREMENT_OFFSET] = prv_handle_bus_measurement_rx; 
  }
  if(s_cb_storage.callbacks[MCI_BROADCAST_VELOCITY - MCI_BROADCAST_MEASUREMENT_OFFSET] == NULL) {
    s_cb_storage.callbacks[MCI_BROADCAST_VELOCITY - MCI_BROADCAST_MEASUREMENT_OFFSET] = prv_handle_speed_rx; 
  }

  s_cb_storage.offset = MCI_BROADCAST_STATUS;
  s_cb_storage.motor_controller = LEFT_MOTOR_CONTROLLER;
  
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
    .filters = {
      [MCP2515_FILTER_ID_RXF0] = {.raw = MCI_ID_UNUSED}, // only want to use one filter
      [MCP2515_FILTER_ID_RXF1] = {.raw = (uint32_t)(LEFT_MOTOR_CONTROLLER_BASE_ADDR + s_cb_storage.offset)},
    },
  };

  mcp2515_init(&s_mcp2515_storage, &mcp2515_settings);
  mcp2515_register_cbs(&s_mcp2515_storage, prv_process_rx, NULL, storage);
}
#endif

static void prv_periodic_broadcast_tx(SoftTimerId timer_id, void *context) {
  MotorControllerBroadcastStorage *storage = context;
  LOG_DEBUG("in here\n");
  if (storage->velocity_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received speed from all motor controllers - clear bitset and broadcast
    storage->velocity_rx_bitset = 0;
    prv_broadcast_speed(storage);
  }
  if (storage->bus_rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received bus measurements from all motor controllers - clear bitset and broadcast
    storage->bus_rx_bitset = 0;
    prv_broadcast_bus_measurement(storage);
  }
  LOG_DEBUG("out\n");
  // TODO(SOFT-139): Send status messages here as well
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
  #ifndef TESTING
    prv_setup_motor_can(storage);
  #endif
  return soft_timer_start_millis(MOTOR_CONTROLLER_BROADCAST_TX_PERIOD_MS, prv_periodic_broadcast_tx,
                                 storage, NULL);
  return STATUS_CODE_OK;

}
