from mu.harness.project import StoreUpdate
from mu.harness.sub_sim import SubSim
from mu.protogen import stores_pb2
from mu.protogen import mcp23008_pb2

NUM_MCP23008_PINS = 8


class Mcp23008(SubSim):
    def update_pin(self, key, pin, state):
        msg = mcp23008_pb2.MuMcp23008Store()
        msg.state[pin] = state
        mask = mcp23008_pb2.MuMcp23008Store()
        mask.state[pin] = 1
        update = StoreUpdate(msg, mask, key)
        self.parent.proj.write_store(update)

    def assert_pin_state(self, key, pin, state):
        assert self.parent.stores[key].state[pin] == state
