import unittest
import time

from mu.integration_tests import int_test
from mu.sims.bms_carrier import BmsCarrier


class TestAds1259(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start('smoke_ads1259', sim_class=BmsCarrier)

    def test_ads1259(self):
        # Smoke test has no output, but you can visually ensure logged reading changes
        time.sleep(0.2) # Let smoke test run once
        self.board.sub_sim('ads1259').update_reading(0x9A)
        time.sleep(0.2)  # Let smoke test run again


if __name__ == '__main__':
    unittest.main()
