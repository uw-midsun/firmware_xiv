/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: stores.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "stores.pb-c.h"
void   mu_log__init
                     (MuLog         *message)
{
  static const MuLog init_value = MU_LOG__INIT;
  *message = init_value;
}
size_t mu_log__get_packed_size
                     (const MuLog *message)
{
  assert(message->base.descriptor == &mu_log__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t mu_log__pack
                     (const MuLog *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &mu_log__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t mu_log__pack_to_buffer
                     (const MuLog *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &mu_log__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
MuLog *
       mu_log__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (MuLog *)
     protobuf_c_message_unpack (&mu_log__descriptor,
                                allocator, len, data);
}
void   mu_log__free_unpacked
                     (MuLog *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &mu_log__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   mu_cmd__init
                     (MuCmd         *message)
{
  static const MuCmd init_value = MU_CMD__INIT;
  *message = init_value;
}
size_t mu_cmd__get_packed_size
                     (const MuCmd *message)
{
  assert(message->base.descriptor == &mu_cmd__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t mu_cmd__pack
                     (const MuCmd *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &mu_cmd__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t mu_cmd__pack_to_buffer
                     (const MuCmd *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &mu_cmd__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
MuCmd *
       mu_cmd__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (MuCmd *)
     protobuf_c_message_unpack (&mu_cmd__descriptor,
                                allocator, len, data);
}
void   mu_cmd__free_unpacked
                     (MuCmd *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &mu_cmd__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   mu_store_info__init
                     (MuStoreInfo         *message)
{
  static const MuStoreInfo init_value = MU_STORE_INFO__INIT;
  *message = init_value;
}
size_t mu_store_info__get_packed_size
                     (const MuStoreInfo *message)
{
  assert(message->base.descriptor == &mu_store_info__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t mu_store_info__pack
                     (const MuStoreInfo *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &mu_store_info__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t mu_store_info__pack_to_buffer
                     (const MuStoreInfo *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &mu_store_info__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
MuStoreInfo *
       mu_store_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (MuStoreInfo *)
     protobuf_c_message_unpack (&mu_store_info__descriptor,
                                allocator, len, data);
}
void   mu_store_info__free_unpacked
                     (MuStoreInfo *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &mu_store_info__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   mu_store_update__init
                     (MuStoreUpdate         *message)
{
  static const MuStoreUpdate init_value = MU_STORE_UPDATE__INIT;
  *message = init_value;
}
size_t mu_store_update__get_packed_size
                     (const MuStoreUpdate *message)
{
  assert(message->base.descriptor == &mu_store_update__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t mu_store_update__pack
                     (const MuStoreUpdate *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &mu_store_update__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t mu_store_update__pack_to_buffer
                     (const MuStoreUpdate *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &mu_store_update__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
MuStoreUpdate *
       mu_store_update__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (MuStoreUpdate *)
     protobuf_c_message_unpack (&mu_store_update__descriptor,
                                allocator, len, data);
}
void   mu_store_update__free_unpacked
                     (MuStoreUpdate *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &mu_store_update__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor mu_log__field_descriptors[1] =
{
  {
    "log",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(MuLog, log),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned mu_log__field_indices_by_name[] = {
  0,   /* field[0] = log */
};
static const ProtobufCIntRange mu_log__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor mu_log__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "MuLog",
  "MuLog",
  "MuLog",
  "",
  sizeof(MuLog),
  1,
  mu_log__field_descriptors,
  mu_log__field_indices_by_name,
  1,  mu_log__number_ranges,
  (ProtobufCMessageInit) mu_log__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor mu_cmd__field_descriptors[1] =
{
  {
    "cmd",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(MuCmd, cmd),
    &mu_cmd_type__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned mu_cmd__field_indices_by_name[] = {
  0,   /* field[0] = cmd */
};
static const ProtobufCIntRange mu_cmd__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor mu_cmd__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "MuCmd",
  "MuCmd",
  "MuCmd",
  "",
  sizeof(MuCmd),
  1,
  mu_cmd__field_descriptors,
  mu_cmd__field_indices_by_name,
  1,  mu_cmd__number_ranges,
  (ProtobufCMessageInit) mu_cmd__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor mu_store_info__field_descriptors[3] =
{
  {
    "key",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_UINT64,
    0,   /* quantifier_offset */
    offsetof(MuStoreInfo, key),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "type",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(MuStoreInfo, type),
    &mu_store_type__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "msg",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(MuStoreInfo, msg),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned mu_store_info__field_indices_by_name[] = {
  0,   /* field[0] = key */
  2,   /* field[2] = msg */
  1,   /* field[1] = type */
};
static const ProtobufCIntRange mu_store_info__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor mu_store_info__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "MuStoreInfo",
  "MuStoreInfo",
  "MuStoreInfo",
  "",
  sizeof(MuStoreInfo),
  3,
  mu_store_info__field_descriptors,
  mu_store_info__field_indices_by_name,
  1,  mu_store_info__number_ranges,
  (ProtobufCMessageInit) mu_store_info__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor mu_store_update__field_descriptors[4] =
{
  {
    "key",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_UINT64,
    0,   /* quantifier_offset */
    offsetof(MuStoreUpdate, key),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "type",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(MuStoreUpdate, type),
    &mu_store_type__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "msg",
    3,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(MuStoreUpdate, msg),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "mask",
    4,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(MuStoreUpdate, mask),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned mu_store_update__field_indices_by_name[] = {
  0,   /* field[0] = key */
  3,   /* field[3] = mask */
  2,   /* field[2] = msg */
  1,   /* field[1] = type */
};
static const ProtobufCIntRange mu_store_update__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 4 }
};
const ProtobufCMessageDescriptor mu_store_update__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "MuStoreUpdate",
  "MuStoreUpdate",
  "MuStoreUpdate",
  "",
  sizeof(MuStoreUpdate),
  4,
  mu_store_update__field_descriptors,
  mu_store_update__field_indices_by_name,
  1,  mu_store_update__number_ranges,
  (ProtobufCMessageInit) mu_store_update__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCEnumValue mu_store_type__enum_values_by_number[11] =
{
  { "LOG", "MU_STORE_TYPE__LOG", 0 },
  { "CMD", "MU_STORE_TYPE__CMD", 1 },
  { "GPIO", "MU_STORE_TYPE__GPIO", 2 },
  { "ADS1015", "MU_STORE_TYPE__ADS1015", 3 },
  { "MCP2515", "MU_STORE_TYPE__MCP2515", 4 },
  { "ADS1259", "MU_STORE_TYPE__ADS1259", 5 },
  { "ADT7476A", "MU_STORE_TYPE__ADT7476A", 6 },
  { "ADC", "MU_STORE_TYPE__ADC", 7 },
  { "PCA9539R", "MU_STORE_TYPE__PCA9539R", 8 },
  { "MCP23008", "MU_STORE_TYPE__MCP23008", 9 },
  { "END", "MU_STORE_TYPE__END", 10 },
};
static const ProtobufCIntRange mu_store_type__value_ranges[] = {
{0, 0},{0, 11}
};
static const ProtobufCEnumValueIndex mu_store_type__enum_values_by_name[11] =
{
  { "ADC", 7 },
  { "ADS1015", 3 },
  { "ADS1259", 5 },
  { "ADT7476A", 6 },
  { "CMD", 1 },
  { "END", 10 },
  { "GPIO", 2 },
  { "LOG", 0 },
  { "MCP23008", 9 },
  { "MCP2515", 4 },
  { "PCA9539R", 8 },
};
const ProtobufCEnumDescriptor mu_store_type__descriptor =
{
  PROTOBUF_C__ENUM_DESCRIPTOR_MAGIC,
  "MuStoreType",
  "MuStoreType",
  "MuStoreType",
  "",
  11,
  mu_store_type__enum_values_by_number,
  11,
  mu_store_type__enum_values_by_name,
  1,
  mu_store_type__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
static const ProtobufCEnumValue mu_cmd_type__enum_values_by_number[3] =
{
  { "NOP", "MU_CMD_TYPE__NOP", 0 },
  { "FINISH_INIT_CONDS", "MU_CMD_TYPE__FINISH_INIT_CONDS", 1 },
  { "NUM_CMDS", "MU_CMD_TYPE__NUM_CMDS", 3 },
};
static const ProtobufCIntRange mu_cmd_type__value_ranges[] = {
{0, 0},{3, 2},{0, 3}
};
static const ProtobufCEnumValueIndex mu_cmd_type__enum_values_by_name[3] =
{
  { "FINISH_INIT_CONDS", 1 },
  { "NOP", 0 },
  { "NUM_CMDS", 2 },
};
const ProtobufCEnumDescriptor mu_cmd_type__descriptor =
{
  PROTOBUF_C__ENUM_DESCRIPTOR_MAGIC,
  "MuCmdType",
  "MuCmdType",
  "MuCmdType",
  "",
  3,
  mu_cmd_type__enum_values_by_number,
  3,
  mu_cmd_type__enum_values_by_name,
  2,
  mu_cmd_type__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
