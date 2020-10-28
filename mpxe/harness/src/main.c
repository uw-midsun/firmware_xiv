#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

#define MAX_LOG_LEN 1024  // Arbitrary
#define MAX_CMD_LEN 256

#define PROJ "can_communication"  // Temporarily hard coded

int main(void) {
  printf("Starting Harness\n");

  bool closed = false;

  char cmd[MAX_CMD_LEN];
  snprintf(cmd, sizeof(cmd),
           "make build PROJECT=can_communication PLATFORM=x86; build/bin/x86/can_communication");
  snprintf(cmd, sizeof(cmd), "make run PROJECT=%s PLATFORM=x86 DEFINE=MPXE", PROJ);

  FILE *proj_fp = popen(cmd, "r");

  char read_buf[MAX_LOG_LEN];
  memset(read_buf, 0, sizeof(read_buf));

  // Currently, don't close until we interrupt. In future this will be ended somehow.
  uint32_t num_lines = 0;
  while (true) {
    char *c = fgets(read_buf, sizeof(read_buf), proj_fp);
    if (read_buf[strlen(read_buf) - 1] == '\n') {
      printf("(Line %u) - %s", num_lines, read_buf);
      num_lines++;
      memset(read_buf, 0, sizeof(read_buf));
    }
  }

  return 0;
}
