from mpxe.protogen import stores_pb2
from mpxe.protogen import mcp23008_pb2
from mpxe.harness.project import StoreUpdate

from mpxe.sims import sim

MCP23008_KEY = (stores_pb2.MxStoreType.MCP23008, 0)
NUM_MCP_PINS = 8


class Mcp23008(sim.Sim):

    def __init__(self):
        self.states = None

    # pylint: disable=unused-argument
    def handle_update(self, pm, proj):
        stores = proj.stores
        if MCP23008_KEY in stores:
            mcp = stores[MCP23008_KEY]
            self.states = [bool(mcp.state[i]) for i in range(len(mcp.state)) if i < NUM_MCP_PINS]

    # Update the store with a new pin state
    def update_pin_state(self, proj, pin, state):
        mcp23008_msg = mcp23008_pb2.MxMcp23008Store()
        mcp23008_msg.state[pin] = state

        mcp23008_mask = mcp23008_pb2.MxMcp23008Store()
        mcp23008_mask.state[pin] = 1

        mcp23008_update = StoreUpdate(
            mcp23008_msg,
            mcp23008_mask,
            stores_pb2.MxStoreType.MCP23008,
            0)

        proj.write_store(mcp23008_update)

    # Compares pin state against store
    # pylint: disable=unused-argument
    def assert_store_value_reading(self, proj, pin, state):
        assert self.states[pin] == state
