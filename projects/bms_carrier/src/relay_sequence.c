#include "relay_sequence.h"

#include "bms.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "current_sense.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "mcp23008_gpio_expander.h"

static GpioAddress s_hv_relay_en = BMS_HV_RELAY_EN_PIN;
static GpioAddress s_gnd_relay_en = BMS_GND_RELAY_EN_PIN;
static GpioAddress s_io_expander_int = BMS_IO_EXPANDER_INT_PIN;
static Mcp23008GpioAddress s_mcp23008_hv = BMS_IO_EXPANDER_HV_SENSE_ADDR;
static Mcp23008GpioAddress s_mcp23008_gnd = BMS_IO_EXPANDER_GND_SENSE_ADDR;

void prv_cancel_timers(RelayStorage *storage) {
  if (storage->assertion_timer_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(storage->assertion_timer_id);
    storage->assertion_timer_id = SOFT_TIMER_INVALID_TIMER;
  }
  if (storage->next_step_timer_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(storage->next_step_timer_id);
    storage->next_step_timer_id = SOFT_TIMER_INVALID_TIMER;
  }
}

void prv_relay_fault(RelayStorage *storage, bool internal) {
  // cancel timers
  LOG_DEBUG("%s relay fault\n", internal ? "internal" : "external");
  prv_cancel_timers(storage);
  // open relays
  relay_open_sequence(storage);
  // skip assertion, we're already faulted
  prv_cancel_timers(storage);
  if (internal) {
    // fault bps
    fault_bps_set(EE_BPS_STATE_FAULT_RELAY);
  }
  CAN_TRANSMIT_BATTERY_RELAY_STATE(storage->hv_enabled, storage->gnd_enabled);
}

void prv_assert_state(SoftTimerId timer_id, void *context) {
  RelayStorage *storage = context;
  // if (storage->hv_enabled != storage->hv_expected_state) {
  //   prv_relay_fault(storage, true);
  // } else if (storage->gnd_enabled != storage->gnd_expected_state) {
  //   prv_relay_fault(storage, true);
  // }
  storage->assertion_timer_id = SOFT_TIMER_INVALID_TIMER;
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

StatusCode prv_relay_rx(const CanMessage *msg, void *context, CanAckStatus *ack) {
  RelayStorage *storage = context;
  uint16_t relay_mask = 0;
  uint16_t relay_state = 0;
  CAN_UNPACK_SET_RELAY_STATES(msg, &relay_mask, &relay_state);
  if (!(relay_mask & (1 << EE_RELAY_ID_BATTERY))) {
    return STATUS_CODE_OK;
  }
  if (relay_state) {
    LOG_DEBUG("relay rx starting relay close sequence\n");
    return relay_close_sequence(storage);
  } else {
    LOG_DEBUG("relay rx starting relay open sequence\n");
    return relay_open_sequence(storage);
  }
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
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_relay_rx, storage);
  return STATUS_CODE_OK;
}

void prv_relay_open_done(SoftTimerId timer_id, void *context) {
  RelayStorage *storage = context;
  LOG_DEBUG("relay open done\n");
  CAN_TRANSMIT_BATTERY_RELAY_STATE(storage->hv_enabled, storage->gnd_enabled);
  storage->next_step_timer_id = SOFT_TIMER_INVALID_TIMER;
}

StatusCode relay_open_sequence(RelayStorage *storage) {
  LOG_DEBUG("starting relay open sequence\n");
  if (current_sense_is_charging()) {
    gpio_set_state(&s_gnd_relay_en, GPIO_STATE_LOW);
    gpio_set_state(&s_hv_relay_en, GPIO_STATE_LOW);
  } else {
    gpio_set_state(&s_hv_relay_en, GPIO_STATE_LOW);
    gpio_set_state(&s_gnd_relay_en, GPIO_STATE_LOW);
  }
  storage->gnd_expected_state = false;
  storage->hv_expected_state = false;
  soft_timer_start_millis(RELAY_SEQUENCE_ASSERTION_DELAY_MS, prv_assert_state, storage,
                          &storage->assertion_timer_id);
  soft_timer_start_millis(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS, prv_relay_open_done, storage,
                          &storage->next_step_timer_id);
  return STATUS_CODE_OK;
}

void prv_relay_close_done(SoftTimerId timer_id, void *context) {
  RelayStorage *storage = context;
  LOG_DEBUG("relay close done\n");
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(NULL, EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS);
  storage->next_step_timer_id = SOFT_TIMER_INVALID_TIMER;
}

void prv_relay_close_hv(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("closing hv\n");
  RelayStorage *storage = context;
  gpio_set_state(&s_hv_relay_en, GPIO_STATE_HIGH);
  storage->gnd_expected_state = true;
  storage->hv_expected_state = true;
  storage->assertion_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->next_step_timer_id = SOFT_TIMER_INVALID_TIMER;
  soft_timer_start_millis(RELAY_SEQUENCE_ASSERTION_DELAY_MS, prv_assert_state, storage,
                          &storage->assertion_timer_id);
  soft_timer_start_millis(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS, prv_relay_close_done, storage,
                          &storage->next_step_timer_id);
}

void prv_relay_close_gnd(RelayStorage *storage) {
  LOG_DEBUG("closing gnd\n");
  gpio_set_state(&s_gnd_relay_en, GPIO_STATE_HIGH);
  storage->gnd_expected_state = true;
  storage->hv_expected_state = false;
  storage->assertion_timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->next_step_timer_id = SOFT_TIMER_INVALID_TIMER;
  soft_timer_start_millis(RELAY_SEQUENCE_ASSERTION_DELAY_MS, prv_assert_state, storage,
                          &storage->assertion_timer_id);
  soft_timer_start_millis(RELAY_SEQUENCE_NEXT_STEP_DELAY_MS, prv_relay_close_hv, storage,
                          &storage->next_step_timer_id);
}

StatusCode relay_close_sequence(RelayStorage *storage) {
  LOG_DEBUG("starting close sequence\n");
  prv_relay_close_gnd(storage);
  return STATUS_CODE_OK;
}

StatusCode relay_fault(RelayStorage *storage) {
  prv_relay_fault(storage, false);
  return STATUS_CODE_OK;
}
