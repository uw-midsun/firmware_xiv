from mu.harness.board_sim import BoardSim
from mu.sims.sub_sims.ads1015 import Ads1015


class PedalBoard(BoardSim):
    def __init__(self, pm, proj_name='pedal_board'):
        super().__init__(pm, proj_name, sub_sim_classes=[Ads1015])
