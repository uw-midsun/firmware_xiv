#pragma once

#include <stdbool.h>
#include <unistd.h>

#include "mpxe-gen/inc/stores.pb-c.h"

typedef uint16_t StoreId;

bool ds_open(pid_t pid, EnumStoreType store_type);

bool ds_close(StoreId store_id);

bool ds_set_val(StoreId store_id, void *msg, void *val);

bool ds_set(StoreId store_id, void *msg);
