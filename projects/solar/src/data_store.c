#include "data_store.h"
#include "event_queue.h"
#include "solar_events.h"

static uint32_t s_data_store[NUM_DATA_POINTS];
static bool s_is_set[NUM_DATA_POINTS];

StatusCode data_store_init(void) {
  // reset all the data points to not set
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    s_is_set[data_point] = false;
  }
  return STATUS_CODE_OK;
}

StatusCode data_store_set(DataPoint data_point, uint32_t value) {
  if (data_point >= NUM_DATA_POINTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_data_store[data_point] = value;
  s_is_set[data_point] = true;
  return STATUS_CODE_OK;
}

StatusCode data_store_set_signed(DataPoint data_point, int32_t value) {
  if (data_point >= NUM_DATA_POINTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_data_store[data_point] = (uint32_t)value;
  s_is_set[data_point] = true;
  return STATUS_CODE_OK;
}

StatusCode data_store_done(void) {
  // the data ready event has no associated data
  return event_raise_priority(DATA_READY_EVENT_PRIORITY, DATA_READY_EVENT, 0);
}

StatusCode data_store_get(DataPoint data_point, uint32_t *value) {
  if (value == NULL || data_point >= NUM_DATA_POINTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *value = s_data_store[data_point];
  return STATUS_CODE_OK;
}

StatusCode data_store_get_signed(DataPoint data_point, int32_t *value) {
  if (value == NULL || data_point >= NUM_DATA_POINTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *value = (int32_t)s_data_store[data_point];
  return STATUS_CODE_OK;
}

StatusCode data_store_get_is_set(DataPoint data_point, bool *is_set) {
  if (is_set == NULL || data_point >= NUM_DATA_POINTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *is_set = s_is_set[data_point];
  return STATUS_CODE_OK;
}
