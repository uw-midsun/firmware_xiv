#include "race_switch.h"

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

static GpioAddress s_race_switch_address = { .port = GPIO_PORT_A, .pin = 4 };

// Must be redefined to pass as a pointer
static GpioAddress s_voltage_regulator_enable_address = VOLTAGE_REGULATOR_ENABLE_ADDRESS;
static GpioAddress s_voltage_regulator_monitor_address = VOLTAGE_REGULATOR_MONITOR_ADDRESS;

// Gpio state is low since car begins in normal mode
static GpioSettings s_gpio_settings = { .state = GPIO_STATE_LOW,
                                        .direction = GPIO_DIR_IN,
                                        .resistor = GPIO_RES_NONE,
                                        .alt_function = GPIO_ALTFN_NONE };

static InterruptSettings s_interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                                  .priority = INTERRUPT_PRIORITY_NORMAL };

static void prv_voltage_monitor_error_callback(VoltageRegulatorError error, void *context) {
  LOG_WARN("Voltage Regulator Error %d\n", error);
}

static VoltageRegulatorSettings s_voltage_regulator_settings = {
  .enable_pin = VOLTAGE_REGULATOR_ENABLE_ADDRESS,
  .monitor_pin = VOLTAGE_REGULATOR_MONITOR_ADDRESS,
  .timer_callback_delay_ms = VOLTAGE_REGULATOR_DELAY_MS,
  .error_callback = prv_voltage_monitor_error_callback,
  .error_callback_context = NULL
};

FSM_DECLARE_STATE(race_switch_off);
FSM_DECLARE_STATE(race_switch_on);

FSM_STATE_TRANSITION(race_switch_off) {
  FSM_ADD_TRANSITION(RACE_STATE_ON, race_switch_on);
}

FSM_STATE_TRANSITION(race_switch_on) {
  FSM_ADD_TRANSITION(RACE_STATE_OFF, race_switch_off);
}

static RaceState s_event_lookup[] = { [RACE_SWITCH_EVENT_OFF] = RACE_STATE_OFF,
                                      [RACE_SWITCH_EVENT_ON] = RACE_STATE_ON };

// Triggered when the fsm switches to the normal mode
// 5V regulator is enabled
static void prv_state_race_off_output(Fsm *fsm, const Event *e, void *context) {
  RaceSwitchFsmStorage *storage = (RaceSwitchFsmStorage *)context;
  voltage_regulator_set_enabled(&(storage->voltage_storage), true);
  storage->current_state = RACE_STATE_OFF;
}

// Triggered when the fsm switches to the race mode
// 5V regulator is disabled
static void prv_state_race_on_output(Fsm *fsm, const Event *e, void *context) {
  RaceSwitchFsmStorage *storage = (RaceSwitchFsmStorage *)context;
  voltage_regulator_set_enabled(&(storage->voltage_storage), false);
  storage->current_state = RACE_STATE_ON;
}

bool race_switch_fsm_process_event(RaceSwitchFsmStorage *storage, Event *e) {
  return fsm_process_event(&(storage->race_switch_fsm), e);
}

static void prv_gpio_interrupt_handler(const GpioAddress *address, void *context) {
  RaceSwitchFsmStorage *storage = (RaceSwitchFsmStorage *)context;
  GpioState input_state;
  gpio_get_state(address, &input_state);

  Event e = { 0 };

  // Rising edge indicates the gpio state has become high and the car should go into race mode
  if (input_state == GPIO_STATE_HIGH) {
    e.id = s_event_lookup[RACE_SWITCH_EVENT_ON];
  } else {
    e.id = s_event_lookup[RACE_SWITCH_EVENT_OFF];
  }

  race_switch_fsm_process_event(storage, &e);
}

StatusCode race_switch_fsm_init(RaceSwitchFsmStorage *storage) {
  fsm_state_init(race_switch_off, prv_state_race_off_output);
  fsm_state_init(race_switch_on, prv_state_race_on_output);

  status_ok_or_return(gpio_init_pin(&s_race_switch_address, &s_gpio_settings));
  status_ok_or_return(gpio_init_pin(&s_voltage_regulator_enable_address, &s_gpio_settings));
  status_ok_or_return(gpio_init_pin(&s_voltage_regulator_monitor_address, &s_gpio_settings));

  // Depending on whether the edge is rising or falling we can determine whether to switch to
  // switch into race or normal mode
  status_ok_or_return(gpio_it_register_interrupt(&s_race_switch_address, &s_interrupt_settings,
                                                 INTERRUPT_EDGE_RISING_FALLING,
                                                 prv_gpio_interrupt_handler, storage));

  status_ok_or_return(
      voltage_regulator_init(&(storage->voltage_storage), &s_voltage_regulator_settings));

  // Since car begins in normal mode, the 5V regulator must be enabled
  status_ok_or_return(voltage_regulator_set_enabled(&(storage->voltage_storage), true));

  // Initially the car starts in normal mode
  storage->current_state = RACE_STATE_OFF;
  fsm_init(&(storage->race_switch_fsm), "Race Switch FSM", &race_switch_off, storage);
  return STATUS_CODE_OK;
}

// Used mainly for testing purposes
RaceState race_switch_fsm_get_current_state(RaceSwitchFsmStorage *storage) {
  return storage->current_state;
}
