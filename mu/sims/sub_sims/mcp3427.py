from mu.harness.project import StoreUpdate
from mu.harness.sub_sim import SubSim
from mu.protogen import stores_pb2
from mu.protogen import mcp3427_pb2

MCP3427_KEY = (stores_pb2.MuStoreType.MCP3427, 0)
NUM_MCP3427_CHANNELS = 2

class Mcp3427(SubSim):
    # val1 writes channel1 of mcp3427, val2 channel2
    def update_mcp3427(self, val1, val2, fault_flag):
        mcp3427_msg = mcp3427_pb2.MuMcp3427Store()
        mcp3427_msg.readings.extend([0] * NUM_MCP3427_CHANNELS)
        mcp3427_msg.fault_flag = fault_flag
        mcp3427_msg.readings[0] = val1
        mcp3427_msg.readings[1] = val2

        mcp3427_mask = mcp3427_pb2.MuMcp3427Store()
        mcp3427_mask.readings.extend([0] * NUM_MCP3427_CHANNELS)
        mcp3427_mask.fault_flag = 1
        mcp3427_mask.readings[0] = 1
        mcp3427_mask.readings[1] = 1

        mcp3427_update = StoreUpdate(mcp3427_msg, mcp3427_mask, MCP3427_KEY)
        self.parent.proj.write_store(mcp3427_update)
