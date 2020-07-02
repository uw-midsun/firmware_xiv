#pragma once

// this module implements a generic mutex-protected ring buffer

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include "status.h"

#define MAX_BUFFER_SIZE 128

typedef struct Ring {
  uint8_t item_size;     // size of item
  uint32_t buffer_size;  // buffer size
  uint32_t write_index;
  uint32_t read_index;
  void *buffer;  // dynamically allocated buffer
  sem_t space_count;
  sem_t item_count;
  uint32_t read_item_count;
  pthread_mutex_t mutex;  // general protection
} Ring;

// Creates a new ring buffer.
StatusCode ring_create(size_t item_size, uint32_t buffer_size, Ring **out);

// Destroys and cleans up a ring buffer.
StatusCode ring_destroy(Ring *ring);

// Writes an item to the ring buffer. Overwrites the oldest value if buffer is full.
StatusCode ring_write(Ring *ring, void *item);

// Writes an item to the ring buffer. Fails if buffer is full.
StatusCode ring_try_write(Ring *ring, void *item);

// Writes an item to the ring buffer. Waits if buffer is full.
StatusCode ring_wait_write(Ring *ring, void *item);

// Reads an item from the buffer and removes it. Waits if buffer is empty.
StatusCode ring_wait_read(Ring *ring, void *out);

// Reads an item from the buffer and removes it. Fails if buffer is empty.
StatusCode ring_try_read(Ring *ring, void *item);

// Reads an item from the buffer but doesn't remove it.
StatusCode ring_look(Ring *ring, void *out);
