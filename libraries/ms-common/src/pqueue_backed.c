#include "pqueue_backed.h"
#include <string.h>

static void prv_init_node(void *node, void *context) {
  PQueueBacked *queue = context;
  memset(node, 0, queue->elem_size);
}

StatusCode pqueue_backed_init_impl(PQueueBacked *queue, PQueueNode *nodes, void *elems,
                                   size_t num_nodes, size_t num_elems, size_t elem_size) {
  if (num_nodes != num_elems + 1) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  queue->elem_size = elem_size;

  pqueue_init(&queue->pqueue, nodes, num_nodes);
  objpool_init_verbose(&queue->pool, elems, elem_size, num_elems, prv_init_node, queue);

  return STATUS_CODE_OK;
}

StatusCode pqueue_backed_push(PQueueBacked *queue, const void *elem, uint16_t prio) {
  if (elem == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  void *node = objpool_get_node(&queue->pool);
  if (node == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  memcpy(node, elem, queue->elem_size);
  return pqueue_push(&queue->pqueue, node, prio);
}

StatusCode pqueue_backed_pop(PQueueBacked *queue, void *elem) {
  void *node = pqueue_pop(&queue->pqueue);
  if (node == NULL) {
    return status_code(STATUS_CODE_EMPTY);
  }

  if (elem != NULL) {
    // Support popping without copying
    memcpy(elem, node, queue->elem_size);
  }

  return objpool_free_node(&queue->pool, node);
}

StatusCode pqueue_backed_peek(PQueueBacked *queue, void *elem) {
  if (elem == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  void *node = pqueue_peek(&queue->pqueue);
  if (node == NULL) {
    return status_code(STATUS_CODE_EMPTY);
  }

  memcpy(elem, node, queue->elem_size);
  return STATUS_CODE_OK;
}

size_t pqueue_backed_size(PQueueBacked *queue) {
  return pqueue_size(&queue->pqueue);
}
