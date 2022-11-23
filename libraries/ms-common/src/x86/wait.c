#include "wait.h"

#include <signal.h>

void wait(void) {
  sigset_t wait_sigset;

  sigemptyset(&wait_sigset);
  sigsuspend(&wait_sigset);
}
