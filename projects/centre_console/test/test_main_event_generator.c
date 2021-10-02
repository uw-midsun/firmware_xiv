#include "centre_console_events.h"
#include "charging_manager.h"
#include "fault_monitor.h"
#include "log.h"
#include "main_event_generator.h"
#include "ms_test_helpers.h"
#include "pedal_monitor.h"
#include "power_fsm.h"
#include "speed_monitor.h"
#include "test_helpers.h"
#include "unity.h"

void teardown_test(void) {}

static PowerState s_current_power_state = NUM_POWER_STATES;
static PedalState s_current_pedal_state = NUM_PEDAL_STATES;
static DriveState s_current_drive_state = NUM_DRIVE_STATES;
static SpeedState s_current_speed_state = NUM_SPEED_STATES;
static ChargingState s_current_charging_state = NUM_CHARGING_STATES;

static MainEventGeneratorStorage s_storage = { 0 };
static PowerFsmStorage s_power_storage = { 0 };
static DriveFsmStorage s_drive_storage = { 0 };

PowerState TEST_MOCK(power_fsm_get_current_state)(PowerFsmStorage *power_fsm) {
  TEST_ASSERT_EQUAL(&s_power_storage, power_fsm);
  return s_current_power_state;
}

PedalState TEST_MOCK(get_pedal_state)(void) {
  return s_current_pedal_state;
}

DriveState TEST_MOCK(drive_fsm_get_global_state)(DriveFsmStorage *storage) {
  TEST_ASSERT_EQUAL(&s_drive_storage, storage);
  return s_current_drive_state;
}

SpeedState *TEST_MOCK(get_global_speed_state)(void) {
  return &s_current_speed_state;
}

ChargingState TEST_MOCK(get_global_charging_state)(void) {
  return s_current_charging_state;
}

void setup_test(void) {
  event_queue_init();
  MainEventGeneratorResources resources = { .power_fsm = &s_power_storage,
                                            .drive_fsm = &s_drive_storage };
  main_event_generator_init(&s_storage, &resources);
}

typedef struct TestScenario {
  PowerState power_state;
  DriveState drive_state;
  PedalState pedal_state;
  SpeedState speed_state;
  ChargingState charging_state;
  EventId input_event;
  EventId output_event;
  bool will_transition;
} TestScenario;

void prv_run_scenarios(const TestScenario *scenarios, uint8_t num_scenarios) {
  for (uint8_t i = 0; i < num_scenarios; i++) {
    TestScenario scenario = scenarios[i];
    s_current_power_state = scenario.power_state;
    s_current_drive_state = scenario.drive_state;
    s_current_pedal_state = scenario.pedal_state;
    s_current_charging_state = scenario.charging_state;
    s_current_speed_state = scenario.speed_state;
    Event e = { .id = scenario.input_event };
    if (scenario.will_transition) {
      TEST_ASSERT_TRUE(main_event_generator_process_event(&s_storage, &e));
      MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, scenario.output_event, 0);
    } else {
      TEST_ASSERT_FALSE(main_event_generator_process_event(&s_storage, &e));
      MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
    }
  }
}

static TestScenario s_power_test_scenarios[] = {
  { .power_state = POWER_STATE_OFF,
    .pedal_state = PEDAL_STATE_PRESSED,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .output_event = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN,
    .will_transition = true },
  { .power_state = POWER_STATE_OFF,
    .pedal_state = PEDAL_STATE_RELEASED,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .output_event = CENTRE_CONSOLE_POWER_EVENT_ON_AUX,
    .will_transition = true },
  { .power_state = POWER_STATE_MAIN,
    .drive_state = DRIVE_STATE_DRIVE,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .will_transition = false },
  { .power_state = POWER_STATE_MAIN,
    .drive_state = DRIVE_STATE_PARKING,
    .pedal_state = PEDAL_STATE_RELEASED,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .output_event = CENTRE_CONSOLE_POWER_EVENT_OFF,
    .will_transition = true },
  { .power_state = POWER_STATE_MAIN,
    .drive_state = DRIVE_STATE_REVERSE,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .will_transition = false },
  { .power_state = POWER_STATE_MAIN,
    .drive_state = DRIVE_STATE_NEUTRAL,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .output_event = CENTRE_CONSOLE_POWER_EVENT_OFF,
    .will_transition = true },
  { .power_state = POWER_STATE_AUX,
    .pedal_state = PEDAL_STATE_RELEASED,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .output_event = CENTRE_CONSOLE_POWER_EVENT_OFF,
    .will_transition = true },
  { .power_state = POWER_STATE_AUX,
    .pedal_state = PEDAL_STATE_PRESSED,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER,
    .output_event = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN,
    .will_transition = true },
  { .power_state = POWER_STATE_FAULT, .will_transition = false },
};

