#include "charger_controller.h"

#include <stdbool.h>
#include <stdint.h>

#include "log.h"
#include "can_transmit.h"
#include "charger_defs.h"
#include "charger_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can_mcp2515.h"
#include "mcp2515.h"
#include "soft_timer.h"

typedef struct ChargerCanTxDataImpl {
  uint16_t max_voltage;
  uint16_t max_current;
  uint8_t charging;
  uint8_t reserved_1;
  uint8_t reserved_2;
  uint8_t reserved_3;
} ChargerCanTxDataImpl;

typedef union ChargerCanTxData {
  uint64_t raw_data;
  ChargerCanTxDataImpl data_impl;
} ChargerCanTxData;

static const ChargerCanTxData tx_data = { .data_impl = {
  .max_voltage = 1512,
  .max_current = 1224,
  .charging = 0,
  }};

// Explicit for readability.
static const ChargerCanJ1939Id s_rx_id = {
 .source_address = 0xE5,  // Indicates BMS in the J1939 standard.
 .pdu_specifics = 0x50,   // Broadcast address (BCA) id.
 .pdu_format = 0xFF,      // From datasheet.
 .dp = 1,
 .r = 0,
 .priority = 0x06,  // From datasheet.
};
static const ChargerCanJ1939Id s_tx_id = {
  .source_address = 0xF4,  // Indicates BMS in the J1939 standard.
  .pdu_specifics = 0xE5,   // Charger control system (CCS) id.
  .pdu_format = 0x06,      // From datasheet.
  .dp = 0,
  .r = 0,
  .priority = 0x06,  // From datasheet.
 };

typedef enum { TX_CONTROL_START_CHARGING = 0, TX_CONTROL_STOP_CHARGING } TxControl;

static SoftTimerId s_charger_timer_id;
static ChargerVC s_vc = {
  .values.complete.current = 0,                         //
  .values.complete.voltage = CHARGER_BATTERY_THRESHOLD  //
};
static GenericCanMcp2515 s_generic_can;

static Mcp2515Settings mcp2515_settings = {
  .spi_port = SPI_PORT_2,                        //
  .spi_baudrate = 6000000,                       //
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },    //
  .miso = { .port = GPIO_PORT_B, .pin = 14 },    //
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },    //
  .cs = { .port = GPIO_PORT_B, .pin = 12 },      //
  .int_pin = { .port = GPIO_PORT_A, .pin = 8 },  //
  .can_bitrate = MCP2515_BITRATE_250KBPS,        //
  .loopback = false,                             //
};

static TxMsgData  prv_build_charger_tx(TxControl charge) {
  TxMsgData tx_data = {
    .fields.max_voltage_high = s_vc.values.bytes.voltage_high,  //
    .fields.max_voltage_low = s_vc.values.bytes.voltage_low,    //
    .fields.max_current_high = s_vc.values.bytes.current_high,  //
    .fields.max_current_low = s_vc.values.bytes.current_low,    //
    .fields.control = charge,                                   //
  };
  return tx_data;
}

static void prv_periodic_charger_tx(SoftTimerId timer_id, void *context) {
  TxMsgData t_data = prv_build_charger_tx(TX_CONTROL_START_CHARGING);

  GenericCanMsg tx_msg = {
    //.id = CHARGER_TX_CAN_ID,    //
    .id = s_tx_id.raw_id,
    .extended = true,           //
    .data = tx_data.raw_data,        //
    .dlc = sizeof(tx_data.raw_data)  //
  };

  generic_can_tx(&s_generic_can.base, &tx_msg);
  LOG_DEBUG("SENT MESSAGE!\n");
  //LOG_DEBUG("Data: %d, %d, %d, ")
  soft_timer_start_millis(CHARGER_TX_PERIOD_MS, prv_periodic_charger_tx, NULL, &s_charger_timer_id);
}

