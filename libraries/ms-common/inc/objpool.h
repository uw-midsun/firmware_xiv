#pragma once
// Object Pool Interface
//
// Manages a pre-allocated array of objects. We use this instead of a heap so we
// don't need to deal with memory fragmentation.
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "misc.h"
#include "status.h"

#define OBJPOOL_MAX_NODES 64

// Function to initialize nodes with
typedef void (*ObjpoolNodeInitFn)(void *node, void *context);

typedef struct ObjectPool {
  void *nodes;
  void *context;
  ObjpoolNodeInitFn init_node;
  size_t num_nodes;
  size_t node_size;
  uint64_t free_bitset;
} ObjectPool;

// Initializes an object pool given a local array (i.e. not a pointer)
#define objpool_init(pool, nodes, init_fn, context)                                           \
  objpool_init_verbose((pool), (nodes), sizeof((nodes)[0]), SIZEOF_ARRAY((nodes)), (init_fn), \
                       (context))

// Initializes an object pool. The specified context is provided for node
// initialization.
StatusCode objpool_init_verbose(ObjectPool *pool, void *nodes, size_t node_size, size_t num_nodes,
                                ObjpoolNodeInitFn init_node, void *context);

// Returns the pointer to an object from the pool.
void *objpool_get_node(ObjectPool *pool);

// Releases the specified node
StatusCode objpool_free_node(ObjectPool *pool, void *node);
