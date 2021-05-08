import unittest
import time

from mu.integration_tests import int_test
from mu.sims.front_pd import FrontPd


class TestPdInit(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start(FrontPd)

    def test_front_init(self):
        time.sleep(1)
        init_pin = self.board.get_gpio('a', 8)
        assert init_pin is False
        self.assert_can_received('FRONT_CURRENT_MEASUREMENT')


if __name__ == '__main__':
    unittest.main()
