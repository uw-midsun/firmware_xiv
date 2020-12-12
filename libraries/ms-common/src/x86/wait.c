#include "wait.h"
#include <stdio.h>
#include <stdlib.h>
// #include <bits/types/__sigset_t.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

static sigset_t s_wait_sigset;
void wait(void) {
  sigemptyset(&s_wait_sigset);
  sigsuspend(&s_wait_sigset);
  return;
}
