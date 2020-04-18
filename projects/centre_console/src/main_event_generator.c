#include "main_event_generator.h"
#include "centre_console_events.h"
#include "charging_manager.h"
#include "drive_fsm.h"
#include "event_queue.h"
#include "log.h"
#include "pedal_monitor.h"
#include "power_fsm.h"
#include "speed_monitor.h"

StatusCode main_event_generator_init(MainEventGeneratorStorage *storage,
                                     MainEventGeneratorResources *resources) {
  storage->power_fsm = resources->power_fsm;
  storage->drive_fsm = resources->drive_fsm;
  return STATUS_CODE_OK;
}

#define prv_false_or_return(transitioned) \
  if (transitioned) return true

#define prv_check_event(event_id, expected_event_id) \
  if ((event_id) != (expected_event_id)) return false

bool prv_process_power_event(MainEventGeneratorStorage *storage, const Event *e) {
  prv_check_event(e->id, CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER);
  const PowerState power_state = power_fsm_get_current_state(storage->power_fsm);
  PedalState pedal_state = get_pedal_state();
  DriveState drive_state = drive_fsm_get_global_state(storage->drive_fsm);
  EventId output_event = NUM_CENTRE_CONSOLE_POWER_EVENTS;
  if (power_state == POWER_STATE_OFF) {
    output_event = (pedal_state == PEDAL_STATE_PRESSED) ? CENTRE_CONSOLE_POWER_EVENT_ON_MAIN
                                                        : CENTRE_CONSOLE_POWER_EVENT_ON_AUX;
  } else if (power_state == POWER_STATE_AUX) {
    output_event = (pedal_state == PEDAL_STATE_PRESSED) ? CENTRE_CONSOLE_POWER_EVENT_ON_MAIN
                                                        : CENTRE_CONSOLE_POWER_EVENT_OFF;
  } else if (power_state == POWER_STATE_MAIN) {
    output_event = (drive_state == DRIVE_STATE_NEUTRAL || drive_state == DRIVE_STATE_PARKING)
                       ? CENTRE_CONSOLE_POWER_EVENT_OFF
                       : NUM_CENTRE_CONSOLE_POWER_EVENTS;
  }
  if (output_event == NUM_CENTRE_CONSOLE_POWER_EVENTS) {
    return false;
  }
  event_raise(output_event, 0);
  return true;
}

#define prv_power_main_or_return(storage)                                         \
  const PowerState power_state = power_fsm_get_current_state(storage->power_fsm); \
  if (power_state != POWER_STATE_MAIN) {                                          \
    return false;                                                                 \
  }

#define prv_check_drive_state(drive_state)                  \
  if (drive_state >= DRIVE_STATE_TRANSITIONING) {           \
    if (drive_state > DRIVE_STATE_TRANSITIONING) {          \
      LOG_CRITICAL("Invalid drive state: %d", drive_state); \
    }                                                       \
    return false;                                           \
  }

bool prv_process_drive_reverse_event(MainEventGeneratorStorage *storage, const Event *e) {
  if (e->id != CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE &&
      e->id != CENTRE_CONSOLE_BUTTON_PRESS_EVENT_REVERSE) {
    return false;
  }

  prv_power_main_or_return(storage);

  const ChargingState charging_state = *get_global_charging_state();
  if (charging_state == CHARGING_STATE_CHARGING) {
    return false;
  }

  EventId output_event = NUM_DRIVE_FSM_INPUT_EVENTS;

  DriveState drive_state = drive_fsm_get_global_state(storage->drive_fsm);
  prv_check_drive_state(drive_state);

  SpeedState speed_state = *get_global_speed_state();
  if (drive_state == DRIVE_STATE_DRIVE || drive_state == DRIVE_STATE_REVERSE) {
    if (speed_state == SPEED_STATE_STATIONARY) {
      output_event = e->id == CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE
                         ? DRIVE_FSM_INPUT_EVENT_DRIVE
                         : DRIVE_FSM_INPUT_EVENT_REVERSE;
    }
  } else {
    output_event = e->id == CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE ? DRIVE_FSM_INPUT_EVENT_DRIVE
                                                                    : DRIVE_FSM_INPUT_EVENT_REVERSE;
  }
  if (output_event == NUM_DRIVE_FSM_INPUT_EVENTS) {
    return false;
  }
  event_raise(output_event, 0);
  return true;
}

bool prv_process_neutral_parking_event(MainEventGeneratorStorage *storage, const Event *e) {
  if (e->id != CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL &&
      e->id != CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING) {
    return false;
  }
  const PowerState power_state = power_fsm_get_current_state(storage->power_fsm);
  if (power_state == POWER_STATE_OFF) {
    return false;
  }
  EventId output_event = NUM_DRIVE_FSM_INPUT_EVENTS;
  SpeedState speed_state = *get_global_speed_state();

  output_event = e->id == CENTRE_CONSOLE_BUTTON_PRESS_EVENT_NEUTRAL ? DRIVE_FSM_INPUT_EVENT_NEUTRAL
                                                                    : DRIVE_FSM_INPUT_EVENT_PARKING;

  if ((e->id == CENTRE_CONSOLE_BUTTON_PRESS_EVENT_PARKING) && speed_state == SPEED_STATE_MOVING) {
    output_event = NUM_DRIVE_FSM_INPUT_EVENTS;
  }

  if (output_event == NUM_DRIVE_FSM_INPUT_EVENTS) {
    return false;
  }
  event_raise(output_event, 0);
  return true;
}

bool main_event_generator_process_event(MainEventGeneratorStorage *storage, const Event *event) {
  prv_false_or_return(prv_process_power_event(storage, event));
  prv_false_or_return(prv_process_drive_reverse_event(storage, event));
  prv_false_or_return(prv_process_neutral_parking_event(storage, event));
  return false;
}
