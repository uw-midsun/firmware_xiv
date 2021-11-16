#include "flash_application.pb.h"
#include "log.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "string.h"

#define MAX_STRING_SIZE 64
#define PROTOBUF_MAXSIZE 128

void setup_test(void) {}

void teardown_test(void) {}

static size_t prv_strnlen(const char *str, size_t maxlen) {
  size_t i;
  for (i = 0; i < maxlen; ++i) {
    if (str[i] == '\0') {
      break;
    }
  }
  return i;
}

static bool prv_encode_string(pb_ostream_t *stream, const pb_field_iter_t *field,
                              void *const *arg) {
  const char *str = *arg;
  // add to stream
  if (!pb_encode_tag_for_field(stream, field)) {  // write tag and wire type
    return false;
  }
  return pb_encode_string(stream, (uint8_t *)str,
                          prv_strnlen(str, MAX_STRING_SIZE));  // write sting
}

static bool prv_decode_string(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  LOG_DEBUG("%i\n", field->index);
  LOG_DEBUG("%s\n", (char *)stream->state);
  LOG_DEBUG("%s\n", (char *)*arg);

  strncpy((char *)*arg, (char *)stream->state, 64);

  return true;
}

void test_protobuf(void) {
  // uint8_t buffer[PROTOBUF_MAXSIZE] = { 0 };

  // char name[] = "new name";

  // UpdateName un;
  // un.new_name.funcs.encode = prv_encode_string;
  // un.new_name.arg = name;

  // pb_ostream_t pb_ostream = pb_ostream_from_buffer(buffer, (size_t)PROTOBUF_MAXSIZE);
  // pb_encode(&pb_ostream, UpdateName_fields, &un);

  // char str[64] = "old";
  // UpdateName nun;
  // nun.new_name.funcs.decode = prv_decode_string;
  // nun.new_name.arg = str;

  // pb_istream_t pb_istream = pb_istream_from_buffer(buffer, (size_t)PROTOBUF_MAXSIZE);
  // pb_decode(&pb_istream, UpdateName_fields, &nun);

  // LOG_DEBUG("%s\n", str);
}
