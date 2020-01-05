#include <stdint.h>
#include "fsm.h"

typedef struct PowerFsmStorage {
  uint16_t fault_bitset;
  Fsm power_fsm;
} PowerFsmStorage;

StatusCode power_fsm_init(PowerFsmStorage *power_fsm);

bool power_fsm_process_event(PowerFsmStorage *power_fsm, const Event *event);