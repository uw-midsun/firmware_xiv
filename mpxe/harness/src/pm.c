#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "pm_parse.h"

#define MAX_LOG_LEN 1024
#define MAX_CMD_LEN 256
#define TAG_LEN (sizeof("[12345]\n"))

typedef enum { READ_END = 0, WRITE_END, NUM_PIPE_ENDS } PipeEnd;

int pstart(char *command, pid_t *pid, bool is_make, FILE **r_out, FILE **w_out) {
  pid_t child_pid;
  int in_pipe[NUM_PIPE_ENDS];
  int out_pipe[NUM_PIPE_ENDS];
  int success = pipe(in_pipe);
  success = pipe(out_pipe);
  (void)success;

  child_pid = fork();
  if (child_pid == -1) {
    return -1;
  }

  // child process
  if (child_pid == 0) {
    dup2(in_pipe[READ_END], STDIN_FILENO);
    dup2(out_pipe[WRITE_END], STDOUT_FILENO);
    close(out_pipe[READ_END]);
    close(in_pipe[WRITE_END]);

    if (is_make) {
      execl("/bin/sh", "/bin/sh", "-c", command, (char *)NULL);
    } else {
      execl(command, command, (char *)NULL);
    }
    exit(0);
  }

  *pid = child_pid;
  *r_out = fdopen(out_pipe[READ_END], "r");
  *w_out = fdopen(in_pipe[WRITE_END], "w");
  close(out_pipe[WRITE_END]);
  close(in_pipe[READ_END]);
  return 0;
}

void log_pstart(char *command, bool is_make) {
  pid_t pid;
  FILE *child_pipe[NUM_PIPE_ENDS];
  pstart(command, &pid, is_make, &child_pipe[READ_END], &child_pipe[WRITE_END]);

  char read_buf[MAX_LOG_LEN];
  memset(read_buf, 0, sizeof(read_buf));

  fprintf(child_pipe[WRITE_END], "asdf ");
  fflush(child_pipe[WRITE_END]);

  char tag[TAG_LEN] = { 0 };
  snprintf(tag, sizeof(tag), "[%d]\n", pid);

  while (true) {
    char *c = fgets(read_buf, sizeof(read_buf), child_pipe[READ_END]);
    if (read_buf[strlen(read_buf) - 1] == '\n') {
      if (is_make) {
        int same =
            strcmp(read_buf, "make[1]: Leaving directory '/home/vagrant/shared/firmware_xiv'\n");
        printf("(pid %i) - %s", pid, read_buf);
        if (same == 0) {
          int killed = kill(-pid, 9);
          return;
        }
      } else {
        printf("(pid %i) - %s", pid, read_buf);
        if (strlen(read_buf) >= TAG_LEN) {
          char tag_buf[TAG_LEN];
          strcpy(tag_buf, &read_buf[strlen(read_buf) - TAG_LEN + 1]);
          int same = strcmp(tag_buf, tag);
          if (strcmp(tag, tag_buf) == 0) {
            int opcode = pm_parse(read_buf);
          }
        }
      }
      memset(read_buf, 0, sizeof(read_buf));
      // else {
      //   int same = strcmp(read_buf, "[0] libraries/ms-common/src/x86/gpio.c:95: gpio
      //   initializing...\n"); if (same == 0) {
      //     printf("same! sending!\n");
      //     fprintf(child_pipe[WRITE_END], "get this!\n");
      //     fflush(child_pipe[WRITE_END]);
      //     printf("finished sending!\n");
      //   }
      // }
    }
  }
}

void build_project(char *name) {
  char cmd[MAX_CMD_LEN];
  snprintf(cmd, sizeof(cmd), "make build PROJECT=%s PIECE= PLATFORM=x86 DEFINE=MPXE", name);
  printf("BUILD PROJECT COMMAND: %s\n", cmd);
  log_pstart(cmd, true);
}

void run_project(char *name) {
  char cmd[MAX_CMD_LEN];
  snprintf(cmd, sizeof(cmd), "build/bin/x86/%s", name);
  log_pstart(cmd, false);
}
