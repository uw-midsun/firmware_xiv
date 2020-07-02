#include "ring.h"

#include <stdlib.h>
#include <string.h>

StatusCode ring_create(size_t item_size, uint32_t buffer_size, Ring **out) {
  if (buffer_size > MAX_BUFFER_SIZE || out == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  Ring *ret = calloc(1, sizeof(Ring));
  if (ret == NULL) {
    *out = ret;
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  ret->buffer = calloc(buffer_size, item_size);
  if (ret->buffer == NULL) {
    ring_destroy(ret);
    *out = NULL;
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  ret->item_size = (uint8_t)item_size;
  ret->buffer_size = buffer_size;
  sem_init(&ret->space_count, 0, buffer_size);
  sem_init(&ret->item_count, 0, 0);
  pthread_mutex_init(&ret->mutex, NULL);
  *out = ret;
  return STATUS_CODE_OK;
}

StatusCode ring_destroy(Ring *ring) {
  if (ring == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  pthread_mutex_lock(&ring->mutex);
  if (ring->buffer != NULL) {
    free(ring->buffer);
  }
  sem_destroy(&ring->space_count);
  sem_destroy(&ring->item_count);
  pthread_mutex_unlock(&ring->mutex);
  pthread_mutex_destroy(&ring->mutex);
  free(ring);
  ring = NULL;
  return STATUS_CODE_OK;
}

// helper to do appropriate pointer math
void *prv_index_addr(Ring *ring, uint32_t index) {
  return (uint8_t *)ring->buffer + (ring->item_size * index);
}

// helper to handle lock/write pattern
void prv_lock_write(Ring *ring, void *item) {
  pthread_mutex_lock(&ring->mutex);
  memcpy(prv_index_addr(ring, ring->write_index), item, ring->item_size);
  ring->write_index = (ring->write_index + 1) % ring->buffer_size;
  ring->read_item_count++;
  pthread_mutex_unlock(&ring->mutex);
}

StatusCode ring_write(Ring *ring, void *item) {
  if (ring == NULL || item == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  int success = sem_trywait(&ring->space_count);
  if (success == 0) {
    // free space case
    prv_lock_write(ring, item);
    sem_post(&ring->item_count);
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  } else {
    // overwrite case
    prv_lock_write(ring, item);
    // we don't post sem in this case
    return STATUS_CODE_OK;
  }
}

StatusCode ring_wait_write(Ring *ring, void *item) {
  if (ring == NULL || item == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  sem_wait(&ring->space_count);
  prv_lock_write(ring, item);
  sem_post(&ring->item_count);
  return STATUS_CODE_OK;
}

StatusCode ring_try_write(Ring *ring, void *item) {
  if (ring == NULL || item == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  int success = sem_trywait(&ring->space_count);
  if (success == 0) {
    prv_lock_write(ring, item);
    sem_post(&ring->item_count);
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
}

// helper to handle lock/read pattern
void prv_lock_read(Ring *ring, void *out) {
  pthread_mutex_lock(&ring->mutex);
  void *addr = prv_index_addr(ring, ring->read_index);
  memcpy(out, addr, ring->item_size);
  ring->read_index = (ring->read_index + 1) % ring->buffer_size;
  ring->read_item_count--;
  pthread_mutex_unlock(&ring->mutex);
}

StatusCode ring_wait_read(Ring *ring, void *out) {
  if (ring == NULL || out == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  sem_wait(&ring->item_count);
  prv_lock_read(ring, out);
  sem_post(&ring->space_count);
  return STATUS_CODE_OK;
}

StatusCode ring_try_read(Ring *ring, void *out) {
  if (ring == NULL || out == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  int success = sem_trywait(&ring->item_count);
  if (success == 0) {
    prv_lock_read(ring, out);
    sem_post(&ring->space_count);
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
}

StatusCode ring_look(Ring *ring, void *out) {
  if (ring == NULL || out == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (ring->read_item_count < 1) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  pthread_mutex_lock(&ring->mutex);
  memcpy(out, prv_index_addr(ring, ring->read_index), ring->item_size);
  pthread_mutex_unlock(&ring->mutex);
  return STATUS_CODE_OK;
}
