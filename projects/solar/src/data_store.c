#include "data_store.h"
#include "event_queue.h"
#include "solar_events.h"

static uint16_t s_data_store[NUM_DATA_POINTS];

StatusCode data_store_enter(DataPoint data_point, uint16_t value) {
  if (data_point >= NUM_DATA_POINTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_data_store[data_point] = value;
  return STATUS_CODE_OK;
}

StatusCode data_store_done(void) {
  // the data ready event has no associated data
  return event_raise_priority(DATA_READY_EVENT_PRIORITY, DATA_READY_EVENT, 0);
}

StatusCode data_store_get(DataPoint data_point, uint16_t *value) {
  if (!value || data_point >= NUM_DATA_POINTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *value = s_data_store[data_point];
  return STATUS_CODE_OK;
}
