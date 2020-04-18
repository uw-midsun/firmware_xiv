#include "speed_monitor.h"
#include "can_unpack.h"
#include "log.h"

static SpeedState s_speed_state = NUM_SPEED_STATES;

typedef struct SpeedMonitorStorage {
  WatchdogTimeout timeout;
  WatchdogStorage watchdog_storage;
} SpeedMonitorStorage;

static SpeedMonitorStorage s_storage = { 0 };

static void prv_did_not_receive_speed_message(void *context) {
  s_speed_state = NUM_SPEED_STATES;
}

static StatusCode prv_receive_velocity(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  uint16_t u_velocity_left, u_velocity_right = 0;
  CAN_UNPACK_MOTOR_VELOCITY(msg, &u_velocity_left, &u_velocity_right);
  float velocity_left = (float)u_velocity_left, velocity_right = (float)u_velocity_right;

  if (velocity_right > STATIONARY_VELOCITY_THRESHOLD ||
      velocity_left > STATIONARY_VELOCITY_THRESHOLD) {
    s_speed_state = SPEED_STATE_MOVING;
  } else {
    s_speed_state = SPEED_STATE_STATIONARY;
  }
  watchdog_kick(&s_storage.watchdog_storage);
  return STATUS_CODE_OK;
}

StatusCode speed_monitor_init(WatchdogTimeout timeout) {
  s_storage = (SpeedMonitorStorage){ .timeout = timeout };
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY, prv_receive_velocity, NULL));
  watchdog_start(&s_storage.watchdog_storage, timeout, prv_did_not_receive_speed_message, NULL);
  return STATUS_CODE_OK;
}

SpeedState *get_global_speed_state(void) {
  return &s_speed_state;
}
