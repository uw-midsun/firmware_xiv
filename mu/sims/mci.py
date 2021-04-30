from mu.harness.board_sim import BoardSim
from mu.sims.sub_sims.mcp2515 import Mcp2515


class Mci(BoardSim):
    def __init__(self, pm, proj_name):
        super().__init__(pm, proj_name, sub_sim_classes=[Mcp2515])
        self.precharge_enable = False

    def handle_store(self, store, key):
        # Upon precharge enable (PA9) going high, set PB0 and PA10 to mock precharge
        if not self.precharge_enable and self.get_gpio('a', 9):
            self.precharge_enable = self.get_gpio('a', 9)
            self.set_timer(0.1, self.set_gpio, ['b', 0, True])
            self.set_timer(0.2, self.set_gpio, ['a', 10, True])
