#include "x86_interrupt.h"

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "interrupt_def.h"
#include "log.h"
#include "status.h"

#define NUM_X86_INTERRUPT_HANDLERS 64
#define NUM_X86_INTERRUPT_INTERRUPTS 128

typedef enum {
  X86_INTERRUPT_STATE_NONE = 0,
  X86_INTERRUPT_STATE_MASK,
  X86_INTERRUPT_STATE_UNMASK,
} X86InterruptState;

typedef struct Interrupt {
  InterruptPriority priority;
  uint8_t handler_id;
  bool is_event;
} Interrupt;

static bool s_in_handler_flag = false;
static X86InterruptState s_interrupt_state_update = X86_INTERRUPT_STATE_NONE;

static pid_t s_pid = 0;

static uint8_t s_x86_interrupt_next_interrupt_id = 0;
static uint8_t s_x86_interrupt_next_handler_id = 0;

static Interrupt s_x86_interrupt_interrupts_map[NUM_X86_INTERRUPT_INTERRUPTS];
static x86InterruptHandler s_x86_interrupt_handlers[NUM_X86_INTERRUPT_HANDLERS];

// Signal handler for all interrupts. Prioritization is handled by the
// implementation of signals and the init function. Signals of higher priority
// interrupt the running of this function. All other signals are stored in a
// pqueue and are executed in order of priority then arrival. Runs the handler
// associated with the interrupt id it receives via the sival_int.
static void prv_sig_handler(int signum, siginfo_t *info, void *ptr) {
  (void)signum;
  (void)ptr;
  s_in_handler_flag = true;
  if (info->si_value.sival_int < NUM_X86_INTERRUPT_INTERRUPTS) {
    // If the interrupt is an event don't run the handler as it is just a wake
    // event.
    if (!s_x86_interrupt_interrupts_map[info->si_value.sival_int].is_event) {
      // Execute the handler passing it the interrupt ID. To determine which
      // handler look up in the interrupts map by interrupt ID.
      s_x86_interrupt_handlers[s_x86_interrupt_interrupts_map[info->si_value.sival_int].handler_id](
          info->si_value.sival_int);
    }
  }
  s_in_handler_flag = false;
}

