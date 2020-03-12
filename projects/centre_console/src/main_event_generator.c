#include "main_event_generator.h"
#include "centre_console_events.h"
#include "charging_manager.h"
#include "drive_fsm.h"
#include "event_queue.h"
#include "log.h"
#include "pedal_monitor.h"
#include "power_fsm.h"

// button input events: drive, reverse, parking, neutral, power
// states:
//  drive state
//  power state
//  charging state
//  pedal state
//  speed

// power:
//   powering on main:
//     power state: off/aux
//     pedal state: pressed
//
//   powering on aux
//     power state: off
//     pedal state: released
//
//   powering off
//     power state: on
//     drive state: parking/neutral
//
// drive/reverse:
//   setting the car to drive/reverse
//     speed: low
//     drive state: drive/reverse
//     charging state: no charging
//
// drive/reverse:
//   setting the car to drive/reverse
//     drive state: parking/neutral
//     charging state: no charging
//
// neutal/parking:
//   setting the car to neutal/parking
//     power state: on
//     speed: low
//

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

bool prv_process_drive_event(MainEventGeneratorStorage *storage, const Event *e) {
  prv_check_event(e->id, CENTRE_CONSOLE_BUTTON_PRESS_EVENT_DRIVE);
  const PowerState power_state = power_fsm_get_current_state(storage->power_fsm);
  if (power_state != POWER_STATE_MAIN) {
    return false;
  }
  DriveState drive_state = drive_fsm_get_global_state(storage->drive_fsm);
  EventId output_event = NUM_CENTRE_CONSOLE_POWER_EVENTS;
  ChargingState charging_state = get_charging_state();
  if (drive_state == DRIVE_STATE_PARKING || drive_state == DRIVE_STATE_NEUTRAL) {
    output_event = (charging_state == CHARGING_STATE_CHARGING) ? 
  } else {

  }
  return false;
}

bool prv_process_reverse_event(MainEventGeneratorStorage *storage, const Event *e) {
  return false;
}

bool prv_process_neutral_event(MainEventGeneratorStorage *storage, const Event *e) {
  return false;
}

bool prv_process_parking_event(MainEventGeneratorStorage *storage, const Event *e) {
  return false;
}

bool main_event_generator_process_event(MainEventGeneratorStorage *storage, const Event *event) {
  prv_false_or_return(prv_process_power_event(storage, event));
  prv_false_or_return(prv_process_drive_event(storage, event));
  prv_false_or_return(prv_process_reverse_event(storage, event));
  prv_false_or_return(prv_process_neutral_event(storage, event));
  prv_false_or_return(prv_process_parking_event(storage, event));
  return false;
}
