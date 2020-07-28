#include "pm_run.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define MAX_LOG_LEN 1024
#define MAX_CMD_LEN 256
#define TAG_LEN (sizeof("[12345]\n"))

typedef enum { READ_END = 0, WRITE_END, NUM_PIPE_ENDS } PipeEnd;

typedef struct ProjectStatus {
  char *name;
  bool built;
} ProjectStatus;

typedef struct Project {
  char *name;
  pid_t pid;
  FILE *w_fd;
  FILE *r_fd;
} Project;

static ProjectStatus *s_statuses;
static uint16_t s_statuses_len;

static Project s_projbuf[PROJ_BUF_SIZE];

int prv_pstart(char *command, pid_t *pid, bool is_make, FILE **r_out, FILE **w_out) {
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

bool pm_start(const char *proj, ProjectId *proj_id) {
  // find first unused project in buffer
  ProjectId id;
  for (int i = 0; i < PROJ_BUF_SIZE; i++) {
    if (s_projbuf[i].pid == 0) {
      id = i;
      if (proj_id != NULL) {
        *proj_id = i;
      }
    }
  }

  // implement pthread to log each project
}

bool pm_init() {
  DIR *d;
  struct dirent *dir;
  d = opendir("projects");
  ProjectStatus buffer[128] = { 0 };
  uint16_t ind = 0;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_name[0] != '.') {
        buffer[ind].built = false;
        buffer[ind].name = malloc(strlen(dir->d_name));
        strcpy(buffer[ind].name, dir->d_name);
        ind++;
      }
    }
    closedir(d);
  }
  s_statuses = calloc(ind, sizeof(ProjectStatus));
  memcpy(s_statuses, buffer, ind * sizeof(ProjectStatus));
  s_statuses_len = ind;
  return true;
}
