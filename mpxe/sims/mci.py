import time

from mpxe.protogen import stores_pb2
from mpxe.protogen import mcp2515_pb2
from mpxe.sims import sim

from mpxe.harness.project import StoreUpdate

GPIO_KEY = (stores_pb2.MxStoreType.GPIO, 0)
MCP2515_KEY = (stores_pb2.MxStoreType.MCP2515, 0)


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
            time.sleep(0.5)
            # update precharge monitor and latch out
            print('setting gpio b0 on')
            self.set_gpio(proj, 'b', 0, True)
            time.sleep(0.1)
            self.set_gpio(proj, 'a', 10, True)

        # grab mcp2515 outbound messages
        if MCP2515_KEY in proj.stores:
            self.tx_id = proj.stores[MCP2515_KEY].tx_id
            self.tx_data = proj.stores[MCP2515_KEY].tx_data
            print('MCP2515 Output: {}'.format(self.tx_data))

    def update_mcp2515_rx(self, proj, rx_id, data):
        mcp2515_msg = mcp2515_pb2.MxMcp2515Store()
        mcp2515_msg.rx_id = rx_id
        mcp2515_msg.rx_extended = False
        mcp2515_msg.rx_dlc = 8
        mcp2515_msg.rx_data = data

        mcp2515_mask = mcp2515_pb2.MxMcp2515Store()
        mcp2515_mask.rx_id = 1
        mcp2515_update = StoreUpdate(mcp2515_msg, mcp2515_mask, stores_pb2.MxStoreType.MCP2515, 0)
        proj.write_store(mcp2515_update)
