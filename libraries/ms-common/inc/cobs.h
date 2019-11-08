#pragma once
// Consistent Overhead Byte Stuffing (COBS)
// Used for packet framing over serial
//
// See https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
#include <stddef.h>
#include <stdint.h>
#include "status.h"

#define COBS_MAX_ENCODED_LEN(data_len) \
  ({                                   \
    size_t _len = (data_len);          \
    _len + _len / 254 + 1;             \
  })

// Takes in an array of input bytes |data| of size |data_len| and an output
// array |encoded| where |encoded_len| points to storage initially set to the
// maximum size of |encoded|. After encoding, |encoded_len| is set to the size
// of the encoded output.
StatusCode cobs_encode(const uint8_t *data, size_t data_len, uint8_t *encoded, size_t *encoded_len);

// Takes in an array of input bytes |encoded| of size |encoded_len| and an
// output array |decoded| where |decoded_len| points to storage initially set to
// the maximum size of |encoded|. After decoding, |decoded_len| is set to the
// size of the decoded output.
StatusCode cobs_decode(const uint8_t *encoded, size_t encoded_len, uint8_t *decoded,
                       size_t *decoded_len);
