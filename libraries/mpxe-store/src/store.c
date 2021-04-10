#include "store.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "delay.h"

#include "critical_section.h"
#include "log.h"
#include "stores.pb-c.h"
#include "x86_interrupt.h"

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
static StoreFuncs s_func_table[MX_STORE_TYPE__END];

static pthread_mutex_t s_sig_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_init_lock = PTHREAD_MUTEX_INITIALIZER;

static MxLog s_mxlog = MX_LOG__INIT;

static bool s_init_cond_complete = false;
static MxStoreUpdate *s_init_cond[MAX_STORE_COUNT];
static int s_num_init_conds;

// MxCmd callback table and function prototypes
// If you are adding a command, you must declare it below and add it to the lookup table
static void prv_handle_finish_conditions(void *context);
static MxCmdCallback s_cmd_cb_lookup[MX_CMD_TYPE__NUM_CMDS] = {
  [MX_CMD_TYPE__FINISH_INIT_CONDS] = prv_handle_finish_conditions,
};

static void prv_handle_finish_conditions(void *context) {
  s_init_cond_complete = true;
  pthread_mutex_unlock(&s_init_lock);
}

// signal handler for catching parent
static void prv_sigusr(int signo) {
  MXDBG("export unlocking in signal handler, signal: %d\n", signo);
  pthread_mutex_unlock(&s_sig_lock);
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
  // Handle command with respective function in lookup table
  if (update->type == MX_STORE_TYPE__CMD) {
    MxCmd *cmd = mx_cmd__unpack(NULL, (size_t)update->msg.len, update->msg.data);
    if (cmd->cmd < MX_CMD_TYPE__NUM_CMDS && s_cmd_cb_lookup[cmd->cmd] != NULL) {
      MXDBG("cmd %d sent\n", cmd->cmd);
      s_cmd_cb_lookup[cmd->cmd](NULL);
      MXDBG("cmd handled\n");
      mx_cmd__free_unpacked(cmd, NULL);
    } else {
      LOG_DEBUG("INVALID COMMAND SENT!\n");
    }
  } else {
    if (s_init_cond_complete) {  // Default activity, call update store for type
      MXDBG("update %d got\n", update->type);
      s_func_table[update->type].update_store(update->msg, update->mask, (void *)update->key);
      MXDBG("update handled\n");
      mx_store_update__free_unpacked(update, NULL);
      MXDBG("update freed\n");
    } else {  // Store initial conditions to be used in store_register
      if (s_num_init_conds < MAX_STORE_COUNT) {
        s_init_cond[s_num_init_conds] = update;
        s_num_init_conds++;
      } else {
        MXDBG("no good\n");
        LOG_WARN("MPXE INITIAL CONDITIONS OVERFLOW!\n");
        MXDBG("overflow\n");
      }
    }
  }
}

// handles getting an update from python, runs as thread
static void *prv_poll_update(void *arg) {
  pid_t ppid = getppid();
  x86_interrupt_pthread_init();

  // read protos from stdin
  // compare using second proto as 'mask'
  struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN | POLLPRI };
  while (true) {
    MXDBG("POLL_THREAD polling!!\n");
    int res = poll(&pfd, 1, 1000);
    MXDBG("POLL_THREAD poll returned %d\n", res);
    if (res == -1) {
      // interrupted
      MXDBG("POLL_THREAD polling interrupted, errno %d, eintr %d\n", errno, EINTR);
      continue;
    } else if (res == 0) {
      continue;  // nothing to read
      MXDBG("POLL_THREAD nothing to read\n");
    } else {
      if (pfd.revents & POLLIN) {
        static uint8_t buf[MAX_STORE_SIZE_BYTES];
        MXDBG("POLL_THREAD reading\n");
        ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
        if (len == -1) {
          LOG_DEBUG("POLL_THREAD read error while polling\n");
        }
        MXDBG("POLL_THREAD before handling\n");
        prv_handle_store_update(buf, len);
        // Signal parent process after poll thread created
        MXDBG("POLL_THREAD After handling\n");

        kill(ppid, SIGUSR2);
        MXDBG("POLL_THREAD after kill signal\n");
      } else {
        MXDBG("POLL_THREAD pollhup\n");
        LOG_DEBUG("pollhup\n");
      }
    }
  }
  MXDBG("returning from poll???\n");
  return NULL;
}

void store_config(void) {
  // do nothing after the first call
  if (store_lib_inited) {
    return;
  }

  // set up signal handler
  signal(SIGUSR1, prv_sigusr);

  // set up store pool
  memset(&s_stores, 0, sizeof(s_stores));

  // set up functions for logging
  StoreFuncs log_funcs = {
    (GetPackedSizeFunc)mx_log__get_packed_size,
    (PackFunc)mx_log__pack,
    (UnpackFunc)NULL,
    (FreeUnpackedFunc)NULL,
    (UpdateStoreFunc)NULL,
  };
  store_register(MX_STORE_TYPE__LOG, log_funcs, &s_mxlog, NULL);
  // set up polling thread
  // struct sigaction action = {.sa_handler = prv_sigusr, .sa_flags = SA_RESTART};
  // sigaction(SIGUSR1, &action, NULL);
  pthread_t poll_thread;
  pthread_create(&poll_thread, NULL, prv_poll_update, NULL);

  // Lock for initial conditions, continue when FINISH_INIT_CONDS recv'd
  pthread_mutex_lock(&s_init_lock);
  pthread_mutex_lock(&s_init_lock);
  pthread_mutex_unlock(&s_init_lock);

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

  // Need to check each index, as freed and set to NULL once used
  for (uint16_t i = 0; i < MAX_STORE_COUNT; i++) {
    if (s_init_cond[i] != NULL) {
      if (s_init_cond[i]->type == type && (void *)(intptr_t)s_init_cond[i]->key == key) {
        funcs.update_store(s_init_cond[i]->msg, s_init_cond[i]->mask, key);
        mx_store_update__free_unpacked(s_init_cond[i], NULL);
        s_init_cond[i] = NULL;
      }
    }
  }
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
  MXDBG("Store export starting, type: %d\n", type);
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

  MXDBG("export locking\n");
  pthread_mutex_lock(&s_sig_lock);
  // write proto to fifo
  MXDBG("export writing\n");
  ssize_t written = write(STDOUT_FILENO, export_buf, export_size);
  // wait for signal that parent got message
  MXDBG("export locking again\n");
  pthread_mutex_lock(&s_sig_lock);
  pthread_mutex_unlock(&s_sig_lock);
  MXDBG("export unlocking\n");

  if (written == -1) {
    LOG_DEBUG("write error while exporting\n");
  }

  // free memory
  free(export_buf);
  free(store_buf);
}

void log_export(char *buf, uint16_t len) {
  store_config();
  s_mxlog.log.len = len;
  s_mxlog.log.data = (uint8_t *)buf;
  store_export(MX_STORE_TYPE__LOG, &s_mxlog, NULL);
}
