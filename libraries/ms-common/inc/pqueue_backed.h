#pragma once
// Priority queue backed by an object pool
#include "objpool.h"
#include "pqueue.h"

typedef struct PQueueBacked {
  ObjectPool pool;
  PQueue pqueue;
  size_t elem_size;
} PQueueBacked;

// Initialize a backed pqueue given local queue node and element arrays.
// Note that the nodes array should be 1 element larger than the element array.
#define pqueue_backed_init(queue, nodes, elems)                                                \
  pqueue_backed_init_impl((queue), (nodes), (elems), SIZEOF_ARRAY(nodes), SIZEOF_ARRAY(elems), \
                          sizeof((elems)[0]))

StatusCode pqueue_backed_init_impl(PQueueBacked *queue, PQueueNode *nodes, void *elems,
                                   size_t num_nodes, size_t num_elems, size_t elem_size);

// Push a copy of the data in elem with the specified priority onto the pqueue.
StatusCode pqueue_backed_push(PQueueBacked *queue, const void *elem, uint16_t prio);

// Pop the minimum node from the pqueue and copy its data into elem.
StatusCode pqueue_backed_pop(PQueueBacked *queue, void *elem);

// Peek at the minimum node in the pqueue and copy its data into elem.
StatusCode pqueue_backed_peek(PQueueBacked *queue, void *elem);

// Returns the number of nodes currently in the queue.
size_t pqueue_backed_size(PQueueBacked *queue);
