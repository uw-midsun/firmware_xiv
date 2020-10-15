from mpxe.protogen import stores_pb2
from mpxe.protogen import mcp2515_pb2
from mpxe.protogen import gpio_pb2

import time

from mpxe.sims import sim

GPIO_KEY = (stores_pb2.MxStoreType.GPIO, 0)
MCP2515_KEY = (stores_pb2.MxStoreType.MCP2515, 0)

class Mci(sim.Sim):
    def __init__(self):
        super(Mci, self).__init__()
        self.gpio_ab = None
        self.tx_id = 0
        self.tx_data = 0
    def handle_update(self, pm, proj):
        def store_gpio_ab():
            self.gpio_ab = [proj.stores[GPIO_KEY].state[i] for i in range(32)]
        if self.gpio_ab == None:
            store_gpio_ab()
        elif proj.stores[GPIO_KEY].state[9] == 1 and self.gpio_ab[9] != 1:
            time.sleep(0.1)
            store_gpio_ab()
            # update precharge monitor and latch out
            self.set_gpio(proj, 'b', 0, True)
            time.sleep(0.1)
            self.set_gpio(proj, 'a', 10, True)
        else:
            store_gpio_ab()
        # grab mcp2515 outbound messages
        if MCP2515_KEY in proj.stores:
            self.tx_id = proj.stores[MCP2515_KEY].tx_id
            self.tx_data = proj.stores[MCP2515_KEY].tx_data
            print('MCP2515 Output: {}'.format(self.tx_data))
    def update_mcp2515_rx(self, proj, id, data):
        mcp2515_msg = mcp2515_pb2.MxMcp2515Store()
        mcp2515_msg.rx_id = id
        mcp2515_msg.rx_extended = False
        mcp2515_msg.rx_dlc = 8
        mcp2515_msg.rx_data = data

        mcp2515_mask = mcp2515_pb2.MxMcp2515Store()
        mcp2515_mask.rx_id = 1

        proj.write_store(store_pb2.MxStoreType.MCP2515, mcp2515_msg, mcp2515_mask)
