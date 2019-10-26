#include "can_interval.h"

#include <stdint.h>
#include <string.h>

#include "can.h"
#include "can_uart.h"
#include "generic_can.h"
#include "objpool.h"
#include "soft_timer.h"
#include "status.h"

static ObjectPool s_can_interval_pool;
static CanInterval s_can_interval_storage[CAN_INTERVAL_POOL_SIZE];

static void prv_can_interval_timer_cb(SoftTimerId id, void *context) {
  (void)id;
  CanInterval *interval = context;
  generic_can_tx(interval->can, &interval->msg);
  soft_timer_start(interval->period, prv_can_interval_timer_cb, context, &interval->timer_id);
}

static void prv_init_can_interval(void *object, void *context) {
  (void)context;
  CanInterval *interval = object;
  interval->period = 0;
  interval->timer_id = SOFT_TIMER_INVALID_TIMER;
  memset(&interval->msg, 0, sizeof(GenericCanMsg));
  interval->can = NULL;
}

void can_interval_init(void) {
  objpool_init(&s_can_interval_pool, s_can_interval_storage, prv_init_can_interval, NULL);
}

StatusCode can_interval_factory(const GenericCan *can, const GenericCanMsg *msg, uint32_t period,
                                CanInterval **interval) {
  CanInterval *interval_impl = objpool_get_node(&s_can_interval_pool);
  if (interval_impl == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  interval_impl->can = can;
  interval_impl->msg = *msg;
  interval_impl->period = period;
  *interval = interval_impl;
  return STATUS_CODE_OK;
}

StatusCode can_interval_send_now(CanInterval *interval) {
  status_ok_or_return(can_interval_disable(interval));
  return can_interval_enable(interval);
}

StatusCode can_interval_enable(CanInterval *interval) {
  if (interval == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Check if already active.
  if (interval->timer_id == SOFT_TIMER_INVALID_TIMER) {
    // Send now.
    status_ok_or_return(generic_can_tx(interval->can, &interval->msg));
    status_ok_or_return(soft_timer_start(interval->period, prv_can_interval_timer_cb,
                                         (void *)interval, &interval->timer_id));
  }

  return STATUS_CODE_OK;
}

StatusCode can_interval_disable(CanInterval *interval) {
  if (interval == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (interval->timer_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(interval->timer_id);
    interval->timer_id = SOFT_TIMER_INVALID_TIMER;
  }
  return STATUS_CODE_OK;
}
