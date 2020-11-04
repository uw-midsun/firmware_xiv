import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.pca9539r import Pca9539r

class TestLeds(int_test.IntTest):
    def setUp(self):
        super(TestLeds, self).setUp()
        self.leds = self.manager.start('smoke_pca9539r', Pca9539r())

    def test_pca9539r(self):
        print("heelo world")
        time.sleep(1)

if __name__ == '__main__':
    unittest.main()
