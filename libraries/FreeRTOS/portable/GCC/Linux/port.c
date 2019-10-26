#include "FreeRTOS.h"
#include "task.h"

// This provides a stubbed out minimal implementation of a FreeRTOS Linux port
// in order to get it compiling.
//
// Note: This will not actually run FreeRTOS on Linux at the moment.

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters) {
  // This is called when each Task is created

  // TODO: Implement this
  //
  // This probably will be something like:
  // 1. Create pthread + block it until it is time to run
  // 2. Update total Tasks
  //
  // Notes:
  // I'm not sure how pre-emption is going to occur, since we can't necessarily
  // guarantee that signals will be sent during valid cancellation points...
  // Ideally, we have a single thread perform all the signal processing...
  //
  // Unfortunately, Linux doesn't implement the POSIX pthread_suspend and
  // pthread_resume_np APIs, so there isn't an equivalent to the Windows
  // SuspendThread. This means that in order to "suspend" a thread, things are
  // much more complicated.
  //
  // The main question is:
  //
  // What happens when you call pthread_yield inside your signal handler? What
  // happens to the Thread context when it is woken up again?
  //
  // I would assume that because signal handlers run in the context of your
  // thread, that you could pthread_yield indefinitely until the FreeRTOS
  // scheduler decides for you to run.
  //
  // If this is the case, then you could simply direct SIG_SUSPEND at a
  // particular thread, and then wake the thread you wish to run via a
  // SIG_RESUME.
  //
  // However, I'm not sure how this approach will work with external
  // interrupts, since this inherently assumes that the SysTick handler is the
  // only one that can suspend/resume a Task (unless a Task yields its
  // time-slice). Maybe this works if we only handle interrupts in the main
  // loop?
  //
  // The alternative is that each Task is a forked as a separate process.
  // However, this isn't great either, as then you don't necessarily have the
  // same address space, and so you would need to implement FreeRTOS data
  // structures using IPC. It also doesn't necessarily solve the problem with
  // external interrupts.
  //
  // IMO, the simplest is to make use of co-operative scheduling, and assume
  // that your Tasks will yield via taskYIELD, or call vTaskDelay. This way,
  // you can have defined cancellation points (at the end of each tick), and a
  // signal handler in the main() thread can defer work. Then interrupts can be
  // delivered either before the Task runs, or after the Task runs.
  return 0;
}

BaseType_t xPortStartScheduler(void) {
  // TODO: Implement this
  //
  // This will probably be something like:
  // 1. Set up tick interrupt via a timer
  // 2. Set up tick interrupt handler
  // 3. Start the first Task
  // 4. Event loop while vPortEndScheduler flag isn't raised
  return 0;
}

void vPortEndScheduler(void) {
  // TODO: Implement this
  //
  // This will probably be something like:
  // 1. Raise vPortEndScheduler flag
  //
  // Not sure, but I don't think we can do anything else here, since vPortEndScheduler can be called from any task,
  // and so it is up to xPortStartScheduler to perform all the cleanup?
  return;
}
