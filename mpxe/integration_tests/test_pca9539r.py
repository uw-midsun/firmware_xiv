import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.pca9539r import Pca9539r
from mpxe.integration_tests.init_cond import pca9539r_init_conditions


class TestPca9539r(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.pca9539r = self.manager.start('smoke_pca9539r', Pca9539r(), pca9539r_init_conditions())

    def test_pca9539r(self):
        time.sleep(0.1)
        self.pca9539r.sim.assert_store_values(self, True)
        time.sleep(0.1)
        self.pca9539r.sim.assert_store_values(self, False)
        value = True
        for _ in range(3):
            time.sleep(1)
            self.pca9539r.sim.assert_store_values(self, value)
            value = not value


if __name__ == '__main__':
    unittest.main()
