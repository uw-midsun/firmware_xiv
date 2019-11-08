#pragma once
// Common helper macros

#define SIZEOF_ARRAY(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define SIZEOF_FIELD(type, field) (sizeof(((type *)0)->field))
// Casts void * to uint8_t *
#define VOID_PTR_UINT8(x) (_Generic((x), void * : (uint8_t *)(x), default : (x)))
#define SWAP_UINT16(x) (uint16_t)(((uint16_t)(x) >> 8) | ((uint16_t)(x) << 8))

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })

#define MAX(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })
