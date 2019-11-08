#pragma once
// Delay function library
// Requires soft timers and interrupts to be initalized.
//
// Blocking delay for a fixed period of time. Still allows interrupts to
// trigger.
//
// Max allowable time is UINT32_MAX in microseconds (4294.967295 seconds).
// If a longer duration is needed, check the condition you are waiting on in a
// loop and then call this function again if the condition is not met.

#include <stdint.h>

// Delay for a period in microseconds.
void delay_us(uint32_t t);

// Delay for a period in milliseconds.
#define delay_ms(time) delay_us((time)*1000)

// Delay for a period in seconds.
#define delay_s(time) delay_us((time)*1000000)
