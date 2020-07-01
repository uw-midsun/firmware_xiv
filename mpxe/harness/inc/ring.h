#pragma once

// this module implements a generic mutex-protected ring buffer

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "status.h"

#define MAX_BUFFER_SIZE 64

typedef struct Ring {
  uint8_t item_size;      // size of item
  uint32_t buffer_size;   // buffer size
  uint32_t write_index;
  uint32_t read_index;
  void *buffer;           // dynamically allocated buffer
  sem_t space_count;
  sem_t item_count;
  uint32_t read_item_count;
  pthread_mutex_t mutex;  // general protection
} Ring;

// Creates a new ring buffer.
// Params
// item_size: sizeof(item)
// buffer_size: max number of items
// out: will contain new Ring if successful, NULL if not
StatusCode ring_create(uint8_t item_size, uint32_t buffer_size, Ring **out);

// Destroys and cleans up a ring buffer.
StatusCode ring_destroy(Ring *ring);

// Writes an item to the ring buffer. Overwrites the oldest value if buffer is full.
StatusCode ring_write(Ring *ring, void *item);

// Writes an item to the ring buffer. Fails if buffer is full.
StatusCode ring_try_write(Ring *ring, void *item);

// Reads an item from the buffer and removes it.
// out will be NULL if read failed.
StatusCode ring_read(Ring *ring, void *out);

// Reads an item from the buffer but doesn't remove it.
// out will be NULL if read failed.
StatusCode ring_look(Ring *ring, void *out);
