from mu.harness.project import StoreUpdate
from mu.harness.sub_sim import SubSim
from mu.protogen import stores_pb2
from mu.protogen import mcp2515_pb2

MCP2515_KEY = (stores_pb2.MuStoreType.MCP2515, 0)


class Mcp2515(SubSim):
    def handle_store(self, store, key):
        if key[0] == stores_pb2.MuStoreType.MCP2515:
            print('MCP2515 Output: {}#{}'.format(store.tx_id, store.tx_data))

    def update_rx(self, rx_id, data):
        mcp2515_msg = mcp2515_pb2.MuMcp2515Store()
        mcp2515_msg.rx_id = rx_id
        mcp2515_msg.rx_extended = False
        mcp2515_msg.rx_dlc = 8
        mcp2515_msg.rx_data = data

        mcp2515_mask = mcp2515_pb2.MuMcp2515Store()
        mcp2515_mask.rx_id = 1
        mcp2515_update = StoreUpdate(mcp2515_msg, mcp2515_mask, MCP2515_KEY)
        self.sim.proj.write_store(mcp2515_update)
