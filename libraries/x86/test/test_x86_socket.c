#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "log.h"
#include "test_helpers.h"
#include "unity.h"
#include "x86_socket.h"

#define TEST_X86_SOCKET_NAME "test_x86_socket"
#define TEST_X86_EXPECTED_RESPONSE "Success!\n"

extern char *program_invocation_short_name;
static X86SocketThread s_thread;
static char s_rx_data[30];
static size_t s_rx_len;

static void prv_handler(X86SocketThread *thread, int client_fd, const char *rx_data, size_t rx_len,
                        void *context) {
  bool *received = context;

  // Write response to client
  const char *response = TEST_X86_EXPECTED_RESPONSE;
  x86_socket_write(client_fd, response, strlen(response));

  // Print received data
  LOG_DEBUG("handling RX (%ld bytes)\n", rx_len);
  fwrite(rx_data, sizeof(char), rx_len, stdout);

  strncpy(s_rx_data, rx_data, sizeof(s_rx_data));
  s_rx_len = rx_len;
  *received = true;
}

void setup_test(void) {
  memset(s_rx_data, 0, sizeof(s_rx_data));
  s_rx_len = 0;
}

void teardown_test(void) {}

void test_x86_socket_connect(void) {
  volatile bool received = false;
  TEST_ASSERT_OK(x86_socket_init(&s_thread, TEST_X86_SOCKET_NAME, prv_handler, &received));

  int client_fd = test_x86_socket_client_init(TEST_X86_SOCKET_NAME);

  // Sending message to socket and block until the handler is run
  const char *msg = "This is a test!\n";
  LOG_DEBUG("Sending %s (%d bytes)\n", msg, strlen(msg));
  TEST_ASSERT_OK(x86_socket_write(client_fd, msg, strlen(msg)));

  while (!received) {
  }

  TEST_ASSERT_EQUAL_STRING(msg, s_rx_data);
  TEST_ASSERT_EQUAL(strlen(msg), s_rx_len);

  char buffer[50] = { 0 };
  ssize_t read_len = read(client_fd, buffer, sizeof(buffer));
  LOG_DEBUG("Response: %s\n", buffer);
  TEST_ASSERT_EQUAL_STRING(TEST_X86_EXPECTED_RESPONSE, buffer);
  TEST_ASSERT_EQUAL(strlen(TEST_X86_EXPECTED_RESPONSE), read_len);
}
