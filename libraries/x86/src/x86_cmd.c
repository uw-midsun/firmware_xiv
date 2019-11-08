#include "x86_cmd.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

typedef struct X86CmdHandler {
  const char *cmd;
  X86CmdHandlerFn fn;
  void *context;
} X86CmdHandler;

typedef struct X86CmdThread {
  X86SocketThread socket;
  X86CmdHandler handlers[X86_CMD_MAX_HANDLERS];
  size_t num_handlers;
} X86CmdThread;

static X86CmdThread s_cmd_thread;

static void prv_socket_handler(struct X86SocketThread *thread, int client_fd, const char *rx_data,
                               size_t rx_len, void *context) {
  X86CmdThread *cmd_thread = context;

  LOG_DEBUG("Received command %s\n", rx_data);

  // Data will be mangled by strtok - make sure we always end with a null
  // terminator
  char *buf = malloc(rx_len + 1);
  buf[rx_len] = '\0';

  const char *args[X86_CMD_MAX_ARGS] = { 0 };
  strncpy(buf, rx_data, rx_len);

  // Don't support more than one command per packet
  char *save_ptr = NULL;
  const char *cmd_word = strtok_r(buf, " \n", &save_ptr);
  size_t num_args = 0;
  while (num_args < X86_CMD_MAX_ARGS &&
         (args[num_args] = strtok_r(NULL, " \n", &save_ptr)) != NULL) {
    num_args++;
  }

  for (size_t i = 0; i < cmd_thread->num_handlers; i++) {
    if (strcmp(cmd_thread->handlers[i].cmd, cmd_word) == 0) {
      cmd_thread->handlers[i].fn(client_fd, cmd_word, args, num_args,
                                 cmd_thread->handlers[i].context);
      break;
    }
  }

  free(buf);
}

void x86_cmd_init(void) {
  x86_socket_init(&s_cmd_thread.socket, X86_CMD_SOCKET_NAME, prv_socket_handler, &s_cmd_thread);
}

StatusCode x86_cmd_register_handler(const char *cmd, X86CmdHandlerFn fn, void *context) {
  X86CmdThread *thread = &s_cmd_thread;

  if (thread->num_handlers >= X86_CMD_MAX_HANDLERS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  thread->handlers[thread->num_handlers] = (X86CmdHandler){
    .cmd = cmd,         //
    .fn = fn,           //
    .context = context  //
  };
  thread->num_handlers++;

  return STATUS_CODE_OK;
}