// Blocks all interrupts (excluding signals to block/unblock interrupts) when
// triggered. Should use signal number |SIGRTMIN + NUM_INTERRUPT_PRIORITIES| to
// trigger. This has to be run as a signal handler since a thread can only
// manipulate its own signal mask or its children up until the point they are
// spawned. As a result the signal handling thread MUST be interrupted in order
// to change its signal mask in the event of a critical section.
static void prv_sig_state_handler(int signum, siginfo_t *info, void *ptr) {
  (void)signum;
  // We actually need to manipulate the block mask of the context that is
  // restored for this thread. So we alter |ctx| rather than directly calling
  // |pthread_sigmask| as this gets overridden on context switch after exiting
  // the handler.
  ucontext_t *ctx = ptr;

  // Based on the sival_int change the state of interrupts. If invalid number
  // silently ignore.
  if (info->si_value.sival_int == X86_INTERRUPT_STATE_MASK) {
    sigaddset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
    sigaddset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
    sigaddset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  } else if (info->si_value.sival_int == X86_INTERRUPT_STATE_UNMASK) {
    sigdelset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
    sigdelset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
    sigdelset(&ctx->uc_sigmask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  }
}

void x86_interrupt_init(void) {
  // Assign the s_pid to be the process id handling the interrupts. This
  // prevents subprocesses from sending a signal to itself instead.
  s_pid = getpid();

  // Create a handler sigaction.
  struct sigaction act;
  act.sa_sigaction = prv_sig_handler;
  act.sa_flags = SA_SIGINFO | SA_RESTART;  // Set SA_RESTART to allow syscalls to be retried.

  // Define an empty blocking mask (no signals are blocked to start).
  sigset_t block_mask;
  sigemptyset(&block_mask);

  // Add a rule for low priority interrupts which blocks only other low priority
  // signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_LOW, &act, NULL);

  // Add a rule for normal priority interrupts which blocks low and other normal
  // priority signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_NORMAL, &act, NULL);

  // Add a rule for high priority interrupts which blocks all other interrupt
  // signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_HIGH, &act, NULL);

  // Add a rule for the masking (critical sectioning) of the signals.
  act.sa_mask = block_mask;
  act.sa_sigaction = prv_sig_state_handler;
  sigaction(SIGRTMIN + NUM_INTERRUPT_PRIORITIES, &act, NULL);

  // Clear statics.
  s_interrupt_state_update = X86_INTERRUPT_STATE_NONE;
  s_in_handler_flag = false;
  s_x86_interrupt_next_interrupt_id = 0;
  s_x86_interrupt_next_handler_id = 0;
  memset(&s_x86_interrupt_interrupts_map, 0, sizeof(s_x86_interrupt_interrupts_map));
  memset(&s_x86_interrupt_handlers, 0, sizeof(s_x86_interrupt_handlers));
}

StatusCode x86_interrupt_register_handler(x86InterruptHandler handler, uint8_t *handler_id) {
  if (s_x86_interrupt_next_handler_id >= NUM_X86_INTERRUPT_HANDLERS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *handler_id = s_x86_interrupt_next_handler_id;
  s_x86_interrupt_next_handler_id++;
  s_x86_interrupt_handlers[*handler_id] = handler;

  return STATUS_CODE_OK;
}

StatusCode x86_interrupt_register_interrupt(uint8_t handler_id, const InterruptSettings *settings,
                                            uint8_t *interrupt_id) {
  if (handler_id >= s_x86_interrupt_next_handler_id ||
      settings->priority >= NUM_INTERRUPT_PRIORITIES || settings->type >= NUM_INTERRUPT_TYPES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_x86_interrupt_next_interrupt_id >= NUM_X86_INTERRUPT_INTERRUPTS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *interrupt_id = s_x86_interrupt_next_interrupt_id;
  s_x86_interrupt_next_interrupt_id++;
  Interrupt interrupt = {
    .priority = settings->priority, .handler_id = handler_id, .is_event = (bool)settings->type
  };
  s_x86_interrupt_interrupts_map[*interrupt_id] = interrupt;

  return STATUS_CODE_OK;
}

StatusCode x86_interrupt_trigger(uint8_t interrupt_id) {
  if (interrupt_id >= s_x86_interrupt_next_interrupt_id) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Enqueue a new signal sent to this process that has a signal number
  // determined by the id for the callback it is going to run.
  siginfo_t value_store;
  value_store.si_value.sival_int = interrupt_id;
  sigqueue(s_pid, SIGRTMIN + (int)s_x86_interrupt_interrupts_map[interrupt_id].priority,
           value_store.si_value);

  return STATUS_CODE_OK;
}

void x86_interrupt_pthread_init(void) {
  sigset_t block_mask;
  sigemptyset(&block_mask);
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  sigaddset(&block_mask, SIGRTMIN + NUM_INTERRUPT_PRIORITIES);
  pthread_sigmask(SIG_BLOCK, &block_mask, NULL);
}

void x86_interrupt_mask(void) {
  siginfo_t value_store;
  value_store.si_value.sival_int = X86_INTERRUPT_STATE_MASK;
  sigqueue(s_pid, SIGRTMIN + NUM_INTERRUPT_PRIORITIES, value_store.si_value);
}

void x86_interrupt_unmask(void) {
  siginfo_t value_store;
  value_store.si_value.sival_int = X86_INTERRUPT_STATE_UNMASK;
  sigqueue(s_pid, SIGRTMIN + NUM_INTERRUPT_PRIORITIES, value_store.si_value);
}

void x86_interrupt_wake(void) {
  siginfo_t value_store;
  uint8_t interrupt_id = NUM_X86_INTERRUPT_INTERRUPTS;
  value_store.si_value.sival_int = interrupt_id;
  sigqueue(s_pid, SIGRTMIN + INTERRUPT_PRIORITY_HIGH, value_store.si_value);
}

bool x86_interrupt_in_handler(void) {
  return s_in_handler_flag;
}
