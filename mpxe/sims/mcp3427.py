from mpxe.protogen import stores_pb2
from mpxe.protogen import mcp3427_pb2

from mpxe.sims import sim

MCP3427_KEY = (stores_pb2.MxStoreType.MCP3427, 0)
NUM_MCP3427_CHANNELS = 2

class Mcp3427(sim.Sim):
    def handle_update(self, pm, proj):
        stores = proj.stores
        if MCP3427_KEY in stores:
            self.readings = [stores[MCP3427_KEY].readings[i] for i in range(NUM_MCP3427_CHANNELS)]

    def update_mcp3427(self, proj, val):
        mcp3427_msg = mcp3427_pb2.MxMcp3427Store()
        mcp3427_msg.fault_flag = val

        mcp3427_mask = mcp3427_pb2.MxMcp3427Store()
        mcp3427_mask.fault_flag = 1

        proj.write_store(mcp3427_msg, mcp3427_mask, stores_pb2.MxStoreType.MCP3427)

    def assert_store_value_reading(self, proj, reading):
        for i in range(NUM_MCP3427_CHANNELS):
            assert(self.readings[i] == reading)