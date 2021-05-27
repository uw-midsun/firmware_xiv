from mu.sims.sub_sims.mcp3427 import Mcp3427
from mu.harness.board_sim import BoardSim

class Solar(BoardSim):
    def __init__(self, pm, proj_name='solar'):
        sub_sim_classes = [
            Mcp3427,
        ]
        super().__init__(pm, proj_name, sub_sim_classes=sub_sim_classes)
