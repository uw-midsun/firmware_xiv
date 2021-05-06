#pragma once

// The BUG macro wraps a function call and aborts the program with a debug message if the call
// returns a non-OK status code.
//
// Usage:
// BUG(initialize_my_critical_module());
//
// BUG is meant to wrap operations where the only sensible thing to do on failure is to exit with
// debugging information, such as initializing modules in main. If a BUG is triggered, it should
// indicate a show-stopping bug that must be fixed before the project can run.
//
// Please DO NOT use this macro if it is possible for firmware to continue after a failure,
// or if there is any possibility that it might be triggered in production (i.e. not as the result
// of a serious and unrecoverable firmware bug). Instead, use status_ok_or_return from status.h,
// or check the status yourself and handle any errors, even if the error handling is just a log.

#define BUG(call)                                                                    \
  ({                                                                                 \
    __typeof__(call) status_code = (call);                                           \
    if (status_code) {                                                               \
      LOG_CRITICAL(                                                                  \
          "\n"                                                                       \
          "****************** BUG TRIGGERED, ABORTING PROGRAM ******************\n"  \
          "This indicates initialization failure or another unrecoverable fault.\n"  \
          "Call returned status code %d:\n"                                          \
          "  %s\n"                                                                   \
          "in function %s\n"                                                         \
          "at %s:%d\n"                                                               \
          "*********************************************************************\n", \
          status_code, #call, __FUNCTION__, __FILE__, __LINE__);                     \
      /* Exit immediately, via a hard fault on STM32 or another method on x86. */    \
      __builtin_trap();                                                              \
    }                                                                                \
  })
