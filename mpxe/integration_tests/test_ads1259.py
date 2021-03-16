import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.ads1259 import Ads1259


class TestAds1259(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.ads1259 = self.manager.start('smoke_ads1259', Ads1259())

    def test_ads1259(self):
        time.sleep(1)
        self.ads1259.sim.update_ads_reading(self.ads1259, 0x9A)
        time.sleep(4)  # smoke test runs
        self.ads1259.sim.assert_store_value_reading(self.ads1259, 0x9A)


if __name__ == '__main__':
    unittest.main()
