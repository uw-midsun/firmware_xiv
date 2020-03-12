#include "centre_console_events.h"
#include "log.h"
#include "main_event_generator.h"
#include "ms_test_helpers.h"
#include "pedal_monitor.h"
#include "power_fsm.h"
#include "test_helpers.h"
#include "unity.h"

void teardown_test(void) {}

static PowerState s_current_power_state = NUM_POWER_STATES;
static PedalState s_current_pedal_state = NUM_PEDAL_STATES;
static DriveState s_current_drive_state = NUM_DRIVE_STATES;

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
  EventId input_event;
  EventId output_event;
  bool will_transition;
} TestScenario;

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
  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_power_test_scenarios); i++) {
    TestScenario scenario = s_power_test_scenarios[i];
    s_current_power_state = scenario.power_state;
    s_current_drive_state = scenario.drive_state;
    s_current_pedal_state = scenario.pedal_state;
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

void test_process_drive_button_scenarios(void) {}
