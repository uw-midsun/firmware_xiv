// This is just a wrapper for a backed priority queue.
// Currently, there is only one global event queue.
#include <stdbool.h>
#include <string.h>

#include "event_queue.h"
#include "fifo.h"
#include "status.h"

typedef struct EventQueue {
  Fifo fifos[NUM_EVENT_PRIORITIES];
  Event event_nodes[NUM_EVENT_PRIORITIES][EVENT_QUEUE_SIZE];
} EventQueue;

static EventQueue s_queue;

void event_queue_init(void) {
  for (size_t i = 0; i < NUM_EVENT_PRIORITIES; i++) {
    fifo_init(&s_queue.fifos[i], s_queue.event_nodes[i]);
  }
}

StatusCode event_raise_priority(EventPriority priority, EventId id, uint16_t data) {
  if (priority >= NUM_EVENT_PRIORITIES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  Event e = {
    .id = id,      //
    .data = data,  //
  };

  return fifo_push(&s_queue.fifos[priority], &e);
}

StatusCode event_process(Event *e) {
  for (size_t i = 0; i < NUM_EVENT_PRIORITIES; i++) {
    if (s_queue.fifos[i].num_elems > 0) {
      return fifo_pop(&s_queue.fifos[i], e);
    }
  }
  return status_code(STATUS_CODE_EMPTY);
}
