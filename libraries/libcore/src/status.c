#include "status.h"

#include <stdlib.h>

static Status s_global_status = {
  .source = "",   //
  .caller = "",   //
  .message = "",  //
};
static StatusCallback s_callback;

StatusCode status_impl_update(const StatusCode code, const char *source, const char *caller,
                              const char *message) {
  s_global_status.code = code;
  s_global_status.source = source;
  s_global_status.caller = caller;
  s_global_status.message = message;
  if (s_callback != NULL) {
    s_callback(&s_global_status);
  }
  return code;
}

Status status_get(void) {
  return s_global_status;
}

void status_register_callback(StatusCallback callback) {
  s_callback = callback;
}
