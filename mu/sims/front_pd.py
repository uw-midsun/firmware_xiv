from mu.harness.board_sim import BoardSim
from mu.sims.sub_sims.pca9539r import Pca9539r


class FrontPd(BoardSim):
    def __init__(self, pm, proj_name):
        sub_sim_classes = [Pca9539r]
        init_cond = self.make_gpio_update('a', 8, False)
        super().__init__(pm, proj_name, sub_sim_classes=sub_sim_classes, init_conds=[init_cond])