static void prv_charger_can_rx(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("RX_CALLBACK CALLED: %ld\n", msg->id);
  if (msg->id != CHARGER_RX_CAN_ID) {
    return;
  }
  RxMsgData rx_msg = { .raw = msg->data };
  LOG_DEBUG("OUT_VOLTAGE %d, OUT_CURRENT: %d, FLAGS: %x\n", rx_msg.fields.out_voltage_high << 8 | rx_msg.fields.out_voltage_high,
      rx_msg.fields.out_current_high << 8 | rx_msg.fields.out_current_high, rx_msg.fields.status_flags);
  // if (rx_msg.fields.status_flags.fields) {
  //   charger_controller_deactivate();
  // }

  if (rx_msg.fields.status_flags & ELCON_STATUS_HW_FAULT) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_HARDWARE_FAILURE);
    LOG_DEBUG("Charger fault hardware\n");
  }
  if (rx_msg.fields.status_flags & ELCON_STATUS_OVERTEMP) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_OVER_TEMP);
    LOG_DEBUG("Charger fault overtemp\n");
  }
  if (rx_msg.fields.status_flags & ELCON_STATUS_INP_VOLTAGE_WRONG) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_WRONG_VOLTAGE);
    LOG_DEBUG("Charger fault wrong_voltage\n");
  }
  if (rx_msg.fields.status_flags & ELCON_STATUS_REVERSE_POLARITY) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_POLARITY_FAILURE);
    LOG_DEBUG("Charger fault polarity failure\n");
  }
  if (rx_msg.fields.status_flags & ELCON_STATUS_COMMS_TIMEOUT) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_COMMUNICATION_TIMEOUT);
    LOG_DEBUG("Charger fault communication target\n");
  }

  // if (rx_msg.fields.status_flags.raw) {
  //   charger_controller_deactivate();
  // }
  // if (rx_msg.fields.status_flags.flags.hardware_failure) {
  //   CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_HARDWARE_FAILURE);
  //   LOG_DEBUG("Charger fault hardware\n");
  // }
  // if (rx_msg.fields.status_flags.flags.over_temp) {
  //   CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_OVER_TEMP);
  //   LOG_DEBUG("Charger fault overtemp\n");
  // }
  // if (rx_msg.fields.status_flags.flags.wrong_voltage) {
  //   CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_WRONG_VOLTAGE);
  //   LOG_DEBUG("Charger fault wrong_voltage\n");
  // }
  // if (rx_msg.fields.status_flags.flags.polarity_failure) {
  //   CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_POLARITY_FAILURE);
  //   LOG_DEBUG("Charger fault polarity failure\n");
  // }
  // if (rx_msg.fields.status_flags.flags.communication_timeout) {
  //   CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_COMMUNICATION_TIMEOUT);
  //   LOG_DEBUG("Charger fault communication target\n");
  // }
}

StatusCode charger_controller_activate(uint16_t max_allowable_current) {
  s_vc.values.complete.current = max_allowable_current;
  prv_periodic_charger_tx(0, NULL);
  return STATUS_CODE_OK;
}

StatusCode charger_controller_deactivate() {
  s_vc.values.complete.current = 0;
  TxMsgData t_data = prv_build_charger_tx(TX_CONTROL_STOP_CHARGING);

  // Send 'stop' message
  GenericCanMsg tx_msg = {
    .id = CHARGER_TX_CAN_ID,    //
    .extended = true,           //
    .data = tx_data.raw_data,        //
    .dlc = sizeof(tx_data.raw_data)  //
  };

  generic_can_tx(&s_generic_can.base, &tx_msg);

  if (soft_timer_cancel(s_charger_timer_id)) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode charger_controller_init() {
  LOG_DEBUG("IN charger cont init\n");
  generic_can_mcp2515_init(&s_generic_can, &mcp2515_settings);
  // mcp2515_register_cbs(s_generic_can.mcp2515, prv_charger_can_rx, NULL, NULL);
  generic_can_register_rx(&s_generic_can.base, prv_charger_can_rx, 0, 0, false, NULL);
  LOG_DEBUG("END charger cont init\n");
  return STATUS_CODE_OK;
}
