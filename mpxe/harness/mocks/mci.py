from ..protogen import stores_pb2
from ..protogen import mcp2515_pb2
from ..protogen import gpio_pb2
from .mock_helpers import write_proj

import time

GPIO_KEY = (stores_pb2.MxStoreType.GPIO, 0)
MCP2515_KEY = (stores_pb2.MxStoreType.MCP2515, 0)

class Mci:
    def __init__(self):
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
            gpio_msg = gpio_pb2.MxGpioStore()
            gpio_msg.state.extend([0] * 6 * 16)
            gpio_msg.state[16] = True
            gpio_msg.state[10] = True
            gpio_mask = gpio_pb2.MxGpioStore()
            gpio_mask.state.extend([0] * 6 * 16)
            gpio_mask.state[16] = True
            gpio_mask.state[10] = True
            write_proj(proj, stores_pb2.MxStoreType.GPIO, gpio_msg, gpio_mask)
        else:
            store_gpio_ab()
        # grab mcp2515 outbound messages
        if MCP2515_KEY in proj.stores:
            self.tx_id = proj.stores[MCP2515_KEY].tx_id
            self.tx_data = proj.stores[MCP2515_KEY].tx_data
            print('MCP2515 Output: {}'.format(self.tx_data))
    def handle_log(self, pm, proj, log):
        print('[{}]'.format(proj.name), log)
    def update_mcp2515_rx(self, proj, id, data):
        mcp2515_msg = mcp2515_pb2.MxMcp2515Store()
        mcp2515_msg.rx_id = id
        mcp2515_msg.rx_extended = False
        mcp2515_msg.rx_dlc = 8
        mcp2515_msg.rx_data = data

        mcp2515_mask = mcp2515_pb2.MxMcp2515Store()
        mcp2515_mask.rx_id = 1

        write_proj(proj, store_pb2.MxStoreType.MCP2515, mcp2515_msg, mcp2515_mask)
