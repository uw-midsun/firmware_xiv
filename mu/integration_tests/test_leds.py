import unittest
import time

from mu.integration_tests import int_test
from mu.sims.leds import Leds


class TestLeds(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start(Leds)

    def test_leds(self):
        time.sleep(0.3)


if __name__ == '__main__':
    unittest.main()
