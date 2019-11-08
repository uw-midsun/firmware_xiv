#include "calib.h"

static PersistStorage s_persist;
static void *s_blob;

StatusCode calib_init(void *blob, size_t blob_size, bool overwrite) {
  s_blob = blob;
  status_ok_or_return(persist_init(&s_persist, CALIB_FLASH_PAGE, blob, blob_size, overwrite));
  return persist_ctrl_periodic(&s_persist, false);
}

StatusCode calib_commit(void) {
  return persist_commit(&s_persist);
}

void *calib_blob(void) {
  return s_blob;
}
