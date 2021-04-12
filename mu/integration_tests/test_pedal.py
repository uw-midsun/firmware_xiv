import unittest
import time

from mu.integration_tests import int_test
from mu.sims.pedal_board import PedalBoard


class TestPedal(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.pedal = self.manager.start('pedal_board', PedalBoard())

    def test_pedal(self):
        time.sleep(0.3)
        # ads1015 reading is zero so throttle should be zero
        self.assert_can_data('PEDAL_OUTPUT', 'throttle_output', 0)

        # set throttle channel to 50
        self.pedal.sim.update_ads_reading(self.pedal, 50, 0)
        time.sleep(0.3)
        # ads1015 reading is nonzero so throttle should be nonzero
        self.assert_can_data('PEDAL_OUTPUT', 'throttle_output', 50)


if __name__ == '__main__':
    unittest.main()
