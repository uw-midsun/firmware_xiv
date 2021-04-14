import threading

from mu.protogen import stores_pb2
from mu.protogen import mcp2515_pb2
from mu.sims import sim

from mu.harness.project import StoreUpdate

GPIO_KEY = (stores_pb2.MuStoreType.GPIO, 0)
MCP2515_KEY = (stores_pb2.MuStoreType.MCP2515, 0)


class Mci(sim.Sim):
    def __init__(self):
        super().__init__()
        self.gpio_ab = None
        self.precharge_enable = False
        self.tx_id = 0
        self.tx_data = 0

    def handle_update(self, pm, proj, key):
        # Upon precharge enable (PA9) going high, set PB0 and PA10 to mock precharge
        if not self.precharge_enable and self.get_gpio(proj, 'a', 9):
            self.precharge_enable = self.get_gpio(proj, 'a', 9)
            set_b0 = threading.Timer(0.1, self.set_gpio, [proj, 'b', 0, True])
            set_b0.start()
            set_a10 = threading.Timer(0.2, self.set_gpio, [proj, 'a', 10, True])
            set_a10.start()

        # grab mcp2515 outbound messages
        if MCP2515_KEY in proj.stores:
            self.tx_id = proj.stores[MCP2515_KEY].tx_id
            self.tx_data = proj.stores[MCP2515_KEY].tx_data
            print('MCP2515 Output: {}'.format(self.tx_data))

    def update_mcp2515_rx(self, proj, rx_id, data):
        mcp2515_msg = mcp2515_pb2.MuMcp2515Store()
        mcp2515_msg.rx_id = rx_id
        mcp2515_msg.rx_extended = False
        mcp2515_msg.rx_dlc = 8
        mcp2515_msg.rx_data = data

        mcp2515_mask = mcp2515_pb2.MuMcp2515Store()
        mcp2515_mask.rx_id = 1
        mcp2515_update = StoreUpdate(mcp2515_msg, mcp2515_mask, stores_pb2.MuStoreType.MCP2515, 0)
        proj.write_store(mcp2515_update)
