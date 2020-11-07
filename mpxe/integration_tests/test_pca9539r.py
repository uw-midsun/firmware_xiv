import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.pca9539r import Pca9539r

class TestPca9539r(int_test.IntTest):
    def setUp(self):
        super(TestPca9539r, self).setUp()
        self.pca9539r = self.manager.start('smoke_pca9539r', Pca9539r())

    def test_pca9539r(self):
        time.sleep(0.1)
        self.pca9539r.sim.assert_store_values(self.pca9539r, True)
        time.sleep(0.1)
        self.pca9539r.sim.assert_store_values(self.pca9539r, False)
        value = True
        for x in range(3):
            time.sleep(1)
            self.pca9539r.sim.assert_store_values(self.pca9539r, value)
            value =  not value

if __name__ == '__main__':
    unittest.main()
