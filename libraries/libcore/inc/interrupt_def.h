#pragma once
// Core interrupt enum definitions shared across architectures.

// The interrupt type runs a callback as soon as the interrupt is triggered. The
// event type simply wakes the device without running a callback.
typedef enum {
  INTERRUPT_TYPE_INTERRUPT = 0,
  INTERRUPT_TYPE_EVENT,
  NUM_INTERRUPT_TYPES,
} InterruptType;

typedef enum {
  INTERRUPT_PRIORITY_HIGH = 0,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_PRIORITY_LOW,
  NUM_INTERRUPT_PRIORITIES,
} InterruptPriority;

// Defines on what edge of an input signal the interrupt triggers on. This is
// not necessarily applicable for all interrupts; however, external interrupts
// will use them.
typedef enum {
  INTERRUPT_EDGE_RISING = 0,
  INTERRUPT_EDGE_FALLING,
  INTERRUPT_EDGE_RISING_FALLING,
  NUM_INTERRUPT_EDGES,
} InterruptEdge;

typedef struct InterruptSettings {
  InterruptType type;
  InterruptPriority priority;
} InterruptSettings;
