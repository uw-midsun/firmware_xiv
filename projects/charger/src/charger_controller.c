#include "charger_controller.h"

#include <stdbool.h>
#include <stdint.h>

#include "can_transmit.h"
#include "charger_defs.h"
#include "charger_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can_mcp2515.h"
#include "mcp2515.h"
#include "soft_timer.h"

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

static TxMsgData prv_build_charger_tx(TxControl charge) {
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
  TxMsgData tx_data = prv_build_charger_tx(TX_CONTROL_START_CHARGING);

  GenericCanMsg tx_msg = {
    .id = CHARGER_TX_CAN_ID,    //
    .extended = true,           //
    .data = tx_data.raw,        //
    .dlc = sizeof(tx_data.raw)  //
  };

  generic_can_tx(&s_generic_can.base, &tx_msg);

  soft_timer_start_millis(CHARGER_TX_PERIOD_MS, prv_periodic_charger_tx, NULL, &s_charger_timer_id);
}

static void prv_charger_can_rx(uint32_t id, bool extended, uint64_t data, size_t dlc,
                               void *context) {
  if (id != CHARGER_RX_CAN_ID) {
    return;
  }
  RxMsgData rx_msg = { .raw = data };
  if (rx_msg.fields.status_flags.raw) {
    charger_controller_deactivate();
  }
  if (rx_msg.fields.status_flags.flags.hardware_failure) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_HARDWARE_FAILURE);
  }
  if (rx_msg.fields.status_flags.flags.over_temp) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_OVER_TEMP);
  }
  if (rx_msg.fields.status_flags.flags.wrong_voltage) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_WRONG_VOLTAGE);
  }
  if (rx_msg.fields.status_flags.flags.polarity_failure) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_POLARITY_FAILURE);
  }
  if (rx_msg.fields.status_flags.flags.communication_timeout) {
    CAN_TRANSMIT_CHARGER_FAULT(EE_CHARGER_FAULT_COMMUNICATION_TIMEOUT);
  }
}

StatusCode charger_controller_activate(uint16_t max_allowable_current) {
  s_vc.values.complete.current = max_allowable_current;
  prv_periodic_charger_tx(0, NULL);
  return STATUS_CODE_OK;
}

StatusCode charger_controller_deactivate() {
  s_vc.values.complete.current = 0;
  TxMsgData tx_data = prv_build_charger_tx(TX_CONTROL_STOP_CHARGING);

  // Send 'stop' message
  GenericCanMsg tx_msg = {
    .id = CHARGER_TX_CAN_ID,    //
    .extended = true,           //
    .data = tx_data.raw,        //
    .dlc = sizeof(tx_data.raw)  //
  };

  generic_can_tx(&s_generic_can.base, &tx_msg);

  if (soft_timer_cancel(s_charger_timer_id)) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode charger_controller_init() {
  generic_can_mcp2515_init(&s_generic_can, &mcp2515_settings);
  mcp2515_register_cbs(s_generic_can.mcp2515, prv_charger_can_rx, NULL, NULL);
  return STATUS_CODE_OK;
}
