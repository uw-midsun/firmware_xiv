#pragma once
// Status Library for more verbose error handling
//
// Usage:
// Use this library in any function where an error could occur. A verbose error
// will improve debug-ability and allow for logging and attempts at recovery.
//
// If an error has occurred in a function that returns a StatusCode then:
// return status_code(SomeStatusCode);
// OR
// return status_msg(SomeStatusCode, "error message");
//
// If no error has occurred it is recommended to:
// return STATUS_CODE_OK;
//
// To handle the returned error:
// StatusCode status = some_func_that_returns_status();
// if (!status_ok(status)) {
//   // Do something to handle the error.
//   // Optionally if recoverable:
//   status_clear();
// }
// // Continue execution
//
// If you need to see the error beyond just the StatusCode in the event multiple
// callers could have caused it or for debugging: Status status = status_get();
//
// If it is necessary to forward the error down the stack then there are three
// options: status_ok_or_return(some_func_that_returns_status()); OR StatusCode
// status = some_func_that_returns_status(); if (!status_ok(status)) {
//   // Maybe do a partial recovery or cleanup.
//   return status;
// }
// OR
// return some_func_that_returns_status();

#include "misc.h"

// StatusCodes for various errors, keep these generic. Never assume their order
// is fixed so refer to them by name only.
typedef enum {
  STATUS_CODE_OK = 0,
  STATUS_CODE_UNKNOWN,
  STATUS_CODE_INVALID_ARGS,
  STATUS_CODE_RESOURCE_EXHAUSTED,
  STATUS_CODE_UNREACHABLE,
  STATUS_CODE_TIMEOUT,
  STATUS_CODE_EMPTY,
  STATUS_CODE_OUT_OF_RANGE,
  STATUS_CODE_UNIMPLEMENTED,
  STATUS_CODE_UNINITIALIZED,
  STATUS_CODE_INTERNAL_ERROR,
  NUM_STATUS_CODES,
} StatusCode;

typedef struct Status {
  StatusCode code;
  const char *source;
  const char *caller;
  const char *message;
} Status;

typedef void (*StatusCallback)(const Status *status);

// Updates a status struct containing an error code and optionally a message.
// This should only be called via the macros.
StatusCode status_impl_update(StatusCode code, const char *source, const char *caller,
                              const char *message);

// Get a copy of the global status so it can be used safely.
Status status_get(void);

// Set a callback that is run whenever the status is changed.
void status_register_callback(StatusCallback callback);

// Macros for convenience.
#define status_code(code) \
  status_impl_update((code), (__FILE__ ":" STRINGIFY(__LINE__)), (__FUNCTION__), "")
#define status_msg(code, message) \
  status_impl_update((code), (__FILE__ ":" STRINGIFY(__LINE__)), (__FUNCTION__), (message))
#define status_ok(code) (STATUS_CODE_OK == (code))
#define status_clear() status_msg(STATUS_CODE_OK, "Clear")

// Use to forward failures or continue on success.
#define status_ok_or_return(code)          \
  ({                                       \
    __typeof__(code) status_expr = (code); \
    if (status_expr) return status_expr;   \
  })
