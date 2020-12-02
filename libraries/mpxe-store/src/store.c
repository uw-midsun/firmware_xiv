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

#include "critical_section.h"
#include "log.h"
#include "stores.pb-c.h"

#define MAX_STORE_COUNT 64
#define INVALID_STORE_ID MAX_STORE_COUNT

typedef struct Store {
  MxStoreType type;
  void *key;
  void *store;
} Store;

// Used to ensure this module is only initialized once
bool store_lib_inited = false;

static Store s_stores[MAX_STORE_COUNT];

// Child to parent fifo - used to talk to harness
static int s_ctop_fifo;

static StoreFuncs s_func_table[MX_STORE_TYPE__END];

static pthread_mutex_t s_sig_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_log_lock = PTHREAD_MUTEX_INITIALIZER;

// signal handler for catching parent
static void prv_sigusr(int signo) {
  pthread_mutex_unlock(&s_sig_lock);
}

static void prv_sigusr2(int signo) {
  pthread_mutex_unlock(&s_log_lock);
}

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
  struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
  while (true) {
    int res = poll(&pfd, 1, -1);
    if (res == -1) {
      // interrupted
      continue;
    } else if (res == 0) {
      continue;  // nothing to read
    } else {
      if (pfd.revents & POLLIN) {
        static uint8_t buf[MAX_STORE_SIZE_BYTES];
        ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
        if (len == -1) {
          LOG_DEBUG("read error while polling\n");
        }
        prv_handle_store_update(buf, len);
      } else {
        LOG_DEBUG("pollhup\n");
      }
    }
  }
  return NULL;
}

void store_config(void) {
  // do nothing after the first call
  if (store_lib_inited) {
    return;
  }

  // set up signal handler
  signal(SIGUSR1, prv_sigusr);
  signal(SIGUSR2, prv_sigusr2);

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

  store_lib_inited = true;
}

void store_register(MxStoreType type, StoreFuncs funcs, void *store, void *key) {
  if (store == NULL) {
    LOG_DEBUG("invalid store\n");
    return;
  }
  s_func_table[type] = funcs;
  // malloc a proto as a store and return a pointer to it
  Store *local_store = prv_get_first_empty();
  local_store->type = type;
  local_store->store = store;
  local_store->key = key;
}

void *store_get(MxStoreType type, void *key) {
  // just linear search for it
  // if key is NULL just get first one with right type
  for (uint16_t i = 0; i < MAX_STORE_COUNT; i++) {
    if (s_stores[i].type == type && (key == NULL || s_stores[i].key == key)) {
      return s_stores[i].store;
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

  pthread_mutex_lock(&s_sig_lock);
  // write proto to fifo
  ssize_t written = write(s_ctop_fifo, export_buf, export_size);
  // wait for signal that parent got message
  pthread_mutex_lock(&s_sig_lock);
  pthread_mutex_unlock(&s_sig_lock);

  if (written == -1) {
    LOG_DEBUG("write error while exporting\n");
  }

  // free memory
  free(export_buf);
  free(store_buf);
}

void log_mutex_lock() {
  pthread_mutex_lock(&s_log_lock);
}

void log_mutex_unlock() {
  pthread_mutex_unlock(&s_log_lock);
}
