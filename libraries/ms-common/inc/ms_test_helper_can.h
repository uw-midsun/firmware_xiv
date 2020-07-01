#pragma once

#include "can.h"
#include "can_msg_defs.h"

StatusCode initialize_can_and_dependencies(CanStorage *storage, SystemCanDevice device,
                                           EventId tx_event, EventId rx_event, EventId fault_event);
