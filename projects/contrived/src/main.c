#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "analyzestack.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// Contrived example of where an ISR (the soft timer interrupt) has a larger max stack frame trace
// than main.

#define CONTRIVED_ARRAY_SIZE 0x80

static void (*s_foo)(void);

static void prv_soft_timer_callback(SoftTimerId timer_id, void *context) {
  char mystr[CONTRIVED_ARRAY_SIZE] = { '\0' };
  for (size_t i = 0; i < CONTRIVED_ARRAY_SIZE; i++) {
    mystr[i] = 'a' + (i % 26);
  }
  mystr[CONTRIVED_ARRAY_SIZE - 1] = '\0';
  LOG_DEBUG("%s\n", mystr);
}

static void __attribute__((noinline)) prv_uses_function_pointer() {
  ANALYZESTACK_ALIAS("testalias")
  s_foo();
}

static void __attribute__((noinline)) prv_foobar(void) {
  LOG_DEBUG("hi!\n");
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  soft_timer_start_millis(100, prv_soft_timer_callback, NULL, NULL);
  s_foo = prv_foobar;
  prv_uses_function_pointer();
  while (true) {
    wait();
  }
  return 0;
}
