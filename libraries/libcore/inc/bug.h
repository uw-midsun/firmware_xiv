#pragma once

#include "status.h"

#define BUG(call)                                                                    \
  ({                                                                                 \
    __typeof__(call) status_code = (call);                                           \
    if (status_code) {                                                               \
      Status global_status = status_get();                                           \
      LOG_CRITICAL(                                                                  \
          "Bug triggered, aborting!\n"                                               \
          "This indicates initialization failure or another unrecoverable fault.\n"  \
          "Call returned status code %d:\n"                                          \
          "  %s\n"                                                                   \
          "Location: %s:%d, in %s\n"                                                 \
          "Info from last time status_code was called (may or may not be useful):\n" \
          "  Status code: %d\n"                                                      \
          "  Location: %s, in %s\n"                                                  \
          "  Message: %s\n",                                                         \
          status_code, #call, __FILE__, __LINE__, __FUNCTION__, global_status.code,  \
          global_status.source, global_status.caller, global_status.message);        \
      __builtin_trap();                                                              \
    }                                                                                \
  })
