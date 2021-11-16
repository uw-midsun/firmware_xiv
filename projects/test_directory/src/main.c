/*
Write a function that periodically sends a CAN message with id 0xA and a random uint16_t as the
body, and requests an ACK with an OK status. Write another function that periodically sends a CAN
message with id 0xB and a different random uint16_t as the body.

Then, register callbacks for both that print the data, but only ACK the 0xA message.

Run the program in two terminals at the same time, and send a screenshot of the output to your lead.
*/
#include <stdint.h>
#include <stdlib.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_A_WAIT_PERIOD_MS 500
#define COUNTER_B_WAIT_PERIOD_MS 1000

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback_a(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("COUNTER A:");

  soft_timer_start_millis(COUNTER_A_WAIT_PERIOD_MS, prv_timer_callback_a, context, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };

  soft_timer_start_millis(COUNTER_A_WAIT_PERIOD_MS, prv_timer_callback_a, &counters, NULL);

  while (true) {
    wait();
  }

  return 0;
}
