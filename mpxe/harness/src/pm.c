#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

#define MAX_LOG_LEN 1024
#define MAX_CMD_LEN 256

typedef enum { READ_END = 0, WRITE_END, NUM_PIPE_ENDS } PipeEnd;

FILE *pstart(char *command, pid_t *pid, bool is_make) {
  pid_t child_pid;
  int fd[NUM_PIPE_ENDS];
  int success = pipe(fd);
  if (success != 0) {
    return NULL;
  }

  child_pid = fork();
  if (child_pid == -1) {
    return NULL;
  }

  // child process
  if (child_pid == 0) {
    close(fd[READ_END]);
    dup2(fd[WRITE_END], STDOUT_FILENO);

    setpgid(child_pid, child_pid);
    if (is_make) {
      execl("/bin/sh", "/bin/sh", "-c", command, (char *)NULL);
    } else {
      execl(command, command, (char *)NULL);
    }
    exit(0);
  } else {
    close(fd[WRITE_END]);
  }

  *pid = child_pid;
  return fdopen(fd[READ_END], "r");
}

void log_pstart(char *command, bool is_make) {
  pid_t pid;
  FILE *p_out = pstart(command, &pid, is_make);

  char read_buf[MAX_LOG_LEN];
  memset(read_buf, 0, sizeof(read_buf));

  while (true) {
    char *c = fgets(read_buf, sizeof(read_buf), p_out);
    if (read_buf[strlen(read_buf) - 1] == '\n') {
      printf("(pid %i) - %s", pid, read_buf);
      if (is_make) {
        int same = strcmp(read_buf, "make[1]: Leaving directory '/home/vagrant/shared/firmware_xiv'\n");
        if (same == 0) {
          int killed = kill(-pid, 9);
          return;
        }
      }
      memset(read_buf, 0, sizeof(read_buf));
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
