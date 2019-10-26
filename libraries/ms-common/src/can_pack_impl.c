#include "can_pack_impl.h"

#include <stdint.h>

#include "can_msg.h"
#include "status.h"

StatusCode can_pack_impl_u8(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                            uint8_t f1, uint8_t f2, uint8_t f3, uint8_t f4, uint8_t f5, uint8_t f6,
                            uint8_t f7, uint8_t f8) {
  if (num_bytes > CAN_PACK_IMPL_MAX_DLC) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "DLC too large");
  }
  *msg = (CanMessage){
    .type = CAN_MSG_TYPE_DATA,                      //
    .source_id = source_id,                         //
    .msg_id = id,                                   //
    .data_u8 = { f1, f2, f3, f4, f5, f6, f7, f8 },  //
    .dlc = num_bytes,                               //
  };
  return STATUS_CODE_OK;
}

StatusCode can_pack_impl_u16(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                             uint16_t f1, uint16_t f2, uint16_t f3, uint16_t f4) {
  if (num_bytes > CAN_PACK_IMPL_MAX_DLC) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "DLC too large");
  }
  *msg = (CanMessage){
    .type = CAN_MSG_TYPE_DATA,       //
    .source_id = source_id,          //
    .msg_id = id,                    //
    .data_u16 = { f1, f2, f3, f4 },  //
    .dlc = num_bytes,                //
  };
  return STATUS_CODE_OK;
}

StatusCode can_pack_impl_u32(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                             uint32_t f1, uint32_t f2) {
  if (num_bytes > CAN_PACK_IMPL_MAX_DLC) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "DLC too large");
  }
  *msg = (CanMessage){
    .type = CAN_MSG_TYPE_DATA,  //
    .source_id = source_id,     //
    .msg_id = id,               //
    .data_u32 = { f1, f2 },     //
    .dlc = num_bytes,           //
  };
  return STATUS_CODE_OK;
}

StatusCode can_pack_impl_u64(CanMessage *msg, uint16_t source_id, CanMessageId id, size_t num_bytes,
                             uint64_t f1) {
  if (num_bytes > CAN_PACK_IMPL_MAX_DLC) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "DLC too large");
  }
  *msg = (CanMessage){
    .type = CAN_MSG_TYPE_DATA,  //
    .source_id = source_id,     //
    .msg_id = id,               //
    .data = f1,                 //
    .dlc = num_bytes,           //
  };
  return STATUS_CODE_OK;
}
