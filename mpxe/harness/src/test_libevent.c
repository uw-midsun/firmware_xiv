#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum { READ_END = 0, WRITE_END, NUM_PIPE_ENDS } PipeEnd;

void do_thing() {
  pid_t child_pid;
  int out_pipe[NUM_PIPE_ENDS];
  int success = pipe(out_pipe);
  if (success == -1) {
    perror("pipe");
    exit(0);
  }

  child_pid = fork();

  if (child_pid == 0) {
    dup2(out_pipe[WRITE_END], STDOUT_FILENO);
    close(out_pipe[READ_END]);

    while (1) {
    //   printf("child print\n");
    //   fflush(stdout);
    //   sleep(1);
      execl("/bin/sh", "/bin/sh", "-c", "make build PROJECT=can_communication PLATFORM=x86 DEFINE=MPXE", (char *)NULL);
    }
  }

  FILE *fp = fdopen(out_pipe[READ_END], "r");
  if (fp == NULL) {
    perror("blep");
    exit(0);
  }
  close(out_pipe[WRITE_END]);

//   int useless[2];
//   int u = pipe(useless);
//   if (u == -1) perror("useless pipe");
//   char msg[] = "writing to useless\n";
//   int test = write(useless[1], msg, sizeof(msg));
//   if (test < 0) perror("write in do_thing");
//   printf("fgetsing:\n");
//   FILE *useless_read = fdopen(useless[0], "r");
//   char ubuf[1024] = { 0 };
//   char *uc = fgets(ubuf, sizeof(ubuf), useless_read);
//   printf("gotten from pipe: %s\n", uc);

  struct pollfd to_poll = {
    .fd = out_pipe[READ_END],
    .events = POLLIN,
  };

  while (1) {
    int res = poll(&to_poll, 1, -1);
    if (res == -1) {
      perror("poll");
      exit(0);
    } else if (res == 0) {
      printf("nothing occured\n");
    } else {
      char read_buf[1024];
      char *c = fgets(read_buf, sizeof(read_buf), fp);
      if (c != NULL) {
        printf("parent reads: %s\n", c);
      }
      memset(read_buf, 0, sizeof(read_buf));
    }
  }
}