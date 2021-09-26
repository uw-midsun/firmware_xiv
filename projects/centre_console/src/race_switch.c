#include "race_switch.h"

#include <stdbool.h>

#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "status.h"
#include "voltage_regulator.h"

#define VOLTAGE_REGULATOR_ENABLE_ADDRESS \
  { .port = GPIO_PORT_B, .pin = 6 }
#define VOLTAGE_REGULATOR_MONITOR_ADDRESS \
  { .port = GPIO_PORT_B, .pin = 7 }
#define VOLTAGE_REGULATOR_DELAY_MS 25
#define RACE_SWITCH_FSM_NAME "Race Switch FSM"
#define RACE_MODE GPIO_STATE_HIGH

static GpioAddress s_race_switch_address = { .port = GPIO_PORT_A, .pin = 4 };

// Must be redefined to pass as a pointer
static GpioAddress s_voltage_regulator_enable_address = VOLTAGE_REGULATOR_ENABLE_ADDRESS;
static GpioAddress s_voltage_regulator_monitor_address = VOLTAGE_REGULATOR_MONITOR_ADDRESS;

static GpioSettings s_gpio_settings = { .state = GPIO_STATE_LOW,
                                        .direction = GPIO_DIR_IN,
                                        .resistor = GPIO_RES_NONE,
                                        .alt_function = GPIO_ALTFN_NONE };
static GpioSettings s_voltage_enable_gpio_settings = { .state = GPIO_STATE_LOW,
                                                       .direction = GPIO_DIR_OUT,
                                                       .resistor = GPIO_RES_NONE,
                                                       .alt_function = GPIO_ALTFN_NONE };

static InterruptSettings s_interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                                  .priority = INTERRUPT_PRIORITY_NORMAL };

static void prv_voltage_monitor_error_callback(VoltageRegulatorError error, void *context) {
  if (error == VOLTAGE_REGULATOR_ERROR_ON_WHEN_SHOULD_BE_OFF) {
    LOG_WARN("Voltage Regulator Error: On when should be off\n");
  } else {
    LOG_WARN("Voltage Regulator Error: Off when should be on\n");
  }
}

static VoltageRegulatorSettings s_voltage_regulator_settings = {
  .enable_pin = VOLTAGE_REGULATOR_ENABLE_ADDRESS,
  .monitor_pin = VOLTAGE_REGULATOR_MONITOR_ADDRESS,
  .timer_callback_delay_ms = VOLTAGE_REGULATOR_DELAY_MS,
  .error_callback = prv_voltage_monitor_error_callback,
  .error_callback_context = NULL,
};

FSM_DECLARE_STATE(race_switch_off);
FSM_DECLARE_STATE(race_switch_on);

FSM_STATE_TRANSITION(race_switch_off) {
  FSM_ADD_TRANSITION(RACE_SWITCH_EVENT_ON, race_switch_on);
}

FSM_STATE_TRANSITION(race_switch_on) {
  FSM_ADD_TRANSITION(RACE_SWITCH_EVENT_OFF, race_switch_off);
}

// Triggered when the fsm switches to the normal mode
// 5V regulator is enabled
static void prv_state_race_off_output(Fsm *fsm, const Event *e, void *context) {
  RaceSwitchFsmStorage *storage = (RaceSwitchFsmStorage *)context;
  voltage_regulator_set_enabled(&storage->voltage_storage, true);
  storage->current_state = RACE_STATE_OFF;
}

// Triggered when the fsm switches to the race mode
// 5V regulator is disabled
static void prv_state_race_on_output(Fsm *fsm, const Event *e, void *context) {
  RaceSwitchFsmStorage *storage = (RaceSwitchFsmStorage *)context;
  voltage_regulator_set_enabled(&storage->voltage_storage, false);
  storage->current_state = RACE_STATE_ON;
}

static StatusCode prv_racemode_callback(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  RaceSwitchFsmStorage *storage = (RaceSwitchFsmStorage *)context;
  uint8_t is_race_mode = false;
  CAN_UNPACK_RACE_NORMAL_SWITCH_MODE(msg, &is_race_mode);
  voltage_regulator_set_enabled(&storage->voltage_storage, !is_race_mode);

  if (is_race_mode) {
    event_raise_no_data(RACE_SWITCH_EVENT_ON);
  } else {
    event_raise_no_data(RACE_SWITCH_EVENT_OFF);
  }

  return STATUS_CODE_OK;
}

bool race_switch_fsm_process_event(RaceSwitchFsmStorage *storage, Event *e) {
  return fsm_process_event(&storage->race_switch_fsm, e);
}

static void prv_gpio_interrupt_handler(const GpioAddress *address, void *context) {
  RaceSwitchFsmStorage *storage = (RaceSwitchFsmStorage *)context;
  GpioState input_state;
  gpio_get_state(address, &input_state);

  if (input_state == RACE_MODE) {
    event_raise_no_data(RACE_SWITCH_EVENT_ON);
  } else {
    event_raise_no_data(RACE_SWITCH_EVENT_OFF);
  }
}

StatusCode race_switch_fsm_init(RaceSwitchFsmStorage *storage) {
  fsm_state_init(race_switch_off, prv_state_race_off_output);
  fsm_state_init(race_switch_on, prv_state_race_on_output);

  status_ok_or_return(gpio_init_pin(&s_race_switch_address, &s_gpio_settings));
  status_ok_or_return(
      gpio_init_pin(&s_voltage_regulator_enable_address, &s_voltage_enable_gpio_settings));
  status_ok_or_return(gpio_init_pin(&s_voltage_regulator_monitor_address, &s_gpio_settings));

  // Depending on whether the edge is rising or falling we can determine whether to switch into
  // race or normal mode
  status_ok_or_return(gpio_it_register_interrupt(&s_race_switch_address, &s_interrupt_settings,
                                                 INTERRUPT_EDGE_RISING_FALLING,
                                                 prv_gpio_interrupt_handler, storage));

  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_RACE_NORMAL_SWITCH_MODE,
                                              prv_racemode_callback, storage));

  status_ok_or_return(
      voltage_regulator_init(&storage->voltage_storage, &s_voltage_regulator_settings));

  // Determine whether the car starts in normal or race mode
  GpioState race_switch_initial_state;
  status_ok_or_return(gpio_get_state(&s_race_switch_address, &race_switch_initial_state));

  bool is_race_mode = race_switch_initial_state == RACE_MODE;

  // If in race mode (gpio state high or 1) voltage regulator must be disabled, otherwise enable
  bool is_voltage_regulator_enabled = !is_race_mode;
  status_ok_or_return(
      voltage_regulator_set_enabled(&storage->voltage_storage, is_voltage_regulator_enabled));

  storage->current_state = is_race_mode ? RACE_STATE_ON : RACE_STATE_OFF;

  if (is_race_mode) {
    fsm_init(&storage->race_switch_fsm, RACE_SWITCH_FSM_NAME, &race_switch_on, storage);
  } else {
    fsm_init(&storage->race_switch_fsm, RACE_SWITCH_FSM_NAME, &race_switch_off, storage);
  }

  return STATUS_CODE_OK;
}

// Used mainly for testing purposes
RaceState race_switch_fsm_get_current_state(RaceSwitchFsmStorage *storage) {
  return storage->current_state;
}
