import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.leds import Leds

class TestLeds(int_test.IntTest):
    def setUp(self):
        super(TestLeds, self).setUp()
        self.leds = self.manager.start('controller_board_blinking_leds', Leds())

    def test_leds(self):
        time.sleep(1)

if __name__ == '__main__':
    unittest.main()
