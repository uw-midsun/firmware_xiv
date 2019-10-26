#pragma once

#include <stddef.h>
#include <stdint.h>

#include "can_msg.h"
#include "status.h"

#define CAN_PACK_IMPL_MAX_DLC 8
#define CAN_PACK_IMPL_EMPTY 0
#define CAN_PACK_IMPL_NO_BYTES 0

// Packs eight u8s into a CAN Msg
StatusCode can_pack_impl_u8(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                            uint8_t f1, uint8_t f2, uint8_t f3, uint8_t f4, uint8_t f5, uint8_t f6,
                            uint8_t f7, uint8_t f8);

// Packs four u16s into a CAN Msg
StatusCode can_pack_impl_u16(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                             uint16_t f1, uint16_t f2, uint16_t f3, uint16_t f4);

// Packs a pair of u32 into a CAN Msg
StatusCode can_pack_impl_u32(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                             uint32_t f1, uint32_t f2);

// Packs a u64 into a CAN Msg
StatusCode can_pack_impl_u64(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                             uint64_t f1);

// Packs an empty CAN Msg
#define can_pack_impl_empty(msg_ptr, source_id, id) \
  can_pack_impl_u64((msg_ptr), (source_id), (id), CAN_PACK_IMPL_NO_BYTES, CAN_PACK_IMPL_EMPTY)
