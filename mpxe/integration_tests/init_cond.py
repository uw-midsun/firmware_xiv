import mpxe.protogen
from mpxe.protogen import stores_pb2
from mpxe.harness.project import StoreUpdate

def mcp23008_init_conditions():
    NUM_MCP_PINS = 8 
    mcp23008_msg             = mpxe.protogen.mcp23008_pb2.MxMcp23008Store()
    mcp23008_msg.state.extend([0] * 8)
    for i in range(NUM_MCP_PINS):
        mcp23008_msg.state[i]       =  i % 2
    
    mcp23008_mask            = mpxe.protogen.mcp23008_pb2.MxMcp23008Store()
    mcp23008_mask.state.extend([0] * 8)
    for i in range(NUM_MCP_PINS):
        mcp23008_mask.state[i]      = 1
    return [StoreUpdate(mcp23008_msg, mcp23008_mask, stores_pb2.MxStoreType.MCP23008, 0)]
