from mpxe.protogen import stores_pb2
from mpxe.protogen import mcp23008_pb2
from mpxe.integration_tests.init_cond import mcp23008_init_conditions

import time

from mpxe.sims import sim

MCP23008_KEY = (stores_pb2.MxStoreType.MCP23008, 0)
NUM_MCP_PINS = 8
INIT_COND = True

class Mcp23008(sim.Sim):

    def handle_update(self, pm, proj):
        stores = proj.stores
        if MCP23008_KEY in stores:
            mcp          = stores[MCP23008_KEY]
            self.states  = [bool(mcp.state[i]) for i in range(len(mcp.state)) if i < NUM_MCP_PINS]
        elif INIT_COND:
            mcp = mcp23008_init_conditions()[0]
            self.states  = [bool(mcp.state[i]) for i in range(len(mcp.state)) if i < NUM_MCP_PINS]

    # Update the store with a new pin state
    def update_pin_state(self, proj, pin, state):
        mcp23008_msg             = mcp23008_pb2.MxMcp23008Store()
        mcp23008_msg.state[pin]  = state

        mcp23008_mask            = mcp23008_pb2.MxMcp23008Store()
        mcp23008_mask.state[pin] = 1

        proj.write_store(mcp23008_msg, mcp23008_mask, stores_pb2.MxStoreType.MCP23008)

    # Compares pin state against store
    def assert_store_value_reading(self, proj, pin, state):
        for i in range(NUM_MCP_PINS):
            assert(self.states[pin] == state)

