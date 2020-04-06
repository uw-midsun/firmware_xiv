#include "mcp2515.h"
#include "generic_can_mcp2515.h"

StatusCode charger_controller_init(GenericCanMcp2515 *can_mcp2515);

StatusCode charger_controller_activate();

StatusCode charger_controller_deactivate();

uint64_t get_max_allowable_vc();
