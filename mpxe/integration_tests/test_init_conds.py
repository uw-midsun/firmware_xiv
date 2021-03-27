import time

from mpxe.integration_tests import int_test
from mpxe.sims.pca9539r import Pca9539r, NUM_PCA_PINS

from mpxe.protogen import pca9539r_pb2
from mpxe.protogen import stores_pb2
from mpxe.harness.project import StoreUpdate


class TestMpxeInit(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.pca9539r = self.manager.start(
            'mpxe_init_conds', Pca9539r(), self.pca9539r_init_conditions())

    def test_initial_conditions(self):
        pca_key1 = 0x74
        time.sleep(0.1)
        for i in range(NUM_PCA_PINS):
            self.pca9539r.sim.assert_store_values(i, i % 2, pca_key1)

        time.sleep(0.1)
        pca_key2 = 0x75
        for i in range(NUM_PCA_PINS):
            self.pca9539r.sim.assert_store_values(i, (i + 1) % 2, pca_key2)

    def pca9539r_init_conditions(self):
        # initial states for first pca9539r
        pca9539r_msg = pca9539r_pb2.MxPca9539rStore()
        pca9539r_msg.state.extend([0] * NUM_PCA_PINS)
        for i in range(NUM_PCA_PINS):
            pca9539r_msg.state[i] = i % 2

        pca9539r_mask = pca9539r_pb2.MxPca9539rStore()
        pca9539r_mask.state.extend([0] * NUM_PCA_PINS)
        for i in range(NUM_PCA_PINS):
            pca9539r_mask.state[i] = 1

        pca9539r_update1 = StoreUpdate(
            pca9539r_msg,
            pca9539r_mask,
            stores_pb2.MxStoreType.PCA9539R,
            0x74)

        # initial states for second pca9539r
        pca9539r_msg2 = pca9539r_pb2.MxPca9539rStore()
        pca9539r_msg2.state.extend([0] * NUM_PCA_PINS)
        for i in range(NUM_PCA_PINS):
            pca9539r_msg2.state[i] = ((i + 1) % 2)

        pca9539r_mask2 = pca9539r_pb2.MxPca9539rStore()
        pca9539r_mask2.state.extend([0] * NUM_PCA_PINS)
        for i in range(NUM_PCA_PINS):
            pca9539r_mask2.state[i] = 1

        pca9539r_update2 = StoreUpdate(
            pca9539r_msg2,
            pca9539r_mask2,
            stores_pb2.MxStoreType.PCA9539R,
            0x75)

        return [pca9539r_update1, pca9539r_update2]
