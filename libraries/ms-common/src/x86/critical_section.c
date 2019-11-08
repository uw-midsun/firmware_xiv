#include "critical_section.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include <pthread.h>

#include "interrupt_def.h"
#include "x86_interrupt.h"

static bool s_interrupts_disabled = false;
static pthread_mutex_t s_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// WARNING: due to skipping the pthread_mutex lock in a signal_handler it is
// possible that during a signal handler's critical section a data race occurs
// due to another thread entering a critical section successfully during the
// handler's execution. This is due to the signal handler not locking the mutex
// however, this is to prevent deadlock. If this begins to be an issue we should
// revisiting the mutex implementation here.

bool critical_section_start(void) {
  if (!x86_interrupt_in_handler()) {
    pthread_mutex_lock(&s_mutex);
  }
  if (!s_interrupts_disabled) {
    // Update the signal mask to prevent interrupts from being executed on the
    // signal handler thread. Note that they can still queue like on an embedded
    // device.
    x86_interrupt_mask();
    s_interrupts_disabled = true;
    // Interrupts got disabled.
    return true;
  }
  // Interrupts did not get disabled.
  return false;
}

void critical_section_end(bool disabled_in_scope) {
  if (!x86_interrupt_in_handler()) {
    pthread_mutex_unlock(&s_mutex);
  }
  if (s_interrupts_disabled && disabled_in_scope) {
    // Clear the block mask for this process to allow signals to be processed.
    // (They will queue when disabled).
    s_interrupts_disabled = false;
    x86_interrupt_unmask();
  }
}

void _critical_section_cleanup(bool *disabled_in_scope) {
  critical_section_end(*disabled_in_scope);
}
