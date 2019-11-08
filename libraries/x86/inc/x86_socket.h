#pragma once
// Creates server thread for abstract unix domain socket on
// @pid/progname/module_name
#include <pthread.h>
#include <stddef.h>
#include "status.h"

#define X86_SOCKET_MAX_CLIENTS 5
#define X86_SOCKET_MAX_PENDING_CONNECTIONS 3
#define X86_SOCKET_RX_BUFFER_LEN 1024

// client_fd provided to allow reply to specific client
struct X86SocketThread;
typedef void (*X86SocketHandler)(struct X86SocketThread *thread, int client_fd, const char *rx_data,
                                 size_t rx_len, void *context);

typedef struct X86SocketThread {
  pthread_t thread;
  pthread_barrier_t barrier;
  pthread_mutex_t keep_alive;
  const char *module_name;
  X86SocketHandler handler;
  void *context;

  int client_fds[X86_SOCKET_MAX_CLIENTS];
} X86SocketThread;

// Initializes server thread on @[pid]/[progname]/module_name
// Registered handler is called when client data is received
StatusCode x86_socket_init(X86SocketThread *thread, char *module_name, X86SocketHandler handler,
                           void *context);

// Write to all connected clients
StatusCode x86_socket_broadcast(X86SocketThread *thread, const char *tx_data, size_t tx_len);

// Write to specific client
StatusCode x86_socket_write(int client_fd, const char *tx_data, size_t tx_len);

// Creates a client that connects to the specified module socket, returning the
// file descriptor
int test_x86_socket_client_init(const char *module_name);
