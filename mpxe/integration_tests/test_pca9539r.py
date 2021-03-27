import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.pca9539r import Pca9539r, NUM_PCA_PINS


class TestPca9539r(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.pca9539r = self.manager.start('smoke_pca9539r', Pca9539r())

    def test_pca9539r(self):
        pca_key = 0x74  # I2C address defined in smoketest
        time.sleep(0.1)
        for i in range(NUM_PCA_PINS):
            self.pca9539r.sim.assert_store_values(i, 1, pca_key)
        time.sleep(0.1)
        for i in range(NUM_PCA_PINS):
            self.pca9539r.sim.assert_store_values(i, 0, pca_key)

        value = 1
        for _ in range(1):
            time.sleep(1)
            for i in range(NUM_PCA_PINS):
                self.pca9539r.sim.assert_store_values(i, value, pca_key)
            value = not value


if __name__ == '__main__':
    unittest.main()
