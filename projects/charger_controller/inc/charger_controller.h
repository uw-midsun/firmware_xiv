#include "mcp2515.h"

typedef struct ChargerData {
  Mcp2515Storage *storage;
  uint32_t id;
  bool extended;
  uint64_t data;
  size_t dlc;
} ChargerData;


