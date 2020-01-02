#pragma once 

// Module for the mechanical brake 
// Detects whether the brake is pressed or unpressed past a set threshold 

// Requires ADS1015 and soft timers to be initialized 
// Requires calibration routines to have been run
    // Can be found in mech_brake_calibration

// The module reads the position of the mechanical brake via the ADS1015, and
// scales those readings into a numerator value corresponding to a denomiator
// of EE_DRIVE_OUTPUT_DENOMINATOR. This scalled numerator value is called the
// pedal position.

// At the same time, it raises events when the brake is pressed and depressed:
//
//    * PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED
//    * PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_UNPRESSED 
//
// When these events are raised, they are given the position value that gets 
// passed into mech_brake_get_position. This allows FSM changes to be triggered 
// in the application code. 

// Windowed reading is used such that once the brake is pressed it must pass a 
// threshold to be considered as braking. Once released it must be released 
// further than the initial threshold. This mechanism is intended to mitigate 
// fluttering about the threshold. 

#include <stdbool.h> 
#include <stdint.h> 

#include "ads1015.h" 
#include "soft_timer.h"
#include "status.h"

typedef struct MechBrakeCalibrationData {
    // When brake is considered fully unpressed 
    int16_t unpressed_value; 
    // When brake is considered fully pressed 
    int16_t full_pressed_value; 
} MechBrakeCalibrationData; 

typedef struct MechBrakeSettings {
    Ads1015Storage *ads1015_storage; 
    // Percentage value above which PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED is raised
    int16_t brake_pressed_threshold_percentage; 
    // Percentage value below which PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_RELEASED is raised
    int16_t brake_released_threshold_percentage; 
    // Percentage tolerance for the lower and upper bound of the position 
    int16_t bounds_tolerance_percentage; 
    Ads1015Channel ads1015_channel; 
} MechBrakeSettings;

typedef struct MechBrakeStorage {
    MechBrakeCalibrationData mech_brake_calibration_data; 
    Ads1015Storage *ads1015_storage; 
    Ads1015Channel ads1015_channel; 
    // Minimum value of the position based on the tolerance value 
    int16_t lower_bound; 
    // Maximum value of the position based on the tolerance value 
    int16_t upper_bound; 
    // Position pressed value calculated using the brake_pressed_threshold_percentage
    int16_t pressed_threshold_position;
    // Position unpressed value calculated using the brake_unpressed_threshold_percentage
    int16_t unpressed_threshold_position;
    bool prev_pressed;
} MechBrakeStorage; 

// Configures the ADS1015 channel, sets mech_brake_get_position to be callback 
// of channel whenever there is an interrupt 
// 
// Calculates lower_bound, upper_bound, pressed_threshold_position, 
// and unpresssed_threshold_position of *storage 
StatusCode mech_brake_init(MechBrakeStorage *storage, const MechBrakeSettings *settings, const MechBrakeCalibrationData *calibration_data); 

// Gets the current position of the mechanical brake, returns that value to *position 
StatusCode mech_brake_get_position(MechBrakeStorage *storage, int16_t *position); 

