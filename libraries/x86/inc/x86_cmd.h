#pragma once
// Launches socket to handle external commands
// Expects space-delimited ASCII commands
// Note that this only handles one command per packet
#include "x86_socket.h"

#define X86_CMD_SOCKET_NAME "cmd"
#define X86_CMD_MAX_HANDLERS 16
#define X86_CMD_MAX_ARGS 5

typedef void (*X86CmdHandlerFn)(int client_fd, const char *cmd, const char *args[], size_t num_args,
                                void *context);

// GCC constructor attribute used to start command thread without explicitly
// calling init Note that unless a module is linked in that registers a command,
// the constructor will not run.
void x86_cmd_init(void) __attribute__((constructor));

StatusCode x86_cmd_register_handler(const char *cmd, X86CmdHandlerFn fn, void *context);
