from mu.harness.board_sim import BoardSim
from mu.sims.sub_sims.ads1259 import Ads1259
from mu.sims.sub_sims.adt7476a import Adt7476a
from mu.sims.sub_sims.mcp23008 import Mcp23008


class BmsCarrier(BoardSim):
    def __init__(self, pm, proj_name='bms_carrier'):
        sub_sim_classes = [
            Ads1259,
            Adt7476a,
            Mcp23008,
        ]
        super().__init__(pm, proj_name, sub_sim_classes=sub_sim_classes)
