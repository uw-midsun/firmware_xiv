#pragma once
// Interrupt initiation module which calls the appropriate setups for each
// architecture.

// Initializes interrupts. Call before any interrupts are added, will reset
// interrupts if called multiple times requiring re-initialization of all
// interrupt modules followed by re-registering all interrupts.
void interrupt_init(void);
