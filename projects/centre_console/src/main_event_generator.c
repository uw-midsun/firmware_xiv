#include "main_event_generator.h"
#include "centre_console_events.h"
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
//     drive state: parking/neutral
//     charging state: no charging
//
// neutal/parking:
//   setting the car to neutal/parking
//     power state: on
//     speed: low
// 

StatusCode main_event_generator_init() {
  
}








