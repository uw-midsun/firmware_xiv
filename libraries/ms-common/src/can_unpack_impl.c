#include "can_unpack_impl.h"

#include <stddef.h>
#include <stdint.h>

#include "can_msg.h"

#define CAN_UNPACK_IF_NOT_NULL(msg_data, f_ptr) \
  if ((f_ptr) != NULL) {                        \
    *(f_ptr) = (msg_data);                      \
  }

StatusCode can_unpack_impl_u8(const CanMessage *msg, size_t expected_dlc, uint8_t *f1, uint8_t *f2,
                              uint8_t *f3, uint8_t *f4, uint8_t *f5, uint8_t *f6, uint8_t *f7,
                              uint8_t *f8) {
  if (expected_dlc != msg->dlc) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "DLC mismatch");
  }
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[0], f1);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[1], f2);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[2], f3);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[3], f4);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[4], f5);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[5], f6);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[6], f7);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u8[7], f8);
  return STATUS_CODE_OK;
}

StatusCode can_unpack_impl_u16(const CanMessage *msg, size_t expected_dlc, uint16_t *f1,
                               uint16_t *f2, uint16_t *f3, uint16_t *f4) {
  if (expected_dlc != msg->dlc) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "DLC mismatch");
  }
  CAN_UNPACK_IF_NOT_NULL(msg->data_u16[0], f1);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u16[1], f2);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u16[2], f3);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u16[3], f4);
  return STATUS_CODE_OK;
}

StatusCode can_unpack_impl_u32(const CanMessage *msg, size_t expected_dlc, uint32_t *f1,
                               uint32_t *f2) {
  if (expected_dlc != msg->dlc) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "DLC mismatch");
  }
  CAN_UNPACK_IF_NOT_NULL(msg->data_u32[0], f1);
  CAN_UNPACK_IF_NOT_NULL(msg->data_u32[1], f2);
  return STATUS_CODE_OK;
}

StatusCode can_unpack_impl_u64(const CanMessage *msg, size_t expected_dlc, uint64_t *f1) {
  if (expected_dlc != msg->dlc) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "DLC mismatch");
  }
  CAN_UNPACK_IF_NOT_NULL(msg->data, f1);
  return STATUS_CODE_OK;
}
