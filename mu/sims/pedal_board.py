from mu.harness.board_sim import BoardSim
from mu.harness.sim_io import SimIo
from mu.sims.sub_sims.ads1015 import Ads1015, ADS1015_KEY


class PedalBoard(BoardSim):
    def __init__(self, pm, proj_name='pedal_board'):
        super().__init__(pm, proj_name, sub_sim_classes=[Ads1015])
        self.pm.new_io(SimIo('throttle', self.get_throttle, self.set_throttle))
        self.pm.new_io(SimIo('brake', self.get_brake, self.set_brake))

    def get_throttle(self):
        return self.stores[ADS1015_KEY].readings[0]

    def get_brake(self):
        return self.stores[ADS1015_KEY].readings[1]

    def set_throttle(self, val):
        self.sub_sims['ads1015'].update_reading(0, int(val))

    def set_brake(self, val):
        self.sub_sims['ads1015'].update_reading(1, int(val))
