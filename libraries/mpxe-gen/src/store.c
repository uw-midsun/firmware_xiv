#include "store.h"

#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gpio.pb-c.h"
#include "log.h"

#define MAX_STORE_COUNT 64
#define INVALID_STORE_ID MAX_STORE_COUNT

typedef struct Store {
  MxStoreType type;
  void *key;
  void *store;
} Store;

// Used to ensure this module is only initialized once
static bool s_initialized = false;

static Store s_stores[MAX_STORE_COUNT];

// Child to parent fifo - used to talk to harness
static int s_ctop_fifo;

static StoreFuncs s_func_table[MX_STORE_TYPE__END];

Store *prv_get_first_empty() {
  for (uint16_t i = 0; i < MAX_STORE_COUNT; i++) {
    if (s_stores[i].key == NULL && s_stores[i].store == NULL) {
      return &s_stores[i];
    }
  }
  return NULL;
}

static void prv_handle_store_update(uint8_t *buf, int64_t len) {
  MxStoreUpdate *update = mx_store_update__unpack(NULL, (size_t)len, buf);
  s_func_table[update->type].update_store(update->msg, update->mask);
  mx_store_update__free_unpacked(update, NULL);
}

// handles getting an update from python, runs as thread
static void *prv_poll_update(void *arg) {
  // read protos from stdin
  // compare using second proto as 'mask'
  // trigger gpio interrupt as necessary
  struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
  LOG_DEBUG("starting to poll\n");
  while (true) {
    int res = poll(&pfd, 1, -1);
    if (res == -1) {
      // LOG_DEBUG("polling error\n");
      // perror(__func__);
      // interrupted
      continue;
    } else if (res == 0) {
      continue;  // nothing to read
    } else {
      if (pfd.revents & POLLIN) {
        static uint8_t buf[4096];  // store may be bigger than this?
        int64_t len = read(STDIN_FILENO, buf, sizeof(buf));
        if (len == -1) {
          // TODO: handle read error case
        }
        LOG_DEBUG("store got buffer with len %ld\n", len);
        prv_handle_store_update(buf, len);
      } else {
        // TODO: handle POLLHUP case
      }
    }
  }
  return NULL;
}

void store_config(void) {
  // do nothing after the first call
  if (s_initialized) {
    return;
  }

  // set up polling thread
  pthread_t poll_thread;
  pthread_create(&poll_thread, NULL, prv_poll_update, NULL);

  // set up store pool
  memset(&s_stores, 0, sizeof(s_stores));

  // open fifo
  char fifo_path[64];
  snprintf(fifo_path, sizeof(fifo_path), "/tmp/%d_ctop", getpid());
  mkfifo(fifo_path, 0666);
  s_ctop_fifo = open(fifo_path, O_WRONLY);

  s_initialized = true;
}

void store_register(MxStoreType type, StoreFuncs funcs, void *store, void *key) {
  s_func_table[type] = funcs;
  // malloc a proto as a store and return a pointer to it
  Store *local_store = prv_get_first_empty();
  if (store == NULL) {
    return;
  }
  local_store->type = type;
  local_store->store = store;
  local_store->key = key;
}

void *store_get(MxStoreType type, void *key) {
  // just linear search for it
  // if key is NULL just get first one with right type
  for (uint16_t i = 0; i < MAX_STORE_COUNT; i++) {
    if (key == NULL && s_stores[i].type == type) {
      return s_stores[i].store;
    } else if (key != NULL && s_stores[i].type == type) {
      if (s_stores[i].key == key) {
        return s_stores[i].store;
      }
    }
  }
  return NULL;
}

void store_export(MxStoreType type, void *store, void *key) {
  // Serialize store to proto
  size_t packed_size = s_func_table[type].get_packed_size(store);
  uint8_t *store_buf = malloc(packed_size);
  s_func_table[type].pack(store, store_buf);

  // Set up message sent to python
  MxStoreInfo msg = MX_STORE_INFO__INIT;
  msg.key = (uint64_t)key;
  msg.type = type;
  msg.msg.data = store_buf;
  msg.msg.len = packed_size;

  // Serialize export message
  size_t export_size = mx_store_info__get_packed_size(&msg);
  uint8_t *export_buf = malloc(export_size);
  mx_store_info__pack(&msg, export_buf);

  // write proto to fifo
  ssize_t written = write(s_ctop_fifo, export_buf, export_size);
  if (written == -1) {
    // TODO: handle error
  }

  // free memory
  free(export_buf);
  free(store_buf);
}
