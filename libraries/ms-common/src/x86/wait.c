#include "wait.h"

#include <signal.h>

static sigset_t s_wait_sigset;
void wait(void) {
  sigemptyset(&s_wait_sigset);
  sigsuspend(&s_wait_sigset);
  return;
}
