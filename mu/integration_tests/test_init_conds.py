import time

from mu.integration_tests import int_test
from mu.harness.project import StoreUpdate
from mu.harness.board_sim import BoardSim
from mu.sims.sub_sims.pca9539r import Pca9539r, NUM_PCA9539R_PINS
from mu.protogen import pca9539r_pb2
from mu.protogen import stores_pb2

PCA9539R_1_KEY = (stores_pb2.MuStoreType.PCA9539R, 0x74)
PCA9539R_2_KEY = (stores_pb2.MuStoreType.PCA9539R, 0x75)


class MuInitConds(BoardSim):
    def __init__(self, pm, proj_name):
        conds = pca9539r_init_conditions()
        super().__init__(pm, proj_name, sub_sim_classes=[Pca9539r], init_conds=conds)


class TestMpxeInit(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start('mu_init_conds', sim_class=MuInitConds)

    def test_initial_conditions(self):
        time.sleep(0.1)
        sub_sim = self.board.sub_sims['Pca9539r']
        for i in range(NUM_PCA9539R_PINS):
            sub_sim.assert_pin_state(PCA9539R_1_KEY, i, i%2)
        for i in range(NUM_PCA9539R_PINS):
            sub_sim.assert_pin_state(PCA9539R_2_KEY, i, (i+1)%2)


# Sets up init conds to a 1-0-1-0 repeating pattern
def pca9539r_init_conditions():
    # initial states for first pca9539r
    pca9539r_msg = pca9539r_pb2.MuPca9539rStore()
    pca9539r_msg.state.extend([0] * NUM_PCA9539R_PINS)
    for i in range(NUM_PCA9539R_PINS):
        pca9539r_msg.state[i] = i % 2

    pca9539r_mask = pca9539r_pb2.MuPca9539rStore()
    pca9539r_mask.state.extend([0] * NUM_PCA9539R_PINS)
    for i in range(NUM_PCA9539R_PINS):
        pca9539r_mask.state[i] = 1

    pca9539r_update1 = StoreUpdate(
        pca9539r_msg,
        pca9539r_mask,
        PCA9539R_1_KEY)

    # initial states for second pca9539r
    pca9539r_msg2 = pca9539r_pb2.MuPca9539rStore()
    pca9539r_msg2.state.extend([0] * NUM_PCA9539R_PINS)
    for i in range(NUM_PCA9539R_PINS):
        pca9539r_msg2.state[i] = ((i + 1) % 2)

    pca9539r_mask2 = pca9539r_pb2.MuPca9539rStore()
    pca9539r_mask2.state.extend([0] * NUM_PCA9539R_PINS)
    for i in range(NUM_PCA9539R_PINS):
        pca9539r_mask2.state[i] = 1

    pca9539r_update2 = StoreUpdate(
        pca9539r_msg2,
        pca9539r_mask2,
        PCA9539R_2_KEY)

    return [pca9539r_update1, pca9539r_update2]
