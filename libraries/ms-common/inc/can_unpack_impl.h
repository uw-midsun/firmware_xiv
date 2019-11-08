#pragma once

#include <stddef.h>
#include <stdint.h>

#include "can_msg.h"
#include "status.h"

#define CAN_UNPACK_IMPL_EMPTY NULL

// Unpacks eight u8s from a CAN Msg
StatusCode can_unpack_impl_u8(const CanMessage *msg, size_t expected_dlc, uint8_t *f1, uint8_t *f2,
                              uint8_t *f3, uint8_t *f4, uint8_t *f5, uint8_t *f6, uint8_t *f7,
                              uint8_t *f8);

// Unpacks four u16s from a CAN Msg
StatusCode can_unpack_impl_u16(const CanMessage *msg, size_t expected_dlc, uint16_t *f1,
                               uint16_t *f2, uint16_t *f3, uint16_t *f4);

// Unpacks a pair of u32 from a CAN Msg
StatusCode can_unpack_impl_u32(const CanMessage *msg, size_t expected_dlc, uint32_t *f1,
                               uint32_t *f2);

// Unpacks a u64 from a CAN Msg
StatusCode can_unpack_impl_u64(const CanMessage *msg, size_t expected_dlc, uint64_t *f1);

// This doesn't do anything since there is no data to unpack. The purpose of
// this is so that codegen doesn't require a special edge case and so every
// message has an unpack macro.
#define can_unpack_impl_empty(msg_ptr)
