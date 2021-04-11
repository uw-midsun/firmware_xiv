#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// Contrived example of where an ISR (the soft timer interrupt) has a larger max stack frame trace
// than main.

#define CONTRIVED_ARRAY_SIZE 0x80

static void prv_soft_timer_callback(SoftTimerId timer_id, void *context) {
  char mystr[CONTRIVED_ARRAY_SIZE] = { '\0' };
  for (size_t i = 0; i < CONTRIVED_ARRAY_SIZE; i++) {
    mystr[i] = 'a' + (i % 26);
  }
  mystr[CONTRIVED_ARRAY_SIZE - 1] = '\0';
  LOG_DEBUG("%s\n", mystr);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  soft_timer_start_millis(100, prv_soft_timer_callback, NULL, NULL);
  LOG_DEBUG("I'm bigger than git_version_init now\n");
  while (true) {
    wait();
  }
  return 0;
}
