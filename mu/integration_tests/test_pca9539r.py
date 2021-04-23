import unittest
import time

from mu.integration_tests import int_test
from mu.sims.front_pd import FrontPd
from mu.sims.sub_sims.pca9539r import Pca9539r, NUM_PCA9539R_PINS
from mu.protogen import stores_pb2

# 0x74 is address used in smoketest
PCA9539R_KEY = Pca9539r.addr_to_key(0x74)


class TestPca9539r(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start('smoke_pca9539r', FrontPd)

    def test_pca9539r(self):
        time.sleep(0.1)
        for i in range(NUM_PCA9539R_PINS):
            self.board.sub_sims['Pca9539r'].assert_pin_state(PCA9539R_KEY, i, 1)
        time.sleep(0.1)
        for i in range(NUM_PCA9539R_PINS):
            self.board.sub_sims['Pca9539r'].assert_pin_state(PCA9539R_KEY, i, 0)

        value = 1
        for _ in range(1):
            time.sleep(0.5)
            for i in range(NUM_PCA9539R_PINS):
                self.board.sub_sims['Pca9539r'].assert_pin_state(PCA9539R_KEY, i, value)
            value = not value


if __name__ == '__main__':
    unittest.main()
