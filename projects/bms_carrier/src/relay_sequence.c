#include "relay_sequence.h"

#include "bms.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "gpio.h"
#include "gpio_it.h"
#include "mcp23008_gpio_expander.h"

static GpioAddress s_hv_relay_en = BMS_HV_RELAY_EN_PIN;
static GpioAddress s_gnd_relay_en = BMS_GND_RELAY_EN_PIN;
static GpioAddress s_io_expander_int = BMS_IO_EXPANDER_INT_PIN;
static Mcp23008GpioAddress s_mcp23008_hv = BMS_IO_EXPANDER_HV_SENSE_ADDR;
static Mcp23008GpioAddress s_mcp23008_gnd = BMS_IO_EXPANDER_GND_SENSE_ADDR;

void prv_relay_fault(RelayStorage *storage, bool internal) {
  // cancel timers
  soft_timer_cancel(storage->assertion_timer_id);
  soft_timer_cancel(storage->next_step_timer_id);
  // open relays
  relay_open_sequence(storage);
  // skip assertion, we're already faulted
  soft_timer_cancel(storage->assertion_timer_id);
  soft_timer_cancel(storage->next_step_timer_id);
  if (internal) {
    // fault bps
    fault_bps_set(EE_BPS_STATE_FAULT_RELAY);
  }
  CAN_TRANSMIT_BATTERY_RELAY_STATE(storage->hv_enabled, storage->gnd_enabled);
}

void prv_assert_state(SoftTimerId timer_id, void *context) {
  RelayStorage *storage = context;
  if (storage->hv_enabled != storage->hv_expected_state) {
    prv_relay_fault(storage, true);
  } else if (storage->gnd_enabled != storage->gnd_expected_state) {
    prv_relay_fault(storage, true);
  }
}

void prv_io_int_callback(const GpioAddress *address, void *context) {
  RelayStorage *storage = context;
  Mcp23008GpioState hv_state = NUM_MCP23008_GPIO_STATES;
  Mcp23008GpioState gnd_state = NUM_MCP23008_GPIO_STATES;
  mcp23008_gpio_get_state(&s_mcp23008_hv, &hv_state);
  mcp23008_gpio_get_state(&s_mcp23008_gnd, &gnd_state);
  storage->gnd_enabled = gnd_state;
  storage->hv_enabled = hv_state;
}

StatusCode prv_power_off_rx(const CanMessage *msg, void *context, CanAckStatus *ack) {
  RelayStorage *storage = context;
  uint16_t step = NUM_EE_POWER_OFF_SEQUENCES;
  CAN_UNPACK_POWER_OFF_SEQUENCE(msg, &step);
  if (step == EE_POWER_OFF_SEQUENCE_OPEN_BATTERY_RELAYS) {
    relay_open_sequence(storage);
  }
  *ack = CAN_ACK_STATUS_OK;
  return STATUS_CODE_OK;
}

StatusCode prv_power_on_rx(const CanMessage *msg, void *context, CanAckStatus *ack) {
  RelayStorage *storage = context;
  uint16_t step = NUM_EE_POWER_MAIN_SEQUENCES;
  CAN_UNPACK_POWER_ON_MAIN_SEQUENCE(msg, &step);
  if (step == EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS) {
    relay_close_sequence(storage);
  }
  *ack = CAN_ACK_STATUS_OK;
  return STATUS_CODE_OK;
}

StatusCode relay_sequence_init(RelayStorage *storage) {
  // register interrupt on GPIO expander
  GpioSettings io_expander_pin_settings = {
    .state = GPIO_STATE_LOW,         //
    .direction = GPIO_DIR_IN,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };
  InterruptSettings io_expander_int_settings = {
    .priority = INTERRUPT_PRIORITY_HIGH,  //
    .type = INTERRUPT_TYPE_INTERRUPT      //
  };
  gpio_init_pin(&s_io_expander_int, &io_expander_pin_settings);
  // from the data sheet, the MCP23008 interrupt is active-low by default
  gpio_it_register_interrupt(&s_io_expander_int, &io_expander_int_settings, INTERRUPT_EDGE_FALLING,
                             prv_io_int_callback, storage);

  // init appropriate pins
  GpioSettings relay_en_pin_settings = {
    .state = GPIO_STATE_LOW,         //
    .direction = GPIO_DIR_OUT,       //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };
  gpio_init_pin(&s_hv_relay_en, &relay_en_pin_settings);
  gpio_init_pin(&s_gnd_relay_en, &relay_en_pin_settings);

  // register callbacks
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE, prv_power_off_rx, storage);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_power_on_rx, storage);
  return STATUS_CODE_OK;
}

void prv_relay_open_done(SoftTimerId timer_id, void *context) {
  RelayStorage *storage = context;
  CAN_TRANSMIT_BATTERY_RELAY_STATE(storage->hv_enabled, storage->gnd_enabled);
}

StatusCode relay_open_sequence(RelayStorage *storage) {
  gpio_set_state(&s_hv_relay_en, GPIO_STATE_LOW);
  gpio_set_state(&s_gnd_relay_en, GPIO_STATE_LOW);
  storage->gnd_expected_state = false;
  storage->hv_expected_state = false;
  soft_timer_start_millis(RELAY_SEQUENCE_ASSERTION_DELAY_MS, prv_assert_state, storage,
                          &storage->assertion_timer_id);
  soft_timer_start_millis(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS, prv_relay_open_done, storage,
                          &storage->next_step_timer_id);
  return STATUS_CODE_OK;
}

void prv_relay_close_done(SoftTimerId timer_id, void *context) {
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(NULL, EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS);
}

void prv_relay_close_hv(SoftTimerId timer_id, void *context) {
  RelayStorage *storage = context;
  gpio_set_state(&s_hv_relay_en, GPIO_STATE_HIGH);
  storage->gnd_expected_state = true;
  storage->hv_expected_state = true;
  soft_timer_start_millis(RELAY_SEQUENCE_ASSERTION_DELAY_MS, prv_assert_state, storage,
                          &storage->assertion_timer_id);
  soft_timer_start_millis(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS, prv_relay_close_done, storage,
                          &storage->next_step_timer_id);
}

void prv_relay_close_gnd(RelayStorage *storage) {
  gpio_set_state(&s_gnd_relay_en, GPIO_STATE_HIGH);
  storage->gnd_expected_state = true;
  storage->hv_expected_state = false;
  soft_timer_start_millis(RELAY_SEQUENCE_ASSERTION_DELAY_MS, prv_assert_state, storage,
                          &storage->assertion_timer_id);
  soft_timer_start_millis(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS, prv_relay_close_hv, storage,
                          &storage->next_step_timer_id);
}

StatusCode relay_close_sequence(RelayStorage *storage) {
  prv_relay_close_gnd(storage);
  return STATUS_CODE_OK;
}

StatusCode relay_fault(RelayStorage *storage) {
  prv_relay_fault(storage, false);
  return STATUS_CODE_OK;
}
