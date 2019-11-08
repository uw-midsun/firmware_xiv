// See http://www.stuartcheshire.org/papers/COBSforSIGCOMM/ and
// https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
//
// Essentially, the goal is to break up the data into groups of either 254
// non-zero bytes or 0-253 non-zero bytes followed by a zero byte. We append a
// zero byte to the data first so this is always possible.
//
// We encode the data by replacing the trailing zero byte and prepending the
// number of non-zero bytes + 1, resulting in an implicit zero. In the case
// where there are 254 non-zero bytes, we use 0xFF to represent 254 data bytes
// not followed by a zero. Since there are now no zeros in the encoded form, we
// can use it as a packet delimiter.
#include "cobs.h"
#include <stddef.h>
#include <string.h>
#include "log.h"

StatusCode cobs_encode(const uint8_t *data, size_t data_len, uint8_t *encoded,
                       size_t *encoded_len) {
  if (data == NULL || encoded == NULL || encoded_len == NULL || data_len == 0 ||
      *encoded_len < COBS_MAX_ENCODED_LEN(data_len)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // We always begin with one overhead byte
  size_t encoded_index = 1;
  size_t last_zero_index = 0;
  uint8_t non_zero_count = 0;
  for (size_t i = 0; i < data_len; i++) {
    if (non_zero_count == 254) {
      // We had 254 non-zero bytes in a row - complete group
      // Use 0xFF to represent special case
      encoded[last_zero_index] = 0xFF;
      last_zero_index = encoded_index++;
      non_zero_count = 0;
    }

    if (data[i] != 0) {
      // Non-zero byte - copy directly into encoded buffer
      encoded[encoded_index++] = data[i];
      non_zero_count++;
    } else {
      // Zero byte - complete group
      encoded[last_zero_index] = non_zero_count + 1;
      last_zero_index = encoded_index++;
      non_zero_count = 0;
    }
  }
  // No more data - complete group
  encoded[last_zero_index] = non_zero_count + 1;

  *encoded_len = encoded_index;

  return STATUS_CODE_OK;
}

StatusCode cobs_decode(const uint8_t *encoded, size_t encoded_len, uint8_t *decoded,
                       size_t *decoded_len) {
  if (encoded == NULL || encoded_len == 0 || decoded == NULL || decoded_len == NULL ||
      *decoded_len < encoded_len) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *decoded_len = 0;
  uint8_t *decoded_ptr = decoded, *encoded_end = encoded + encoded_len;
  while (encoded < encoded_end) {
    uint8_t code = *encoded++;
    uint8_t len = code - 1;
    if (code == 0 || encoded + len > encoded_end) {
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
    memcpy(decoded_ptr, encoded, len);
    encoded += len;
    decoded_ptr += len;
    if (code != 0xFF && encoded < encoded_end) {
      *decoded_ptr++ = 0;
    }
  }

  *decoded_len = (size_t)(decoded_ptr - decoded);
  return STATUS_CODE_OK;
}
