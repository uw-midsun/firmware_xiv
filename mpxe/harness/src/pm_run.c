#include "pm_run.h"

#include <dirent.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

#define MAX_LOG_LEN 1024
#define MAX_CMD_LEN 256
#define TAG_LEN (sizeof("[12345]\n"))

typedef void (*PollEventHandler)(char *c, void *context);

typedef enum { READ_END = 0, WRITE_END, NUM_PIPE_ENDS } PipeEnd;

typedef struct ProjectStatus {
  char *name;
  bool built;
} ProjectStatus;

typedef struct Project {
  char *name;
  pid_t pid;
  FILE *r;
  FILE *w;
  struct pollfd pfd;
  pthread_mutex_t mut;
  PollEventHandler handler;
} Project;

// for keeping track of what's built
static ProjectStatus *s_statuses;
static uint16_t s_statuses_len;

// for holding handles to projects, should only be written by main thread
static Project *s_projs;
static uint16_t s_projs_len;

ProjectStatus *prv_get_status(const char *name) {
  for (int i = 0; i < s_statuses_len; i++) {
    if (strcmp(proj, s_statuses[i].name) == 0) {
      return &s_statuses[i];
    }
  }
  return NULL;
}

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

void prv_proj_handle(char *c, void *context) {
  Project *proj = context;
  printf("(pid %d) - %s\n", proj->pid, c);
  // TODO: Handle tags
}

// handler for make 'projects'
void prv_make_handle(char *c, void *context) {
  Project *make = context;
  printf("(pid %i) - %s\n", make->pid, c);
  static char *end = "make[1]: Leaving directory '/home/vagrant/shared/firmware_xiv'\n";
  int same = strcmp(c, end);
  if (same == 0) {
    int killed = kill(-pid, 9);
    pthread_exit(NULL);
  }
}

// thread worker that polls the proj fd and calls handler
void *prv_logger(void *arg) {
  Project *proj = arg;
  while (1) {
    int res = poll(&proj->pfd, 1, -1);
    if (res == -1) {
      perror(__func__);
    } else if (res == 0) {
      printf("nothing to read\n");
    } else {
      if (proj->pfd.revents & POLLIN) {
        static char buf[1024];
        char *c = fgets(buf, sizeof(buf), proj->r);
        proj->handler(c);
        memset(buf, 0, sizeof(buf));
      } else {
        free(proj);
        pthread_exit(NULL);
      }
    }
  }
}

void *prv_maker(void *arg) {
  // make the project and wait for it
  const char *proj_name = arg;
  char cmd[1024];
  snprintf(cmd, sizeof(cmd), "make build PROJECT=%s PIECE= PLATFORM=x86 DEFINE=MPXE", proj);
  Project *make_proj = malloc(sizeof(Project));
  make_proj->handler = prv_make_handle;
  int good = prv_pstart(cmd, &make_proj->pid, true, &make_proj->r, &make_proj->w);
  pthread_t tid;
  pthread_create(&tid, NULL, prv_logger, &make_proj);
  pthread_join(tid, NULL);
  free(make_proj);
  // TODO: build error handling
  printf("finished build\n");
  // mark as built
  for (int i = 0; i < s_statuses_len; i++) {
    if (strcmp(proj_name, s_statuses[i].name) == 0) {
      s_statuses[i].built = true;
    }
  }
  // TODO: output an event here to signify done
  pthread_exit(NULL);
}

void *prv_runner(void *arg) {
  Project
}

bool pm_build(const char *name, bool block) {
  ProjectStatus status = prv_get_status(name);
  if (!status || status->built) {
    return false;
  }
  pthread_t tid;
  pthread_create(&tid, NULL, prv_maker, name);
  if (block) {
    pthread_join(tid, NULL);
  }
  return true;
}

bool pm_start(const char *name, bool block) {
  // check valid arg
  ProjectStatus status = prv_get_status(name);
  if (!status || !status->built) {
    return false;
  }
  // start project
  // TODO: store project globally
  Project proj = { .handler = prv_proj_handle };
  char cmd[MAX_CMD_LEN];
  snprintf(cmd, sizeof(cmd), "build/bin/x86/%s", name);
  int good = prv_pstart(cmd, &proj->pid, false, &proj->r, &proj->w);
  pthread_t tid;
  pthread_create(&tid, NULL, prv_logger, &proj);
}

bool pm_stop(ProjectId proj_id) {
  // just call kill on the process
  // handle POLLHUP in revents
}

bool pm_init() {
  // find a list of all projects and mark as unbuilt
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
