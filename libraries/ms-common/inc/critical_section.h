#pragma once
// Critical sections to protect code that should not be interrupted.

#include <stdbool.h>

// To protect critical code use the following two functions:
// EXAMPLE:
//
// bool disabled = critical_section_start();
// // Critical code here.
// critical_section_end(disabled);
// // ...
//
// This will also protect nested attempts from enabling and disabling interrupts
// from prematurely ending the critical section.

// Use this macro to mark a function as critical - it will automatically be
// ended as the function goes out of scope. EXAMPLE:
//
// void foo(void) {
//   CRITICAL_SECTION_AUTOEND;
//   // Critical code here.
// }
#define CRITICAL_SECTION_AUTOEND \
  __attribute__((cleanup(_critical_section_cleanup))) bool _disabled = critical_section_start();

// Disables all interrupts across all lines/inputs. Returns true if the function
// disabled interrupts.
bool critical_section_start(void);

// Enables all registered interrupts on all lines/inputs. Passing true to this
// function can be used to forcibly end all critical sections.
void critical_section_end(bool disabled_in_scope);

// Internal use only - do not call outside of CRITICAL_SECTION_AUTOEND
void _critical_section_cleanup(bool *disabled_in_scope);
