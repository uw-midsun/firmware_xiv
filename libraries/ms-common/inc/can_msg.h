#pragma once
// Defines the CAN message type
// This is kept in a separate file to prevent cyclic dependencies
#include <stddef.h>
#include <stdint.h>

#define CAN_MSG_INVALID_ID (UINT16_MAX)
#define CAN_MSG_INVALID_DEVICE (UINT16_MAX)

#define CAN_MSG_MAX_DEVICES (1 << 4)
#define CAN_MSG_MAX_IDS (1 << 6)

// TODO(ELEC-202): determine which messages are considered "critical"
#define CAN_MSG_IS_CRITICAL(msg) ((msg)->msg_id < 14)

#define CAN_MSG_SET_RAW_ID(can_msg, can_id) \
  do {                                      \
    CanId id = { .raw = (can_id) };         \
    (can_msg)->source_id = id.source_id;    \
    (can_msg)->msg_id = id.msg_id;          \
    (can_msg)->type = id.type;              \
  } while (0)

typedef enum {
  CAN_MSG_TYPE_DATA = 0,
  CAN_MSG_TYPE_ACK,
  NUM_CAN_MSG_TYPES,
} CanMsgType;

typedef uint16_t CanMessageId;

typedef struct CanMessage {
  uint16_t source_id;
  CanMessageId msg_id;
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
  CanMsgType type;
  size_t dlc;
} CanMessage;

typedef union CanId {
  uint16_t raw;
  struct {
    uint16_t source_id : 4;
    uint16_t type : 1;
    uint16_t msg_id : 6;
  };
} CanId;
