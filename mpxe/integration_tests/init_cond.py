from mpxe.protogen import mcp23008_pb2, pca9539r_pb2

from mpxe.protogen import stores_pb2
from mpxe.harness.project import StoreUpdate


def mcp23008_init_conditions():
    NUM_MCP_PINS = 8
    mcp23008_msg = mcp23008_pb2.MxMcp23008Store()
    mcp23008_msg.state.extend([0] * 8)
    for i in range(NUM_MCP_PINS):
        mcp23008_msg.state[i] = i % 2

    mcp23008_mask = mcp23008_pb2.MxMcp23008Store()
    mcp23008_mask.state.extend([0] * 8)
    for i in range(NUM_MCP_PINS):
        mcp23008_mask.state[i] = 1
    return [StoreUpdate(mcp23008_msg, mcp23008_mask, stores_pb2.MxStoreType.MCP23008, 0)]


def pca9539r_init_conditions():
    NUM_PCA_PINS = 16
    pca9539r_msg = pca9539r_pb2.MxPca9539rStore()
    pca9539r_msg.state.extend([0] * NUM_PCA_PINS)
    for i in range(NUM_PCA_PINS):
        pca9539r_msg.state[i] = i % 2

    pca9539r_mask = pca9539r_pb2.MxPca9539rStore()
    pca9539r_mask.state.extend([0] * NUM_PCA_PINS)
    for i in range(NUM_PCA_PINS):
        pca9539r_mask.state[i] = 1

    pca9539r_update1 = StoreUpdate(
        pca9539r_msg,
        pca9539r_mask,
        stores_pb2.MxStoreType.PCA9539R,
        0x74)

    pca9539r_msg2 = pca9539r_pb2.MxPca9539rStore()
    pca9539r_msg2.state.extend([0] * NUM_PCA_PINS)
    for i in range(NUM_PCA_PINS):
        pca9539r_msg2.state[i] = ((i + 1) % 2)

    pca9539r_mask2 = pca9539r_pb2.MxPca9539rStore()
    pca9539r_mask2.state.extend([0] * NUM_PCA_PINS)
    for i in range(NUM_PCA_PINS):
        pca9539r_mask2.state[i] = 1

    pca9539r_update2 = StoreUpdate(
        pca9539r_msg2,
        pca9539r_mask2,
        stores_pb2.MxStoreType.PCA9539R,
        0x75)

    return [pca9539r_update1, pca9539r_update2]
