#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "interrupt_def.h"
#include "status.h"

typedef void (*x86InterruptHandler)(uint8_t interrupt_id);

// Initializes the interrupt internals. If called multiple times the subsequent
// attempts will clear everything resulting in the need to re initialize all
// interrupts.
void x86_interrupt_init(void);

// Registers an ISR handler. The handler_id is updated to the id assigned to the
// handler if registered successfully.
StatusCode x86_interrupt_register_handler(x86InterruptHandler handler, uint8_t *handler_id);

// Registers a callback to a handler assigning it a new id from the global
// interrupt id pool.
StatusCode x86_interrupt_register_interrupt(uint8_t handler_id, const InterruptSettings *settings,
                                            uint8_t *interrupt_id);

// Triggers a software interrupt by interrupt_id.
StatusCode x86_interrupt_trigger(uint8_t interrupt_id);

// Wakes wait()
void x86_interrupt_wake(void);

// Configures the block mask on the signal handler for critical sections.
void x86_interrupt_mask(void);
void x86_interrupt_unmask(void);

// Inits the correct signal mask on a pthread.
void x86_interrupt_pthread_init(void);

bool x86_interrupt_in_handler(void);
