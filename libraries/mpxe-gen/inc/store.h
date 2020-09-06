#pragma once

#include "stores.pb-c.h"

typedef size_t (*GetPackedSizeFunc)(const void *msg);
typedef size_t (*PackFunc)(const void *msg, uint8_t *out);
typedef void *(*UnpackFunc)(ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
typedef void (*FreeUnpackedFunc)(void *msg, ProtobufCAllocator *allocator);
typedef void (*UpdateStoreFunc)(ProtobufCBinaryData msg, ProtobufCBinaryData mask);

typedef struct StoreFuncs {
  GetPackedSizeFunc get_packed_size;
  PackFunc pack;
  UnpackFunc unpack;
  FreeUnpackedFunc free_unpacked;
  UpdateStoreFunc update_store;
} StoreFuncs;

// Initialize store, call from every driver and just do nothing on calls after the first
void store_init(EnumStoreType type, StoreFuncs funcs);

// Allocate a store specific to the driver type. Returns the store pointer.
// If key is NULL, the store is deemed unique
void store_register(EnumStoreType type, void *store, void *key);

// Gets the matching pointer for the type.
// If key is NULL, it's assumed the store is unique.
void *store_get(EnumStoreType type, void *key);

// Call at every update to export 
void store_export(EnumStoreType type, void *store, void *key);
