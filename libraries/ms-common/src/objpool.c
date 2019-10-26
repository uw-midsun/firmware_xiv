// We use a bitset to represent free nodes
#include <stdbool.h>
#include <string.h>

#include "critical_section.h"
#include "objpool.h"
#include "status.h"

#define OBJPOOL_GET(pool, index) \
  ((void *)((uint8_t *)(pool)->nodes + ((index) * (pool)->node_size)))

// Check in range and alignment
#define OBJPOOL_NODE_INVALID(pool, node)                                                      \
  ((pool)->nodes > (node) ||                                                                  \
   ((uint8_t *)(pool)->nodes + (pool)->num_nodes * (pool)->node_size) <= (uint8_t *)(node) || \
   (size_t)((uint8_t *)(node) - (uint8_t *)(pool)->nodes) % (pool)->node_size != 0)

#define OBJPOOL_GET_INDEX(pool, node) \
  ((size_t)((uint8_t *)(node) - (uint8_t *)(pool)->nodes) / (pool->node_size))

StatusCode objpool_init_verbose(ObjectPool *pool, void *nodes, size_t node_size, size_t num_nodes,
                                ObjpoolNodeInitFn init_node, void *context) {
  if (num_nodes > OBJPOOL_MAX_NODES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  memset(pool, 0, sizeof(*pool));

  pool->nodes = nodes;
  pool->context = context;
  pool->num_nodes = num_nodes;
  pool->node_size = node_size;
  pool->init_node = init_node;

  for (size_t i = 0; i < num_nodes; i++) {
    void *node = OBJPOOL_GET(pool, i);
    objpool_free_node(pool, node);
  }

  return STATUS_CODE_OK;
}

void *objpool_get_node(ObjectPool *pool) {
  bool disabled = critical_section_start();

  // Find first set bit - returns 0 if no bits are set, 1-indexed
  size_t index = (size_t)__builtin_ffsll((int32_t)pool->free_bitset);
  if (index == 0) {
    critical_section_end(disabled);
    return NULL;
  }

  pool->free_bitset &= ~((uint64_t)1 << (index - 1));

  critical_section_end(disabled);

  return OBJPOOL_GET(pool, index - 1);
}

StatusCode objpool_free_node(ObjectPool *pool, void *node) {
  bool disabled = critical_section_start();

  if (node == NULL || OBJPOOL_NODE_INVALID(pool, node)) {
    critical_section_end(disabled);
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(node, 0, pool->node_size);
  if (pool->init_node != NULL) {
    pool->init_node(node, pool->context);
  }

  pool->free_bitset |= ((uint64_t)1 << OBJPOOL_GET_INDEX(pool, node));

  critical_section_end(disabled);

  return STATUS_CODE_OK;
}
