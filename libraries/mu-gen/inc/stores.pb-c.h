/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: stores.proto */

#ifndef PROTOBUF_C_stores_2eproto__INCLUDED
#define PROTOBUF_C_stores_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _MuLog MuLog;
typedef struct _MuCmd MuCmd;
typedef struct _MuStoreInfo MuStoreInfo;
typedef struct _MuStoreUpdate MuStoreUpdate;


/* --- enums --- */

typedef enum _MuStoreType {
  MU_STORE_TYPE__LOG = 0,
  MU_STORE_TYPE__CMD = 1,
  MU_STORE_TYPE__GPIO = 2,
  MU_STORE_TYPE__ADS1015 = 3,
  MU_STORE_TYPE__MCP2515 = 4,
  MU_STORE_TYPE__ADS1259 = 5,
  MU_STORE_TYPE__ADT7476A = 6,
  MU_STORE_TYPE__ADC = 7,
  MU_STORE_TYPE__PCA9539R = 8,
  MU_STORE_TYPE__MCP23008 = 9,
  MU_STORE_TYPE__END = 10
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(MU_STORE_TYPE)
} MuStoreType;
typedef enum _MuCmdType {
  MU_CMD_TYPE__NOP = 0,
  MU_CMD_TYPE__FINISH_INIT_CONDS = 1,
  MU_CMD_TYPE__NUM_CMDS = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(MU_CMD_TYPE)
} MuCmdType;

/* --- messages --- */

struct  _MuLog
{
  ProtobufCMessage base;
  ProtobufCBinaryData log;
};
#define MU_LOG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mu_log__descriptor) \
    , {0,NULL} }


struct  _MuCmd
{
  ProtobufCMessage base;
  MuCmdType cmd;
};
#define MU_CMD__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mu_cmd__descriptor) \
    , MU_CMD_TYPE__NOP }


struct  _MuStoreInfo
{
  ProtobufCMessage base;
  uint64_t key;
  MuStoreType type;
  ProtobufCBinaryData msg;
};
#define MU_STORE_INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mu_store_info__descriptor) \
    , 0, MU_STORE_TYPE__LOG, {0,NULL} }


struct  _MuStoreUpdate
{
  ProtobufCMessage base;
  uint64_t key;
  MuStoreType type;
  ProtobufCBinaryData msg;
  ProtobufCBinaryData mask;
};
#define MU_STORE_UPDATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mu_store_update__descriptor) \
    , 0, MU_STORE_TYPE__LOG, {0,NULL}, {0,NULL} }


/* MuLog methods */
void   mu_log__init
                     (MuLog         *message);
size_t mu_log__get_packed_size
                     (const MuLog   *message);
size_t mu_log__pack
                     (const MuLog   *message,
                      uint8_t             *out);
size_t mu_log__pack_to_buffer
                     (const MuLog   *message,
                      ProtobufCBuffer     *buffer);
MuLog *
       mu_log__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mu_log__free_unpacked
                     (MuLog *message,
                      ProtobufCAllocator *allocator);
/* MuCmd methods */
void   mu_cmd__init
                     (MuCmd         *message);
size_t mu_cmd__get_packed_size
                     (const MuCmd   *message);
size_t mu_cmd__pack
                     (const MuCmd   *message,
                      uint8_t             *out);
size_t mu_cmd__pack_to_buffer
                     (const MuCmd   *message,
                      ProtobufCBuffer     *buffer);
MuCmd *
       mu_cmd__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mu_cmd__free_unpacked
                     (MuCmd *message,
                      ProtobufCAllocator *allocator);
/* MuStoreInfo methods */
void   mu_store_info__init
                     (MuStoreInfo         *message);
size_t mu_store_info__get_packed_size
                     (const MuStoreInfo   *message);
size_t mu_store_info__pack
                     (const MuStoreInfo   *message,
                      uint8_t             *out);
size_t mu_store_info__pack_to_buffer
                     (const MuStoreInfo   *message,
                      ProtobufCBuffer     *buffer);
MuStoreInfo *
       mu_store_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mu_store_info__free_unpacked
                     (MuStoreInfo *message,
                      ProtobufCAllocator *allocator);
/* MuStoreUpdate methods */
void   mu_store_update__init
                     (MuStoreUpdate         *message);
size_t mu_store_update__get_packed_size
                     (const MuStoreUpdate   *message);
size_t mu_store_update__pack
                     (const MuStoreUpdate   *message,
                      uint8_t             *out);
size_t mu_store_update__pack_to_buffer
                     (const MuStoreUpdate   *message,
                      ProtobufCBuffer     *buffer);
MuStoreUpdate *
       mu_store_update__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mu_store_update__free_unpacked
                     (MuStoreUpdate *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*MuLog_Closure)
                 (const MuLog *message,
                  void *closure_data);
typedef void (*MuCmd_Closure)
                 (const MuCmd *message,
                  void *closure_data);
typedef void (*MuStoreInfo_Closure)
                 (const MuStoreInfo *message,
                  void *closure_data);
typedef void (*MuStoreUpdate_Closure)
                 (const MuStoreUpdate *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    mu_store_type__descriptor;
extern const ProtobufCEnumDescriptor    mu_cmd_type__descriptor;
extern const ProtobufCMessageDescriptor mu_log__descriptor;
extern const ProtobufCMessageDescriptor mu_cmd__descriptor;
extern const ProtobufCMessageDescriptor mu_store_info__descriptor;
extern const ProtobufCMessageDescriptor mu_store_update__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_stores_2eproto__INCLUDED */
