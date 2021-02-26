#include "wait.h"

#include <signal.h>

void wait(void) {
  sigset_t s_wait_sigset;

  sigemptyset(&s_wait_sigset);
  sigsuspend(&s_wait_sigset);
}