void test_process_power_button_scenarios(void) {
  prv_run_scenarios(s_power_test_scenarios, SIZEOF_ARRAY(s_power_test_scenarios));
}

static TestScenario s_drive_button_scenarios[] = {
  { .power_state = POWER_STATE_OFF,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE,
    .will_transition = false },
  { .power_state = POWER_STATE_MAIN,
    .charging_state = CHARGING_STATE_CHARGING,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE,
    .will_transition = false },
  { .power_state = POWER_STATE_MAIN,
    .charging_state = CHARGING_STATE_NOT_CHARGING,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE,
    .drive_state = DRIVE_STATE_NEUTRAL,
    .speed_state = SPEED_STATE_MOVING,
    .output_event = DRIVE_FSM_INPUT_EVENT_DRIVE,
    .will_transition = true },
  { .power_state = POWER_STATE_MAIN,
    .charging_state = CHARGING_STATE_NOT_CHARGING,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE,
    .drive_state = DRIVE_STATE_NEUTRAL,
    .speed_state = SPEED_STATE_STATIONARY,
    .output_event = DRIVE_FSM_INPUT_EVENT_DRIVE,
    .will_transition = true },
  { .power_state = POWER_STATE_AUX,
    .charging_state = CHARGING_STATE_NOT_CHARGING,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_REVERSE,
    .drive_state = DRIVE_STATE_NEUTRAL,
    .speed_state = SPEED_STATE_STATIONARY,
    .will_transition = false },
  { .power_state = POWER_STATE_MAIN,
    .charging_state = CHARGING_STATE_NOT_CHARGING,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_REVERSE,
    .drive_state = DRIVE_STATE_PARKING,
    .speed_state = SPEED_STATE_STATIONARY,
    .output_event = DRIVE_FSM_INPUT_EVENT_REVERSE,
    .will_transition = true },
  { .power_state = POWER_STATE_MAIN,
    .charging_state = CHARGING_STATE_NOT_CHARGING,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_REVERSE,
    .drive_state = DRIVE_STATE_DRIVE,
    .speed_state = SPEED_STATE_MOVING,
    .will_transition = false },
  { .power_state = POWER_STATE_MAIN,
    .charging_state = CHARGING_STATE_NOT_CHARGING,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE,
    .drive_state = DRIVE_STATE_REVERSE,
    .speed_state = SPEED_STATE_STATIONARY,
    .output_event = DRIVE_FSM_INPUT_EVENT_DRIVE,
    .will_transition = true },
};

void test_process_drive_button_scenarios(void) {
  prv_run_scenarios(s_drive_button_scenarios, SIZEOF_ARRAY(s_drive_button_scenarios));
}

static TestScenario s_neutral_reverse_button_scenarios[] = {
  { .power_state = POWER_STATE_MAIN,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL,
    .drive_state = DRIVE_STATE_REVERSE,
    .speed_state = SPEED_STATE_STATIONARY,
    .output_event = DRIVE_FSM_INPUT_EVENT_NEUTRAL,
    .will_transition = true },
  { .power_state = POWER_STATE_MAIN,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL,
    .drive_state = DRIVE_STATE_DRIVE,
    .speed_state = SPEED_STATE_MOVING,
    .output_event = DRIVE_FSM_INPUT_EVENT_NEUTRAL,
    .will_transition = true },
  { .power_state = POWER_STATE_MAIN,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL,
    .drive_state = DRIVE_STATE_PARKING,
    .output_event = DRIVE_FSM_INPUT_EVENT_NEUTRAL,
    .will_transition = true },
  { .power_state = POWER_STATE_MAIN,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING,
    .drive_state = DRIVE_STATE_DRIVE,
    .speed_state = SPEED_STATE_MOVING,
    .will_transition = false },
  { .power_state = POWER_STATE_MAIN,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING,
    .drive_state = DRIVE_STATE_DRIVE,
    .speed_state = SPEED_STATE_STATIONARY,
    .output_event = DRIVE_FSM_INPUT_EVENT_PARKING,
    .will_transition = true },
  { .power_state = POWER_STATE_AUX,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING,
    .drive_state = DRIVE_STATE_NEUTRAL,
    .speed_state = SPEED_STATE_STATIONARY,
    .output_event = DRIVE_FSM_INPUT_EVENT_PARKING,
    .will_transition = false },
  { .power_state = POWER_STATE_OFF,
    .input_event = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL,
    .drive_state = DRIVE_STATE_PARKING,
    .speed_state = SPEED_STATE_STATIONARY,
    .will_transition = false },
};

void test_process_neutral_reverse_button_scenarios(void) {
  prv_run_scenarios(s_neutral_reverse_button_scenarios,
                    SIZEOF_ARRAY(s_neutral_reverse_button_scenarios));
}
