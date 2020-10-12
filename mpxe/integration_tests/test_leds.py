import sys
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import time

import int_test

class TestLeds(int_test.IntTest):
    def setUp(self):
        super(TestLeds, self).setUp()
        self.leds = self.manager.start('controller_board_blinking_leds')

    def test_leds(self):
        time.sleep(1)

if __name__ == '__main__':
    unittest.main()
